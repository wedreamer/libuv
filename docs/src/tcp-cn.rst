
.. _tcp:

:c:type:`uv_tcp_t` --- TCP handle
=================================

TCP 句柄用于表示 TCP 流和服务器.

:c:type:`uv_tcp_t` is a 'subclass' of :c:type:`uv_stream_t`.


Data types
----------

.. c:type:: uv_tcp_t

    TCP handle type.


Public members
^^^^^^^^^^^^^^

N/A

.. seealso:: The :c:type:`uv_stream_t` members also apply.


API
---

.. c:function:: int uv_tcp_init(uv_loop_t* loop, uv_tcp_t* handle)

    初始化句柄。目前还没有创建套接字.

.. c:function:: int uv_tcp_init_ex(uv_loop_t* loop, uv_tcp_t* handle, unsigned int flags)

    使用指定的标志初始化句柄。目前只有 `flags` 参数的低 8 位用作套接字域。将为给定域创建一个套接字。如果指定域是 ``AF_UNSPEC`` 则不创建套接字，就像 :c:func:`uv_tcp_init`.

    .. versionadded:: 1.7.0

.. c:function:: int uv_tcp_open(uv_tcp_t* handle, uv_os_sock_t sock)

    打开现有文件描述符或 SOCKET 作为 TCP 句柄.

    .. versionchanged:: 1.2.1 the file descriptor is set to non-blocking mode.

    .. note::
        不检查传递的文件描述符或 SOCKET 的类型，但要求它表示有效的流套接字.

.. c:function:: int uv_tcp_nodelay(uv_tcp_t* handle, int enable)

    启用 `TCP_NODELAY`，禁用 Nagle 算法.

.. c:function:: int uv_tcp_keepalive(uv_tcp_t* handle, int enable, unsigned int delay)

    启用/禁用 TCP 保持活动。 `delay` 是以秒为单位的初始延迟，当 `enable` 为零时忽略.

    达到“延迟”后，仍然会发生 10 次连续探测，每次探测与前一次间隔 1 秒。如果在此过程结束时连接仍然丢失，则句柄将被销毁，并传递给相应回调的“UV_ETIMEDOUT”错误.

.. c:function:: int uv_tcp_simultaneous_accepts(uv_tcp_t* handle, int enable)

    启用/禁用在侦听新 TCP 连接时由操作系统排队的同步异步接受请求.

    此设置用于调整 TCP 服务器以获得所需的性能.
    同时接受可以显着提高接受连接的速率（这就是默认启用它的原因），但可能导致多进程设置中的负载分布不均匀.

.. c:function:: int uv_tcp_bind(uv_tcp_t* handle, const struct sockaddr* addr, unsigned int flags)

    将句柄绑定到地址和端口。 `addr` 应该指向一个初始化的 `struct sockaddr_in` 或 `struct sockaddr_in6`.

    当端口已经被占用时，你会看到 :c:func:`uv_listen` 或 :c:func:`uv_tcp_connect` 的 `UV_EADDRINUSE` 错误。也就是说，成功调用此函数并不能保证调用 :c:func:`uv_listen` 或 :c:func:`uv_tcp_connect` 也会成功.

    `flags` 可以包含 `UV_TCP_IPV6ONLY`，在这种情况下双栈支持被禁用并且只使用 IPv6.

.. c:function:: int uv_tcp_getsockname(const uv_tcp_t* handle, struct sockaddr* name, int* namelen)

    获取句柄绑定的当前地址。 `name` 必须指向有效且足够大的内存块，建议使用 `struct sockaddr_storage` 来支持 IPv4 和 IPv6.

.. c:function:: int uv_tcp_getpeername(const uv_tcp_t* handle, struct sockaddr* name, int* namelen)

    获取连接到句柄的对等方的地址。 `name` 必须指向有效且足够大的内存块，建议使用 `struct sockaddr_storage` 来支持 IPv4 和 IPv6.

.. c:function:: int uv_tcp_connect(uv_connect_t* req, uv_tcp_t* handle, const struct sockaddr* addr, uv_connect_cb cb)

    建立 IPv4 或 IPv6 TCP 连接。提供一个初始化的 TCP 句柄和一个未初始化的:c:type:`uv_connect_t`。 `addr` 应该指向一个初始化的 `struct sockaddr_in` 或 `struct sockaddr_in6`.

    在 Windows 上，如果 "addr" 被初始化为指向一个未指定的地址（"0.0.0.0" 或 ""::"），它将被更改为指向 "localhost".
    这样做是为了匹配 Linux 系统的行为.

    建立连接或发生连接错误时进行回调.

    .. versionchanged:: 1.19.0 added ``0.0.0.0`` and ``::`` to ``localhost``
        mapping

.. seealso:: The :c:type:`uv_stream_t` API functions also apply.

.. c:function:: int uv_tcp_close_reset(uv_tcp_t* handle, uv_close_cb close_cb)

    通过发送 RST 数据包重置 TCP 连接。这是通过将 `SO_LINGER` 套接字选项设置为零的逗留间隔，然后调用 :c:func:`uv_close` 来完成的。由于某些平台不一致，不允许混合使用 :c:func:`uv_shutdown` 和 :c:func:`uv_tcp_close_reset` 调用。

    .. versionadded:: 1.32.0

.. c:function:: int uv_socketpair(int type, int protocol, uv_os_sock_t socket_vector[2], int flags0, int flags1)

    创建一对具有指定属性的连接套接字.
    生成的句柄可以传递给 `uv_tcp_open`，与 `uv_spawn` 一起使用，或用于任何其他目的.

    `flags0` 和 `flags1` 的有效值为:

      - UV_NONBLOCK_PIPE: 为 `OVERLAPPED` 或 `FIONBIO`/`O_NONBLOCK` I/O 使用打开指定的套接字句柄.
        对于将由 libuv 使用的句柄，这是推荐的，否则通常不推荐。

    等效于 :man:`socketpair(2)`，域为 AF_UNIX。

    .. versionadded:: 1.41.0
