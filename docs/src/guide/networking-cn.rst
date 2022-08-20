网络
==========

libuv 中的网络与直接使用 BSD 套接字没有太大区别接口，有些事情更容易，都是非阻塞的，但概念仍然存在相同。 此外, libuv 提供了实用功能来抽象烦人的，重复性和低级任务，例如使用 BSD 套接字设置套接字结构、DNS 查找和调整各种套接字参数。

``uv_tcp_t`` 和 ``uv_udp_t`` 结构用于网络 I/O。

.. NOTE::

  本章中的代码示例用于展示某些 libuv API。 它们不是优质代码的示例。 它们泄漏内存并且并不总是正确关闭连接。

TCP
---

TCP 是一种面向连接的流协议，因此基于 libuv 流基础设施。

Server
++++++

服务器套接字通过以下方式进行:

1. ``uv_tcp_init`` the TCP handle.
2. ``uv_tcp_bind`` it.
3. 每当客户端建立新连接时，在句柄上调用 ``uv_listen`` 以调用回调。
4. 使用 ``uv_accept`` 接受连接.
5. 使用 :ref:`stream 操作 <buffers-and-streams>` 与客户端通信。

这是一个简单的 echo 服务器

.. rubric:: tcp-echo-server/main.c - The listen socket
.. literalinclude:: ../../code/tcp-echo-server/main.c
    :language: c
    :linenos:
    :lines: 68-
    :emphasize-lines: 4-5,7-10

您可以看到实用函数 ``uv_ip4_addr`` 用于将人类可读的 IP 地址、端口对转换为 BSD 套接字 API 所需的 sockaddr_in 结构。 可以使用 `uv_ip4_name` 获得相反的结果。

.. NOTE::

    ip4 函数有 ``uv_ip6_*`` 类似方法.

大多数设置函数都是同步的，因为它们受 CPU 限制。 ``uv_listen`` 是我们返回 libuv 回调样式的地方。 第二个参数是积压队列——排队连接的最大长度

当客户端发起连接时，需要回调来为客户端套接字设置句柄并使用 uv_accept 关联句柄。 在这种情况下，我们还建立了从该流中读取的兴趣。

.. rubric:: tcp-echo-server/main.c - Accepting the client
.. literalinclude:: ../../code/tcp-echo-server/main.c
    :language: c
    :linenos:
    :lines: 51-66
    :emphasize-lines: 9-10

其余的函数集与流示例非常相似，可以在代码中找到。 只要记住在不需要套接字时调用 uv_close 即可。 如果您对接受连接不感兴趣，这甚至可以在 ``uv_listen`` 回调中完成。

Client
++++++

在服务器上执行 bind/listen/accept 的地方，在客户端只需调用 `uv_tcp_connect`。 `uv_tcp_connect` 使用了与 `uv_listen` 相同的 `uv_connect_cb` 样式回调。 尝试::

    uv_tcp_t* socket = (uv_tcp_t*)malloc(sizeof(uv_tcp_t));
    uv_tcp_init(loop, socket);

    uv_connect_t* connect = (uv_connect_t*)malloc(sizeof(uv_connect_t));

    struct sockaddr_in dest;
    uv_ip4_addr("127.0.0.1", 80, &dest);

    uv_tcp_connect(connect, socket, (const struct sockaddr*)&dest, on_connect);

其中 ``on_connect`` 将在连接建立后被调用。 回调接收 ``uv_connect_t`` 结构，它有一个成员 ``.handle`` 指向套接字.

UDP
---

`用户数据报协议`_ 提供无连接、不可靠的网络通信。 因此 libuv 不提供流。 相反, libuv 通过 `uv_udp_t` 句柄（用于接收）和 `uv_udp_send_t` 请求（用于发送）和相关函数提供非阻塞 UDP 支持。 也就是说，用于读/写的实际 API 与普通流读取非常相似。 为了了解如何使用 UDP, 该示例显示了从 `DHCP`_ 服务器获取 IP 地址的第一阶段——DHCP 发现.

.. note::

    您必须以 **root** 身份运行 `udp-dhcp`，因为它使用低于 1024 的众所周知的端口号.

.. rubric:: udp-dhcp/main.c - Setup and send UDP packets
.. literalinclude:: ../../code/udp-dhcp/main.c
    :language: c
    :linenos:
    :lines: 7-11,104-
    :emphasize-lines: 8,10-11,17-18,21

.. note::

    IP 地址 `0.0.0.0` 用于绑定所有接口。 IP 地址 `255.255.255.255` 是一个广播地址，意味着数据包将被发送到子网上的所有接口。 端口 ``0`` 表示操作系统随机分配一个端口.

首先，我们将接收套接字设置为绑定到端口 68(DHCP 客户端)上的所有接口并开始对其进行读取。 这将从任何回复的 DHCP 服务器读回响应。 我们使用 UV_UDP_REUSEADDR 标志与在同一端口上运行在这台计算机上的任何其他系统 DHCP 客户端配合使用。然后我们设置一个类似的发送套接字并使用 uv_udp_send 在端口 67(DHCP 服务器)上发送一个 *广播消息*。.

**必须** 设置广播标志，否则你会得到一个 ``EACCES``错误[#]_。 发送的确切消息与本书无关，如果您有兴趣可以研究代码。 像往常一样，如果出现问题，读取和写入回调将收到 < 0 的状态代码。

由于 UDP 套接字未连接到特定对等方，因此读取回调会接收有关数据包发送者的额外参数.

如果没有更多数据要读取， `nread` 可能为零。 如果 ``addr`` 为 NULL, 表示没有可读取的内容(回调不应该做任何事情)，如果不为 NULL, 则表示在 ``addr`` 处从主机接收到一个空数据报。 如果分配器提供的缓冲区不足以容纳数据，则 ``flags`` 参数可能是 ``UV_UDP_PARTIAL``。 *在这种情况下，操作系统将丢弃不适合的数据* (这就是你的 UDP！).

.. rubric:: udp-dhcp/main.c - Reading packets
.. literalinclude:: ../../code/udp-dhcp/main.c
    :language: c
    :linenos:
    :lines: 17-40
    :emphasize-lines: 1,23

UDP 配置
+++++++++++

生存时间
~~~~~~~~~~~~

可以使用 uv_udp_set_ttl 更改在套接字上发送的数据包的 TTL.

仅 IPv6 堆栈
~~~~~~~~~~~~~~~

IPv6 套接字可用于 IPv4 和 IPv6 通信。 如果您只想将套接字限制为 IPv6, 请将 ``UV_UDP_IPV6ONLY`` 标志传递给 ``uv_udp_bind`` [#]_.

组播
~~~~~~~~~

套接字可以（取消）订阅多播组使用:

.. code::block:: c

    int uv_udp_set_membership(uv_udp_t* handle, const char* multicast_addr, const char* interface_addr, uv_membership membership);

where ``membership`` is ``UV_JOIN_GROUP`` or ``UV_LEAVE_GROUP``.

多播的概念在 `本指南` 中得到了很好的解释_.

.. _this guide: https://www.tldp.org/HOWTO/Multicast-HOWTO-2.html

组播包本地环回默认开启[#]_ , 使用 `uv_udp_set_multicast_loop` 关闭.

可以使用更改多播数据包的数据包生存时间 ``uv_udp_set_multicast_ttl``.

查询 DNS
------------

libuv 提供异步 DNS 解析。 为此, 它提供了自己的 ``getaddrinfo`` 替代 [#]_。 在回调中，您可以对检索到的地址执行正常的套接字操作。 让我们连接到 Libera.chat 以查看 DNS 解析的示例.

.. rubric:: dns/main.c
.. literalinclude:: ../../code/dns/main.c
    :language: c
    :linenos:
    :lines: 61-
    :emphasize-lines: 12

如果 `uv_getaddrinfo` 返回非零，则设置中出现问题，您的回调根本不会被调用。 所有参数都可以在 uv_getaddrinfo 返回后立即释放。 `hostname`、 `servname` 和 `hints` 结构记录在 `getaddrinfo 手册页 <getaddrinfo_>`_ 中。 回调可以是 ``NULL`` 在这种情况下函数将同步运行.

在解析器回调中，您可以从 struct addrinfo(s) 的链表中选择任何 IP。 这也演示了 `uv_tcp_connect`。 有必要在回调中调用 `uv_freeaddrinfo`.

.. rubric:: dns/main.c
.. literalinclude:: ../../code/dns/main.c
    :language: c
    :linenos:
    :lines: 42-60
    :emphasize-lines: 8,16

libuv 也提供了 `uv_getnameinfo`_ 的反方法.

.. _uv_getnameinfo: http://docs.libuv.org/en/v1.x/dns.html#c.uv_getnameinfo

网络接口
------------------

关于系统网络接口的信息可以通过 libuv 使用 `uv_interface_addresses` 获得。 这个简单的程序只打印出所有界面细节，以便您了解可用的字段。 这是有助于让您的服务在启动时绑定到 IP 地址.

.. rubric:: interfaces/main.c
.. literalinclude:: ../../code/interfaces/main.c
    :language: c
    :linenos:
    :emphasize-lines: 9,17

``is_internal`` 对于环回接口是真的。 注意，如果一个物理接口有多个 IPv4/IPv6 地址，名字会被报多次，每个地址报一次.

.. _c-ares: https://c-ares.haxx.se
.. _getaddrinfo: https://man7.org/linux/man-pages/man3/getaddrinfo.3.html

.. _User Datagram Protocol: https://en.wikipedia.org/wiki/User_Datagram_Protocol
.. _DHCP: https://tools.ietf.org/html/rfc2131

----

.. [#] https://beej.us/guide/bgnet/html/#broadcast-packetshello-world
.. [#] on Windows only supported on Windows Vista and later.
.. [#] https://www.tldp.org/HOWTO/Multicast-HOWTO-6.html#ss6.1
.. [#] libuv use the system ``getaddrinfo`` in the libuv threadpool. libuv
    v0.8.0 and earlier also included c-ares_ as an alternative, but this has been
    removed in v0.9.0.
