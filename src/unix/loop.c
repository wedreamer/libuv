/* Copyright Joyent, Inc. and other Node contributors. All rights reserved.
 *
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
#include "heap-inl.h"
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

// loop 初始化
int uv_loop_init(uv_loop_t* loop) {
  uv__loop_internal_fields_t* lfields;
  void* saved_data;
  int err;

  // 提供给用户使用的上下文数据
  saved_data = loop->data;
  memset(loop, 0, sizeof(*loop));
  // 重新赋值
  loop->data = saved_data;

  lfields = (uv__loop_internal_fields_t*) uv__calloc(1, sizeof(*lfields));
  if (lfields == NULL)
    return UV_ENOMEM;
  // 初始化 lfields
  loop->internal_fields = lfields;

  // 初始化 lfields 中的审计锁, 因为可能多个线程都会使用
  err = uv_mutex_init(&lfields->loop_metrics.lock);
  if (err)
    goto fail_metrics_mutex_init;

  // 初始化事件堆
  heap_init((struct heap*) &loop->timer_heap);
  // 各种队列初始化
  // wq ??
  QUEUE_INIT(&loop->wq);
  QUEUE_INIT(&loop->idle_handles);
  QUEUE_INIT(&loop->async_handles);
  QUEUE_INIT(&loop->check_handles);
  QUEUE_INIT(&loop->prepare_handles);
  QUEUE_INIT(&loop->handle_queue);

  // 初始化 active_handles, 初始值为 0
  loop->active_handles = 0;
  // 初始化 active_reqs.count, 初始值为 0
  loop->active_reqs.count = 0;
  // 初始化 nfds
  loop->nfds = 0;
  // 初始化 watchers
  loop->watchers = NULL;
  // 初始化 nwatchers 数目
  loop->nwatchers = 0;
  // 初始化 pending_queue 与 watcher_queue
  QUEUE_INIT(&loop->pending_queue);
  QUEUE_INIT(&loop->watcher_queue);

  // 初始化 closing_handles
  loop->closing_handles = NULL;
  // 更新 loop 时间
  uv__update_time(loop);
  // 初始化 async_io_watcher.fd
  loop->async_io_watcher.fd = -1;
  // 初始化 async_wfd
  loop->async_wfd = -1;
  // 初始化 signal_pipefd
  loop->signal_pipefd[0] = -1;
  loop->signal_pipefd[1] = -1;
  // 初始化 backend_fd
  loop->backend_fd = -1;
  // 初始化 emfile_fd
  loop->emfile_fd = -1;

  // 初始化 timer_counter
  loop->timer_counter = 0;
  // 初始化 stop_flag
  loop->stop_flag = 0;

  // 对应平台的初始化
  err = uv__platform_loop_init(loop);
  if (err)
    goto fail_platform_init;

  // TODO： 全局信号初始化
  uv__signal_global_once_init();
  // child_watcher 的 信号 初始化
  err = uv_signal_init(loop, &loop->child_watcher);
  if (err)
    goto fail_signal_init;

  // TODO：
  uv__handle_unref(&loop->child_watcher);
  loop->child_watcher.flags |= UV_HANDLE_INTERNAL;
  // 初始化 process_handles
  QUEUE_INIT(&loop->process_handles);

  // cloexec_lock rw 锁互斥锁初始化
  err = uv_rwlock_init(&loop->cloexec_lock);
  if (err)
    goto fail_rwlock_init;

  // wq_mutex 互斥锁初始化
  err = uv_mutex_init(&loop->wq_mutex);
  if (err)
    goto fail_mutex_init;

  // 异步队列以及异步回调队列初始化
  err = uv_async_init(loop, &loop->wq_async, uv__work_done);
  if (err)
    goto fail_async_init;

  // 初始化异步 handle 的 flags
  uv__handle_unref(&loop->wq_async);
  // 设置 wq_async 的 flags
  loop->wq_async.flags |= UV_HANDLE_INTERNAL;

  return 0;

fail_async_init:
  uv_mutex_destroy(&loop->wq_mutex);

fail_mutex_init:
  uv_rwlock_destroy(&loop->cloexec_lock);

fail_rwlock_init:
  uv__signal_loop_cleanup(loop);

fail_signal_init:
  uv__platform_loop_delete(loop);

fail_platform_init:
  uv_mutex_destroy(&lfields->loop_metrics.lock);

fail_metrics_mutex_init:
  uv__free(lfields);
  loop->internal_fields = NULL;

  uv__free(loop->watchers);
  loop->nwatchers = 0;
  return err;
}


int uv_loop_fork(uv_loop_t* loop) {
  int err;
  unsigned int i;
  uv__io_t* w;

  err = uv__io_fork(loop);
  if (err)
    return err;

  err = uv__async_fork(loop);
  if (err)
    return err;

  err = uv__signal_loop_fork(loop);
  if (err)
    return err;

  /* Rearm all the watchers that aren't re-queued by the above. */
  for (i = 0; i < loop->nwatchers; i++) {
    w = loop->watchers[i];
    if (w == NULL)
      continue;

    if (w->pevents != 0 && QUEUE_EMPTY(&w->watcher_queue)) {
      w->events = 0; /* Force re-registration in uv__io_poll. */
      QUEUE_INSERT_TAIL(&loop->watcher_queue, &w->watcher_queue);
    }
  }

  return 0;
}


void uv__loop_close(uv_loop_t* loop) {
  uv__loop_internal_fields_t* lfields;

  uv__signal_loop_cleanup(loop);
  uv__platform_loop_delete(loop);
  uv__async_stop(loop);

  if (loop->emfile_fd != -1) {
    uv__close(loop->emfile_fd);
    loop->emfile_fd = -1;
  }

  if (loop->backend_fd != -1) {
    uv__close(loop->backend_fd);
    loop->backend_fd = -1;
  }

  uv_mutex_lock(&loop->wq_mutex);
  assert(QUEUE_EMPTY(&loop->wq) && "thread pool work queue not empty!");
  assert(!uv__has_active_reqs(loop));
  uv_mutex_unlock(&loop->wq_mutex);
  uv_mutex_destroy(&loop->wq_mutex);

  /*
   * Note that all thread pool stuff is finished at this point and
   * it is safe to just destroy rw lock
   */
  uv_rwlock_destroy(&loop->cloexec_lock);

#if 0
  assert(QUEUE_EMPTY(&loop->pending_queue));
  assert(QUEUE_EMPTY(&loop->watcher_queue));
  assert(loop->nfds == 0);
#endif

  uv__free(loop->watchers);
  loop->watchers = NULL;
  loop->nwatchers = 0;

  lfields = uv__get_internal_fields(loop);
  uv_mutex_destroy(&lfields->loop_metrics.lock);
  uv__free(lfields);
  loop->internal_fields = NULL;
}


int uv__loop_configure(uv_loop_t* loop, uv_loop_option option, va_list ap) {
  uv__loop_internal_fields_t* lfields;

  lfields = uv__get_internal_fields(loop);
  if (option == UV_METRICS_IDLE_TIME) {
    lfields->flags |= UV_METRICS_IDLE_TIME;
    return 0;
  }

  if (option != UV_LOOP_BLOCK_SIGNAL)
    return UV_ENOSYS;

  if (va_arg(ap, int) != SIGPROF)
    return UV_EINVAL;

  loop->flags |= UV_LOOP_BLOCK_SIGPROF;
  return 0;
}
