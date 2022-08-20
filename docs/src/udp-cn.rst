
.. _udp:

:c:type:`uv_udp_t` --- UDP handle
=================================

UDP 句柄封装了客户端和服务器的 UDP 通信.


Data types
----------

.. c:type:: uv_udp_t

    UDP handle type.

.. c:type:: uv_udp_send_t

    UDP send request type.

.. c:type:: uv_udp_flags

    Flags used in :c:func:`uv_udp_bind` and :c:type:`uv_udp_recv_cb`..

    ::

        enum uv_udp_flags {
            /* Disables dual stack mode. */
            UV_UDP_IPV6ONLY = 1,
            /*
            * Indicates message was truncated because read buffer was too small. The
            * remainder was discarded by the OS. Used in uv_udp_recv_cb.
            */
            UV_UDP_PARTIAL = 2,
            /*
            * Indicates if SO_REUSEADDR will be set when binding the handle in
            * uv_udp_bind.
            * This sets the SO_REUSEPORT socket flag on the BSDs and OS X. On other
            * Unix platforms, it sets the SO_REUSEADDR flag. What that means is that
            * multiple threads or processes can bind to the same address without error
            * (provided they all set the flag) but only the last one to bind will receive
            * any traffic, in effect "stealing" the port from the previous listener.
            */
            UV_UDP_REUSEADDR = 4,
            /*
             * Indicates that the message was received by recvmmsg, so the buffer provided
             * must not be freed by the recv_cb callback.
             */
            UV_UDP_MMSG_CHUNK = 8,
            /*
             * Indicates that the buffer provided has been fully utilized by recvmmsg and
             * that it should now be freed by the recv_cb callback. When this flag is set
             * in uv_udp_recv_cb, nread will always be 0 and addr will always be NULL.
             */
            UV_UDP_MMSG_FREE = 16,
            /*
             * Indicates if IP_RECVERR/IPV6_RECVERR will be set when binding the handle.
             * This sets IP_RECVERR for IPv4 and IPV6_RECVERR for IPv6 UDP sockets on
             * Linux. This stops the Linux kernel from supressing some ICMP error messages
             * and enables full ICMP error reporting for faster failover.
             * This flag is no-op on platforms other than Linux.
             */
            UV_UDP_LINUX_RECVERR = 32,
            /*
            * Indicates that recvmmsg should be used, if available.
            */
            UV_UDP_RECVMMSG = 256
        };

.. c:type:: void (*uv_udp_send_cb)(uv_udp_send_t* req, int status)

    传递给 uv_udp_send 的回调类型定义，在数据发送后调用.

.. c:type:: void (*uv_udp_recv_cb)(uv_udp_t* handle, ssize_t nread, const uv_buf_t* buf, const struct sockaddr* addr, unsigned flags)

    传递给 uv_udp_recv_start 的回调类型定义，当端点接收数据时调用.

    * `handle`: UDP句柄
    * `nread`:  已接收的字节数.
      如果没有更多数据要读取，则为 0。请注意, 0 也可能意味着接收到一个空数据报(在这种情况下，`addr` 不是 NULL). 
      < 0 如果检测到传输错误；如果使用:man:`recvmmsg(2)` 将不再接收块并且可以安全地释放缓冲区.
    * `buf`: :c:type:`uv_buf_t` 与接收到的数据.
    * `addr`: ``struct sockaddr*`` 包含发送者的地址.
      可以为 NULL。仅在回调期间有效.
    * `flags`: 一个或多个 or'ed UV_UDP_* 常量.

    被调用者负责释放缓冲区, libuv 不重用它.
    错误时缓冲区可能是空缓冲区(其中 `buf->base` == NULL 和 `buf->len` == 0).

    当使用 :man:`recvmmsg(2)` 时，块将设置 `UV_UDP_MMSG_CHUNK` 标志，这些标志不能被释放。如果没有发生错误，将有一个最终回调，将 `nread` 设置为 0, `addr` 设置为 NULL, 并且缓冲区指向最初分配的数据，清除 `UV_UDP_MMSG_CHUNK` 标志并设置 `UV_UDP_MMSG_FREE` 标志。如果发生 UDP 套接字错误，`nread` 将 < 0。在任何一种情况下, 被调用者现在都可以安全地释放提供的缓冲区.

    .. versionchanged:: 1.40.0 added the `UV_UDP_MMSG_FREE` flag.

    .. note::
        当没有可读取的内容时，将使用 `nread` == 0 和 `addr` == NULL 调用接收回调，当接收到空的 UDP 数据包时使用 `nread` == 0 和 `addr` != NULL 调用接收回调.

.. c:enum:: uv_membership

    多播地址的成员资格类型.

    ::

        typedef enum {
            UV_LEAVE_GROUP = 0,
            UV_JOIN_GROUP
        } uv_membership;


Public members
^^^^^^^^^^^^^^

.. c:member:: size_t uv_udp_t.send_queue_size

    排队等待发送的字节数。该字段严格显示当前排队的信息量.

.. c:member:: size_t uv_udp_t.send_queue_count

    当前在队列中等待处理的发送请求数.

.. c:member:: uv_udp_t* uv_udp_send_t.handle

    发生此发送请求的 UDP 句柄.

.. seealso:: The :c:type:`uv_handle_t` members also apply.


API
---

.. c:function:: int uv_udp_init(uv_loop_t* loop, uv_udp_t* handle)

    初始化一个新的 UDP 句柄。实际的套接字是延迟创建的.
    Returns 0 on success.

.. c:function:: int uv_udp_init_ex(uv_loop_t* loop, uv_udp_t* handle, unsigned int flags)

    使用指定的标志初始化句柄。 `flags` 参数的低 8 位用作套接字域。将为给定域创建一个套接字.
    如果指定域是 ``AF_UNSPEC`` 则不创建套接字，就像 :c:func:`uv_udp_init`.

    其余位可用于设置这些标志之一:

    * `UV_UDP_RECVMMSG`: 如果设置, 并且平台支持它, recvmmsg(2) 将使用.

    .. versionadded:: 1.7.0
    .. versionchanged:: 1.37.0 added the `UV_UDP_RECVMMSG` flag.

.. c:function:: int uv_udp_open(uv_udp_t* handle, uv_os_sock_t sock)

    打开现有文件描述符或 Windows SOCKET 作为 UDP 句柄.

    Unix only:
    `sock` 参数的唯一要求是它遵循数据报协定（在未连接模式下工作，支持 sendmsg()/recvmsg() 等）.
    换句话说，其他数据报类型的套接字，如原始套接字或网络链接套接字也可以传递给这个函数.

    .. versionchanged:: 1.2.1 the file descriptor is set to non-blocking mode.

    .. note::
        不检查传递的文件描述符或 SOCKET 的类型，但要求它表示有效的数据报套接字.

.. c:function:: int uv_udp_bind(uv_udp_t* handle, const struct sockaddr* addr, unsigned int flags)

    将 UDP 句柄绑定到 IP 地址和端口.

    :param handle: UDP 句柄。应该用 :c:func:`uv_udp_init` 初始化.

    :param addr: `struct sockaddr_in` 或 `struct sockaddr_in6` 绑定地址和端口.

    :param flags: 指示如何绑定套接字, 支持 "UV_UDP_IPV6ONLY"、"UV_UDP_REUSEADDR" 和 "UV_UDP_RECVERR".

    :returns: 0 on success, or an error code < 0 on failure.

.. c:function:: int uv_udp_connect(uv_udp_t* handle, const struct sockaddr* addr)

    将 UDP 句柄与远程地址和端口相关联，因此此句柄发送的每条消息都会自动发送到该目的地.
    使用 `NULL` `addr` 调用此函数会断开句柄.
    尝试在已连接的句柄上调用 `uv_udp_connect()` 将导致 `UV_EISCONN` 错误。试图断开一个不是 connected 将返回一个 `UV_ENOTCONN` 错误.

    :param handle: UDP 句柄。应该用 :c:func:`uv_udp_init` 初始化.

    :param addr: `struct sockaddr_in` 或 `struct sockaddr_in6` 与要关联的地址和端口.

    :returns: 0 on success, or an error code < 0 on failure.

    .. versionadded:: 1.27.0

.. c:function:: int uv_udp_getpeername(const uv_udp_t* handle, struct sockaddr* name, int* namelen)

    在已连接的 UDP 句柄上获取 UDP 句柄的远程 IP 和端口.
    在未连接的句柄上，它返回 `UV_ENOTCONN`.

    :param handle: UDP 句柄。应该用 :c:func:`uv_udp_init` 初始化并绑定.

    :param name: 指向要填充地址数据的结构的指针.
        为了支持 IPv4 和 IPv6 `struct sockaddr_storage` 应该被使用.

    :param namelen: 在输入时，它表示“名称”字段的数据。在输出时，它指示填充了多少.

    :returns: 0 on success, or an error code < 0 on failure

    .. versionadded:: 1.27.0

.. c:function:: int uv_udp_getsockname(const uv_udp_t* handle, struct sockaddr* name, int* namelen)

    获取UDP句柄的本地IP和端口.

    :param handle: UDP 句柄。应该用 :c:func:`uv_udp_init` 初始化并绑定.

    :param name: 指向要填充地址数据的结构的指针.
        为了支持 IPv4 和 IPv6 `struct sockaddr_storage` 应该被使用.

    :param namelen: 在输入时，它表示“名称”字段的数据。在输出时，它指示填充了多少.

    :returns: 0 on success, or an error code < 0 on failure.

.. c:function:: int uv_udp_set_membership(uv_udp_t* handle, const char* multicast_addr, const char* interface_addr, uv_membership membership)

    为多播地址设置成员资格

    :param handle: UDP 句柄。应该用 :c:func:`uv_udp_init` 初始化.

    :param multicast_addr: 设置成员资格的多播地址.

    :param interface_addr: 接口地址.

    :param membership: 应该是“UV_JOIN_GROUP”或“UV_LEAVE_GROUP”.

    :returns: 0 on success, or an error code < 0 on failure.

.. c:function:: int uv_udp_set_source_membership(uv_udp_t* handle, const char* multicast_addr, const char* interface_addr, const char* source_addr, uv_membership membership)

    为特定于源的多播组设置成员资格.

    :param handle: UDP 句柄。应该用 :c:func:`uv_udp_init` 初始化.

    :param multicast_addr: 设置成员资格的多播地址.

    :param interface_addr: 接口地址.

    :param source_addr: 源地址.

    :param membership: 应该是“UV_JOIN_GROUP”或“UV_LEAVE_GROUP”.

    :returns: 0 on success, or an error code < 0 on failure.

    .. versionadded:: 1.32.0

.. c:function:: int uv_udp_set_multicast_loop(uv_udp_t* handle, int on)

    设置 IP 多播循环标志。使多播数据包循环回本地套接字.

    :param handle: UDP 句柄。应该用 :c:func:`uv_udp_init` 初始化.

    :param on: 1 for on, 0 for off.

    :returns: 0 on success, or an error code < 0 on failure.

.. c:function:: int uv_udp_set_multicast_ttl(uv_udp_t* handle, int ttl)

    设置多播 ttl.

    :param handle: UDP 句柄。应该用 :c:func:`uv_udp_init` 初始化.

    :param ttl: 1 through 255.

    :returns: 0 on success, or an error code < 0 on failure.

.. c:function:: int uv_udp_set_multicast_interface(uv_udp_t* handle, const char* interface_addr)

    设置组播接口发送或接收数据.

    :param handle: UDP 句柄。应该用 :c:func:`uv_udp_init` 初始化.

    :param interface_addr: 接口地址.

    :returns: 0 on success, or an error code < 0 on failure.

.. c:function:: int uv_udp_set_broadcast(uv_udp_t* handle, int on)

    开启或关闭广播.

    :param handle: UDP 句柄。应该用 :c:func:`uv_udp_init` 初始化.

    :param on: 1 for on, 0 for off.

    :returns: 0 on success, or an error code < 0 on failure.

.. c:function:: int uv_udp_set_ttl(uv_udp_t* handle, int ttl)

    设定生活的时间.

    :param handle: UDP 句柄。应该用 :c:func:`uv_udp_init` 初始化.

    :param ttl: 1 through 255.

    :returns: 0 on success, or an error code < 0 on failure.

.. c:function:: int uv_udp_send(uv_udp_send_t* req, uv_udp_t* handle, const uv_buf_t bufs[], unsigned int nbufs, const struct sockaddr* addr, uv_udp_send_cb send_cb)

    通过 UDP 套接字发送数据。如果套接字之前没有绑定:c:func:`uv_udp_bind`，它将绑定到 0.0.0.0("所有接口" IPv4 地址）和一个随机端口号.

    在 Windows 上, 如果 "addr" 被初始化为指向一个未指定的地址("0.0.0.0" 或 "::"), 它将被更改为指向 "localhost".
    这样做是为了匹配 Linux 系统的行为.

    对于连接的 UDP 句柄，`addr` 必须设置为 `NULL`，否则会返回 `UV_EISCONN` 错误.

    对于无连接的UDP句柄, `addr`不能为 `NULL`, 否则会返回 `UV_EDESTADDRREQ`错误.

    :param req: UDP 请求句柄。不需要初始化.

    :param handle: UDP 句柄。应该用 :c:func:`uv_udp_init` 初始化.

    :param bufs: 要发送的缓冲区列表.

    :param nbufs: `bufs` 中的缓冲区数量.

    :param addr: `struct sockaddr_in` 或 `struct sockaddr_in6` 带有远程对等体的地址和端口.

    :param send_cb: 发送数据时调用的回调.

    :returns: 0 on success, or an error code < 0 on failure.

    .. versionchanged:: 1.19.0 added ``0.0.0.0`` and ``::`` to ``localhost``
        mapping

    .. versionchanged:: 1.27.0 added support for connected sockets

.. c:function:: int uv_udp_try_send(uv_udp_t* handle, const uv_buf_t bufs[], unsigned int nbufs, const struct sockaddr* addr)

    与 :c:func:`uv_udp_send` 相同，但如果无法立即完成，则不会将发送请求排队.

    对于连接的 UDP 句柄，`addr` 必须设置为 `NULL`，否则会返回 `UV_EISCONN` 错误.

    对于无连接的UDP句柄, `addr`不能为`NULL`，否则会返回`UV_EDESTADDRREQ`错误.

    :returns: >= 0: 发送的字节数（它与给定的缓冲区大小匹配）.
        < 0: 负错误码(无法立即发送消息时返回 ``UV_EAGAIN``).

    .. versionchanged:: 1.27.0 added support for connected sockets

.. c:function:: int uv_udp_recv_start(uv_udp_t* handle, uv_alloc_cb alloc_cb, uv_udp_recv_cb recv_cb)

    准备接收数据。如果套接字之前没有绑定:c:func:`uv_udp_bind`，它绑定到 0.0.0.0("所有接口" IPv4 地址）和一个随机端口号.

    :param handle: UDP 句柄。应该用 :c:func:`uv_udp_init` 初始化.

    :param alloc_cb: 需要临时存储时调用的回调.

    :param recv_cb: 回调以调用接收到的数据.

    :returns: 0 on success, or an error code < 0 on failure.

    .. note::
        当使用: man:`recvmmsg(2)` 时，一次接收的消息数量受最大大小 dgram 的数量限制，这些 dgram 将适合在 `alloc_cb` 中分配的缓冲区和 `alloc_cb` 中的 `suggested_size` udp_recv 始终设置为 1 max size dgram 的大小.

    .. versionchanged:: 1.35.0 added support for :man:`recvmmsg(2)` on supported platforms).
                        The use of this feature requires a buffer larger than
                        2 * 64KB to be passed to `alloc_cb`.
    .. versionchanged:: 1.37.0 :man:`recvmmsg(2)` support is no longer enabled implicitly,
                        it must be explicitly requested by passing the `UV_UDP_RECVMMSG` flag to
                        :c:func:`uv_udp_init_ex`.
    .. versionchanged:: 1.39.0 :c:func:`uv_udp_using_recvmmsg` can be used in `alloc_cb` to
                        determine if a buffer sized for use with :man:`recvmmsg(2)` should be
                        allocated for the current handle/platform.

.. c:function:: int uv_udp_using_recvmmsg(uv_udp_t* handle)

    如果 UDP 句柄是使用 `UV_UDP_RECVMMSG` 标志创建的并且平台支持:man:`recvmmsg(2)`，则返回 1, 否则返回 0.

    .. versionadded:: 1.39.0

.. c:function:: int uv_udp_recv_stop(uv_udp_t* handle)

    停止监听传入的数据报.

    :param handle: UDP 句柄。应该用 :c:func:`uv_udp_init` 初始化.

    :returns: 0 on success, or an error code < 0 on failure.

.. c:function:: size_t uv_udp_get_send_queue_size(const uv_udp_t* handle)

    Returns `handle->send_queue_size`.

    .. versionadded:: 1.19.0

.. c:function:: size_t uv_udp_get_send_queue_count(const uv_udp_t* handle)

    Returns `handle->send_queue_count`.

    .. versionadded:: 1.19.0

.. seealso:: The :c:type:`uv_handle_t` API functions also apply.
