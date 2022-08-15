
.. _poll:

:c:type:`uv_poll_t` --- Poll handle
===================================

轮询句柄用于监视文件描述符的可读性、可写性和断开连接，类似于:man:`poll(2)` 的目的.

轮询句柄的目的是启用集成外部库，这些库依赖于事件循环向其发送有关套接字状态更改的信号，例如 c-ares 或 libssh2。 不建议将 uv_poll_t 用于任何其他目的； :c:type:`uv_tcp_t`、:c:type:`uv_udp_t` 等提供了一种比 :c:type:`uv_poll_t` 实现的实现更快、更具可扩展性的实现，尤其是在 Windows 上.

轮询句柄可能偶尔会发出文件描述符可读或可写的信号，即使它不是。 因此，用户在尝试读取或写入 fd 时应始终准备好处理 EAGAIN 或等效项.

同一个套接字有多个活动轮询句柄是不行的，这可能导致 libuv 忙循环或其他故障.

用户不应在活动轮询句柄轮询文件描述符时关闭它。 这可能导致句柄报告错误，但它也可能开始轮询另一个套接字。 但是 fd 可以在调用 :c:func:`uv_poll_stop` 或 :c:func:`uv_close` 后立即安全关闭.

.. note::
    在 Windows 上，只能使用轮询句柄轮询套接字。 在 Unix 上，可以使用 :man:`poll(2)` 接受的任何文件描述符.

.. note::
    在 AIX 上，不支持监视断开连接.

Data types
----------

.. c:type:: uv_poll_t

    Poll handle type.

.. c:type:: void (*uv_poll_cb)(uv_poll_t* handle, int status, int events)

    传递给 :c:func:`uv_poll_start` 的回调类型定义。

.. c:type:: uv_poll_event

    Poll event types

    ::

        enum uv_poll_event {
            UV_READABLE = 1,
            UV_WRITABLE = 2,
            UV_DISCONNECT = 4,
            UV_PRIORITIZED = 8
        };


Public members
^^^^^^^^^^^^^^

N/A

.. seealso:: The :c:type:`uv_handle_t` members also apply.


API
---

.. c:function:: int uv_poll_init(uv_loop_t* loop, uv_poll_t* handle, int fd)

    Initialize the handle using a file descriptor.

    .. versionchanged:: 1.2.2 the file descriptor is set to non-blocking mode.

.. c:function:: int uv_poll_init_socket(uv_loop_t* loop, uv_poll_t* handle, uv_os_sock_t socket)

    使用套接字描述符初始化句柄。 在 Unix 上，这与 :c:func:`uv_poll_init` 相同。 在 Windows 上，它需要一个 SOCKET 句柄.

    .. versionchanged:: 1.2.2 the socket is set to non-blocking mode.

.. c:function:: int uv_poll_start(uv_poll_t* handle, int events, uv_poll_cb cb)

    开始轮询文件描述符。 `events` 是一个位掩码，由 `UV_READABLE`、`UV_WRITABLE`、`UV_PRIORITIZED` 和 `UV_DISCONNECT` 组成。 一旦检测到事件，将调用回调，并将 `status` 设置为 0，并将检测到的事件设置在 `events` 字段中.

    `UV_PRIORITIZED` 事件用于监视 sysfs 中断或 TCP 带外消息.

    `UV_DISCONNECT` 事件是可选的，因为它可能不会被报告并且用户可以随意忽略它，但它可以帮助优化关闭路径，因为可以避免额外的读取或写入调用.

    如果在轮询时发生错误， `status` 将 < 0 并对应于 `UV_E*` 错误代码之一（请参阅:ref:`errors`）。 当句柄处于活动状态时，用户不应关闭套接字。 如果用户仍然这样做，回调 *可能* 被称为报告错误状态，但这 **不** 保证.

    .. note::
        在已经激活的句柄上调用 :c:func:`uv_poll_start` 很好。 这样做将更新正在监视的事件掩码.

    .. note::
        虽然可以设置 `UV_DISCONNECT`，但在 AIX 上不受支持，因此不会在回调中的 `events` 字段上设置.

    .. note::
        如果设置了 “UV_READABLE” 或 “UV_WRITABLE” 事件之一，只要给定的 fd/socket 相应地保持可读或可写，就会再次调用回调。 特别是在以下每种情况下:

        * 回调已被调用，因为套接字变得可读/可写并且回调根本没有对该套接字进行读/写.
        * 回调提交了对套接字的读取，但尚未读取所有可用数据（当设置了 `UV_READABLE` 时）.
        * 回调在套接字上提交了一个写操作，但之后它仍然是可写的（当设置了 `UV_WRITABLE` 时）.
        * 在与此套接字关联的轮询句柄上调用:c:func:`uv_poll_start` 之前，套接字已经变得可读/可写，并且从那时起套接字的状态没有改变.

        在上面列出的所有场景中，套接字保持可读或可写，因此将再次调用回调（取决于位掩码中设置的事件）。 这种行为称为水平触发.

    .. versionchanged:: 1.9.0 Added the `UV_DISCONNECT` event.
    .. versionchanged:: 1.14.0 Added the `UV_PRIORITIZED` event.

.. c:function:: int uv_poll_stop(uv_poll_t* poll)

    停止轮询文件描述符，回调将不再被调用.

    .. note::
        调用 :c:func:`uv_poll_stop` 立即生效：任何挂起的回调也会被取消，即使套接字状态更改通知已经挂起

.. seealso:: The :c:type:`uv_handle_t` API functions also apply.
