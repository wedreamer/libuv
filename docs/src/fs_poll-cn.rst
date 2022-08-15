
.. _fs_poll:

:c:type:`uv_fs_poll_t` --- FS Poll handle
=========================================

FS 轮询句柄允许用户监视给定路径的更改。 与 :c:type:`uv_fs_event_t` 不同，fs 轮询句柄使用 `stat` 来检测文件何时发生更改，因此它们可以在 fs 事件句柄无法处理的文件系统上工作。


Data types
----------

.. c:type:: uv_fs_poll_t

    FS Poll handle type.

.. c:type:: void (*uv_fs_poll_cb)(uv_fs_poll_t* handle, int status, const uv_stat_t* prev, const uv_stat_t* curr)

    回调传递给 :c:func:`uv_fs_poll_start`，当被监控路径发生任何变化时，将在句柄启动后重复调用。

    如果 `path` 不存在或不可访问，则使用 `status < 0` 调用回调。 观察者 * 没有 * 停止，但您的回调不会再次调用，直到发生某些变化（例如，当文件被创建或错误原因发生变化时）.

    当 `status == 0` 时，回调接收指向新旧 :c:type:`uv_stat_t` 结构的指针。 它们仅在回调期间有效.


Public members
^^^^^^^^^^^^^^

N/A

.. seealso:: The :c:type:`uv_handle_t` members also apply.


API
---

.. c:function:: int uv_fs_poll_init(uv_loop_t* loop, uv_fs_poll_t* handle)

    Initialize the handle.

.. c:function:: int uv_fs_poll_start(uv_fs_poll_t* handle, uv_fs_poll_cb poll_cb, const char* path, unsigned int interval)

    每 `interval` 毫秒检查 `path` 处的文件是否有更改.

    .. note::
        为了获得最大的可移植性，请使用多秒间隔。 亚秒级间隔不会检测到许多文件系统上的所有更改.

.. c:function:: int uv_fs_poll_stop(uv_fs_poll_t* handle)

    停止句柄，回调将不再被调用.

.. c:function:: int uv_fs_poll_getpath(uv_fs_poll_t* handle, char* buffer, size_t* size)

    获取句柄监控的路径。 缓冲区必须由用户预先分配。 成功时返回 0，失败时返回错误代码 < 0。 成功时，`buffer` 将包含路径和`size` 长度。 如果缓冲区不够大，将返回 `UV_ENOBUFS` 并将 `size` 设置为所需的大小.

    .. versionchanged:: 1.3.0 the returned length no longer includes the terminating null byte,
                        and the buffer is not null terminated.

    .. versionchanged:: 1.9.0 the returned length includes the terminating null
                        byte on `UV_ENOBUFS`, and the buffer is null terminated
                        on success.


.. seealso:: The :c:type:`uv_handle_t` API functions also apply.
