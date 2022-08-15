
.. _pipe:

:c:type:`uv_pipe_t` --- Pipe handle
===================================

管道句柄对 Unix 上的流文件（包括本地域套接字、管道和 FIFO） 和 Windows 上的命名管道提供了抽象。

:c:type:`uv_pipe_t` is a 'subclass' of :c:type:`uv_stream_t`.


Data types
----------

.. c:type:: uv_pipe_t

    Pipe handle type.


Public members
^^^^^^^^^^^^^^

.. c:member:: int uv_pipe_t.ipc

    此管道是否适用于进程之间的句柄传递。 只有将传递句柄的连接管道应该设置此标志，而不是调用 uv_accept 的侦听管道.

.. seealso:: The :c:type:`uv_stream_t` members also apply.


API
---

.. c:function:: int uv_pipe_init(uv_loop_t* loop, uv_pipe_t* handle, int ipc)

    初始化管道句柄。 `ipc` 参数是一个布尔值，用于指示此管道是否将用于在进程之间传递句柄（这可能会更改线路上的字节）。 只有将传递句柄的连接管道应该设置此标志，而不是调用 uv_accept 的侦听管道.

.. c:function:: int uv_pipe_open(uv_pipe_t* handle, uv_file file)

    打开现有文件描述符或 HANDLE 作为管道.

    .. versionchanged:: 1.2.1 the file descriptor is set to non-blocking mode.

    .. note::
        不检查传递的文件描述符或 HANDLE 的类型，但要求它表示有效管道.

.. c:function:: int uv_pipe_bind(uv_pipe_t* handle, const char* name)

    将管道绑定到文件路径 (Unix) 或名称 (Windows).

    .. note::
        Unix 上的路径被截断为 sizeof(sockaddr_un.sun_path) 字节，通常在 92 到 108 字节之间.

.. c:function:: void uv_pipe_connect(uv_connect_t* req, uv_pipe_t* handle, const char* name, uv_connect_cb cb)

    连接到 Unix 域套接字或命名管道.

    .. note::
        Unix 上的路径被截断为 sizeof(sockaddr_un.sun_path) 字节，通常在 92 到 108 字节之间.

.. c:function:: int uv_pipe_getsockname(const uv_pipe_t* handle, char* buffer, size_t* size)

    获取 Unix 域套接字或命名管道的名称.

    必须提供预先分配的缓冲区。 size 参数保存缓冲区的长度，并设置为输出时写入缓冲区的字节数。 如果缓冲区不够大，将返回 "UV_ENOBUFS" 并且 len 将包含所需的大小.

    .. versionchanged:: 1.3.0 the returned length no longer includes the terminating null byte,
                        and the buffer is not null terminated.

.. c:function:: int uv_pipe_getpeername(const uv_pipe_t* handle, char* buffer, size_t* size)

    获取句柄连接到的 Unix 域套接字或命名管道的名称.

    必须提供预先分配的缓冲区。 size 参数保存缓冲区的长度，并设置为输出时写入缓冲区的字节数。 如果缓冲区不够大，将返回 `UV_ENOBUFS` 并且 len 将包含所需的大小.

    .. versionadded:: 1.3.0

.. c:function:: void uv_pipe_pending_instances(uv_pipe_t* handle, int count)

    设置管道服务器等待连接时挂起的管道实例句柄数.

    .. note::
        此设置仅适用于 Windows.

.. c:function:: int uv_pipe_pending_count(uv_pipe_t* handle)
.. c:function:: uv_handle_type uv_pipe_pending_type(uv_pipe_t* handle)

    用于通过 IPC 管道接收句柄.

    首先 - 调用 :c:func:`uv_pipe_pending_count`，如果 > 0 则初始化给定 `type` 的句柄，由 :c:func:`uv_pipe_pending_type` 返回
  并调用 `uv_accept(pipe, handle)`.

.. seealso:: The :c:type:`uv_stream_t` API functions also apply.

.. c:function:: int uv_pipe_chmod(uv_pipe_t* handle, int flags)

    更改管道权限，允许从不同用户运行的进程访问它。 使管道可被所有用户写入或读取。 模式可以是 `UV_WRITABLE`、 `UV_READABLE` 或 `UV_WRITABLE | UV_READABLE`. 这个函数是阻塞的。

    .. versionadded:: 1.16.0

.. c:function:: int uv_pipe(uv_file fds[2], int read_flags, int write_flags)

    创建一对连接的管道句柄。数据可以写入`fds[1]`并从`fds[0]`读取。 生成的句柄可以传递给 `uv_pipe_open`，与 `uv_spawn` 一起使用，或用于任何其他目的。

    `flags` 的有效值为:

      - UV_NONBLOCK_PIPE: 为 `OVERLAPPED` 或 `FIONBIO`/`O_NONBLOCK` I/O 使用打开指定的套接字句柄。对于 libuv 将使用的句柄，建议这样做，否则通常不建议使用.

    等效于设置了 `O_CLOEXEC` 标志的 :man:`pipe(2)`.

    .. versionadded:: 1.41.0
