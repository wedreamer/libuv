
.. _fs_event:

:c:type:`uv_fs_event_t` --- FS Event handle
===========================================

FS 事件句柄允许用户监视给定路径的更改，例如，如果文件被重命名或其中存在一般更改。 此句柄为每个平台上的作业使用最佳支持.

.. note::
    对于 AIX, 必须安装非默认 IBM bos.ahafs 包。 AIX 事件基础结构文件系统 (ahafs) 有一些限制:

        - ahafs 跟踪每个进程的监控并且不是线程安全的。 必须为同一事件的每个监视器生成一个单独的进程.
        - 如果仅监视包含文件夹，则不会收到文件修改（写入文件）的事件.

    有关更多详细信息, 请参阅文档_.

    Tz/OS 文件系统事件监视基础结构不会通知正在监视的目录中的文件创建/删除。 有关更多详细信息，请参阅 `IBM 知识中心`_.

    .. _documentation: https://developer.ibm.com/articles/au-aix_event_infrastructure/
    .. _`IBM Knowledge centre`: https://www.ibm.com/support/knowledgecenter/en/SSLTBW_2.2.0/com.ibm.zos.v2r1.bpxb100/ioc.htm




Data types
----------

.. c:type:: uv_fs_event_t

    FS 事件句柄类型.

.. c:type:: void (*uv_fs_event_cb)(uv_fs_event_t* handle, const char* filename, int events, int status)

    回调传递给 :c:func:`uv_fs_event_start` 将在句柄启动后重复调用。 如果句柄以目录开头，则 `filename` 参数将是目录中包含的文件的相对路径。 `events` 参数是 :c:type:`uv_fs_event` 元素的 ORed 掩码.

.. c:type:: uv_fs_event

    :c:type:`uv_fs_event_t` 处理监视器的事件类型.

    ::

        enum uv_fs_event {
            UV_RENAME = 1,
            UV_CHANGE = 2
        };

.. c:type:: uv_fs_event_flags

    可以传递给 :c:func:`uv_fs_event_start` 以控制其行为的标志.

    ::

        enum uv_fs_event_flags {
            /*
            * By default, if the fs event watcher is given a directory name, we will
            * watch for all events in that directory. This flags overrides this behavior
            * and makes fs_event report only changes to the directory entry itself. This
            * flag does not affect individual files watched.
            * This flag is currently not implemented yet on any backend.
            */
            UV_FS_EVENT_WATCH_ENTRY = 1,
            /*
            * By default uv_fs_event will try to use a kernel interface such as inotify
            * or kqueue to detect events. This may not work on remote file systems such
            * as NFS mounts. This flag makes fs_event fall back to calling stat() on a
            * regular interval.
            * This flag is currently not implemented yet on any backend.
            */
            UV_FS_EVENT_STAT = 2,
            /*
            * By default, event watcher, when watching directory, is not registering
            * (is ignoring) changes in its subdirectories.
            * This flag will override this behaviour on platforms that support it.
            */
            UV_FS_EVENT_RECURSIVE = 4
        };


Public members
^^^^^^^^^^^^^^

N/A

.. seealso:: The :c:type:`uv_handle_t` members also apply.


API
---

.. c:function:: int uv_fs_event_init(uv_loop_t* loop, uv_fs_event_t* handle)

    Initialize the handle.

.. c:function:: int uv_fs_event_start(uv_fs_event_t* handle, uv_fs_event_cb cb, const char* path, unsigned int flags)

    使用给定的回调启动句柄，它将监视指定的 `path` 以进行更改。 `flags` 可以是 :c:type:`uv_fs_event_flags` 的 ORed 掩码.

    .. note:: Currently the only supported flag is ``UV_FS_EVENT_RECURSIVE`` and
              only on OSX and Windows.

.. c:function:: int uv_fs_event_stop(uv_fs_event_t* handle)

    停止句柄，回调将不再被调用.

.. c:function:: int uv_fs_event_getpath(uv_fs_event_t* handle, char* buffer, size_t* size)

    获取句柄监控的路径。 缓冲区必须由用户预先分配。 成功时返回 0, 失败时返回错误代码 < 0。 成功时，`buffer` 将包含路径和`size` 长度。 如果缓冲区不够大，将返回 `UV_ENOBUFS` 并将 `size` 设置为所需的大小，包括空终止符.

    .. versionchanged:: 1.3.0 the returned length no longer includes the terminating null byte,
                        and the buffer is not null terminated.

    .. versionchanged:: 1.9.0 the returned length includes the terminating null
                        byte on `UV_ENOBUFS`, and the buffer is null terminated
                        on success.

.. seealso:: The :c:type:`uv_handle_t` API functions also apply.
