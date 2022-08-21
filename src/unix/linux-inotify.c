/* Copyright Joyent, Inc. and other Node contributors. All rights reserved.
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#include "uv.h"
#include "uv/tree.h"
#include "internal.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <errno.h>

#include <sys/inotify.h>
#include <sys/types.h>
#include <unistd.h>

struct watcher_list {
  // 红黑树
  RB_ENTRY(watcher_list) entry;
  QUEUE watchers;
  int iterating;
  char* path;
  int wd;
};

struct watcher_root {
  struct watcher_list* rbh_root;
};
#define CAST(p) ((struct watcher_root*)(p))


static int compare_watchers(const struct watcher_list* a,
                            const struct watcher_list* b) {
  if (a->wd < b->wd) return -1;
  if (a->wd > b->wd) return 1;
  return 0;
}


RB_GENERATE_STATIC(watcher_root, watcher_list, entry, compare_watchers)


static void uv__inotify_read(uv_loop_t* loop,
                             uv__io_t* w,
                             unsigned int revents);

static void maybe_free_watcher_list(struct watcher_list* w,
                                    uv_loop_t* loop);

static int init_inotify(uv_loop_t* loop) {
  int fd;

  // 如果已经存在 inotify_fd 不需要初始化
  if (loop->inotify_fd != -1)
    return 0;

  // 调用 inotify_init1 申请文件描述符 -> man inotify_init1
  fd = inotify_init1(IN_NONBLOCK | IN_CLOEXEC);
  if (fd < 0)
    return UV__ERR(errno);

  // 绑定到 loop 上
  loop->inotify_fd = fd;
  // uv__inotify_read io 初始化
  // watcher 为 loop->inotify_read_watcher
  // 回调为 uv__inotify_read
  // 对应的文件描述符为 loop->inotify_fd
  uv__io_init(&loop->inotify_read_watcher, uv__inotify_read, loop->inotify_fd);
  // 启动 io, 事件为 POLLIN, 观察可读
  uv__io_start(loop, &loop->inotify_read_watcher, POLLIN);

  return 0;
}


int uv__inotify_fork(uv_loop_t* loop, void* old_watchers) {
  /* Open the inotify_fd, and re-arm all the inotify watchers. */
  int err;
  struct watcher_list* tmp_watcher_list_iter;
  struct watcher_list* watcher_list;
  struct watcher_list tmp_watcher_list;
  QUEUE queue;
  QUEUE* q;
  uv_fs_event_t* handle;
  char* tmp_path;

  if (old_watchers != NULL) {
    /* We must restore the old watcher list to be able to close items
     * out of it.
     */
    loop->inotify_watchers = old_watchers;

    QUEUE_INIT(&tmp_watcher_list.watchers);
    /* Note that the queue we use is shared with the start and stop()
     * functions, making QUEUE_FOREACH unsafe to use. So we use the
     * QUEUE_MOVE trick to safely iterate. Also don't free the watcher
     * list until we're done iterating. c.f. uv__inotify_read.
     */
    RB_FOREACH_SAFE(watcher_list, watcher_root,
                    CAST(&old_watchers), tmp_watcher_list_iter) {
      watcher_list->iterating = 1;
      QUEUE_MOVE(&watcher_list->watchers, &queue);
      while (!QUEUE_EMPTY(&queue)) {
        q = QUEUE_HEAD(&queue);
        handle = QUEUE_DATA(q, uv_fs_event_t, watchers);
        /* It's critical to keep a copy of path here, because it
         * will be set to NULL by stop() and then deallocated by
         * maybe_free_watcher_list
         */
        tmp_path = uv__strdup(handle->path);
        assert(tmp_path != NULL);
        QUEUE_REMOVE(q);
        QUEUE_INSERT_TAIL(&watcher_list->watchers, q);
        uv_fs_event_stop(handle);

        QUEUE_INSERT_TAIL(&tmp_watcher_list.watchers, &handle->watchers);
        handle->path = tmp_path;
      }
      watcher_list->iterating = 0;
      maybe_free_watcher_list(watcher_list, loop);
    }

    QUEUE_MOVE(&tmp_watcher_list.watchers, &queue);
    while (!QUEUE_EMPTY(&queue)) {
        q = QUEUE_HEAD(&queue);
        QUEUE_REMOVE(q);
        handle = QUEUE_DATA(q, uv_fs_event_t, watchers);
        tmp_path = handle->path;
        handle->path = NULL;
        err = uv_fs_event_start(handle, handle->cb, tmp_path, 0);
        uv__free(tmp_path);
        if (err)
          return err;
    }
  }

  return 0;
}


static struct watcher_list* find_watcher(uv_loop_t* loop, int wd) {
  // loop 中 inotify_watchers 是否存在 w
  struct watcher_list w;
  w.wd = wd;
  // 红黑树查找
  return RB_FIND(watcher_root, CAST(&loop->inotify_watchers), &w);
}

static void maybe_free_watcher_list(struct watcher_list* w, uv_loop_t* loop) {
  /* if the watcher_list->watchers is being iterated over, we can't free it. */
  if ((!w->iterating) && QUEUE_EMPTY(&w->watchers)) {
    /* No watchers left for this path. Clean up. */
    RB_REMOVE(watcher_root, CAST(&loop->inotify_watchers), w);
    inotify_rm_watch(loop->inotify_fd, w->wd);
    uv__free(w);
  }
}

// 读通知事件
static void uv__inotify_read(uv_loop_t* loop,
                             uv__io_t* dummy,
                             unsigned int events) {
  const struct inotify_event* e;
  struct watcher_list* w;
  uv_fs_event_t* h;
  QUEUE queue;
  QUEUE* q;
  const char* path;
  ssize_t size;
  const char *p;
  /* needs to be large enough for sizeof(inotify_event) + strlen(path) */
  char buf[4096];

  // 循环
  for (;;) {
    do
      // 读取 inotify_fd 事件信息
      size = read(loop->inotify_fd, buf, sizeof(buf));
    while (size == -1 && errno == EINTR);

    // 错误处理
    if (size == -1) {
      assert(errno == EAGAIN || errno == EWOULDBLOCK);
      break;
    }

    assert(size > 0); /* pre-2.6.21 thing, size=0 == read buffer too small */

    /* Now we have one or more inotify_event structs. */
    for (p = buf; p < buf + size; p += sizeof(*e) + e->len) {
      e = (const struct inotify_event*) p;

      events = 0;
      if (e->mask & (IN_ATTRIB|IN_MODIFY))
        // change 事件
        events |= UV_CHANGE;
      if (e->mask & ~(IN_ATTRIB|IN_MODIFY))
        // rename 事件
        events |= UV_RENAME;

      w = find_watcher(loop, e->wd);
      // 找到对应的 watcher
      if (w == NULL)
        continue; /* Stale event, no watchers left. */

      /* inotify does not return the filename when monitoring a single file
       * for modifications. Repurpose the filename for API compatibility.
       * I'm not convinced this is a good thing, maybe it should go.
       */
      path = e->len ? (const char*) (e + 1) : uv__basename_r(w->path);

      /* We're about to iterate over the queue and call user's callbacks.
       * What can go wrong?
       * A callback could call uv_fs_event_stop()
       * and the queue can change under our feet.
       * So, we use QUEUE_MOVE() trick to safely iterate over the queue.
       * And we don't free the watcher_list until we're done iterating.
       *
       * First,
       * tell uv_fs_event_stop() (that could be called from a user's callback)
       * not to free watcher_list.
       */
      // 遍历触发回调
      w->iterating = 1;
      QUEUE_MOVE(&w->watchers, &queue);
      while (!QUEUE_EMPTY(&queue)) {
        q = QUEUE_HEAD(&queue);
        h = QUEUE_DATA(q, uv_fs_event_t, watchers);

        QUEUE_REMOVE(q);
        QUEUE_INSERT_TAIL(&w->watchers, q);

        h->cb(h, path, events, 0);
      }
      /* done iterating, time to (maybe) free empty watcher_list */
      w->iterating = 0;
      maybe_free_watcher_list(w, loop);
    }
  }
}


int uv_fs_event_init(uv_loop_t* loop, uv_fs_event_t* handle) {
  // 初始化 UV_FS_EVENT handle
  uv__handle_init(loop, (uv_handle_t*)handle, UV_FS_EVENT);
  return 0;
}


int uv_fs_event_start(uv_fs_event_t* handle,
                      uv_fs_event_cb cb,
                      const char* path,
                      unsigned int flags) {
  struct watcher_list* w;
  size_t len;
  int events;
  int err;
  int wd;

  // 当前 handle 是否存活
  if (uv__is_active(handle))
    return UV_EINVAL;

  // 初始化通知
  err = init_inotify(handle->loop);
  if (err)
    return err;

  events = IN_ATTRIB
         | IN_CREATE
         | IN_MODIFY
         | IN_DELETE
         | IN_DELETE_SELF
         | IN_MOVE_SELF
         | IN_MOVED_FROM
         | IN_MOVED_TO;

  // 添加监控
  wd = inotify_add_watch(handle->loop->inotify_fd, path, events);
  // 添加失败
  if (wd == -1)
    return UV__ERR(errno);

  // 是否已经添加到 loop 中
  w = find_watcher(handle->loop, wd);
  // 已经存在不需要再添加
  if (w)
    goto no_insert;

  // 申请 watcher_list
  len = strlen(path) + 1;
  w = uv__malloc(sizeof(*w) + len);
  // 申请失败
  if (w == NULL)
    return UV_ENOMEM;

  // 初始化
  w->wd = wd;
  w->path = memcpy(w + 1, path, len);
  QUEUE_INIT(&w->watchers);
  w->iterating = 0;
  RB_INSERT(watcher_root, CAST(&handle->loop->inotify_watchers), w);

no_insert:
  // 列表中已经存在不需要再添加 fd
  // 激活 handle
  uv__handle_start(handle);
  // 当前 watcher 插入到 watcher_list 的 watcher
  QUEUE_INSERT_TAIL(&w->watchers, &handle->watchers);
  // 设置 handle 的 path
  handle->path = w->path;
  // 设置 callback
  handle->cb = cb;
  // 设置 wd
  handle->wd = wd;

  return 0;
}


int uv_fs_event_stop(uv_fs_event_t* handle) {
  struct watcher_list* w;

  if (!uv__is_active(handle))
    return 0;

  w = find_watcher(handle->loop, handle->wd);
  assert(w != NULL);

  handle->wd = -1;
  handle->path = NULL;
  uv__handle_stop(handle);
  QUEUE_REMOVE(&handle->watchers);

  maybe_free_watcher_list(w, handle->loop);

  return 0;
}


void uv__fs_event_close(uv_fs_event_t* handle) {
  uv_fs_event_stop(handle);
}
