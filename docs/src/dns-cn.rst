
.. _dns:

DNS 工具方法
=====================

libuv 提供了 `getaddrinfo` 和 `getnameinfo` 的异步声明。


Data types
----------

.. c:type:: uv_getaddrinfo_t

    `getaddrinfo` request type.

.. c:type:: void (*uv_getaddrinfo_cb)(uv_getaddrinfo_t* req, int status, struct addrinfo* res)

    完成后将使用 getaddrinfo 请求结果调用的回调。 如果它被取消，`status` 将具有 `UV_ECANCELED` 的值.

.. c:type:: uv_getnameinfo_t

    `getnameinfo` request type.

.. c:type:: void (*uv_getnameinfo_cb)(uv_getnameinfo_t* req, int status, const char* hostname, const char* service)

    完成后将使用 getnameinfo 请求结果调用的回调。 如果它被取消，`status` 将具有 `UV_ECANCELED` 的值.


Public members
^^^^^^^^^^^^^^

.. c:member:: uv_loop_t* uv_getaddrinfo_t.loop

    启动此 getaddrinfo 请求的循环以及将报告完成的位置。 只读.

.. c:member:: struct addrinfo* uv_getaddrinfo_t.addrinfo

    指向包含结果的 `struct addrinfo` 的指针。 必须由用户使用 :c:func:`uv_freeaddrinfo` 释放.

    .. versionchanged:: 1.3.0 the field is declared as public.

.. c:member:: uv_loop_t* uv_getnameinfo_t.loop

    启动此 getnameinfo 请求的循环以及将报告完成的位置。 只读.

.. c:member:: char[NI_MAXHOST] uv_getnameinfo_t.host

    包含结果主机的字符数组。 它是空终止的.

    .. versionchanged:: 1.3.0 the field is declared as public.

.. c:member:: char[NI_MAXSERV] uv_getnameinfo_t.service

    包含结果服务的字符数组。 它是空终止的.

    .. versionchanged:: 1.3.0 the field is declared as public.

.. seealso:: The :c:type:`uv_req_t` members also apply.


API
---

.. c:function:: int uv_getaddrinfo(uv_loop_t* loop, uv_getaddrinfo_t* req, uv_getaddrinfo_cb getaddrinfo_cb, const char* node, const char* service, const struct addrinfo* hints)

    Asynchronous :man:`getaddrinfo(3)`.

    节点或服务中的任何一个都可以为 NULL，但不能同时为 NULL.

    `hints` 是指向带有附加地址类型约束的 struct addrinfo 的指针，即 NULL。 有关更多详细信息，请参阅 `man -s 3 getaddrinfo`.

    成功返回 0 或失败时返回错误代码 < 0。 如果成功，回调将在未来的某个时间被调用并使用查找结果，即:

    * status == 0, the res argument points to a valid `struct addrinfo`, or
    * status < 0, the res argument is NULL. See the UV_EAI_* constants.

    调用 :c:function:`uv freeaddrinfo` 释放 addrinfo 结构.

    .. versionchanged:: 1.3.0 the callback parameter is now allowed to be NULL,
                        in which case the request will run **synchronously**.

.. c:function:: void uv_freeaddrinfo(struct addrinfo* ai)

    释放结构 addrinfo。 允许传递 NULL 并且是无操作的.

.. c:function:: int uv_getnameinfo(uv_loop_t* loop, uv_getnameinfo_t* req, uv_getnameinfo_cb getnameinfo_cb, const struct sockaddr* addr, int flags)

    Asynchronous :man:`getnameinfo(3)`.

    成功返回 0 或失败时返回错误代码 < 0。 如果成功，回调将在未来某个时间被调用，并带有查找结果。 有关详细信息，请参阅 `man -s 3 getnameinfo`.

    .. versionchanged:: 1.3.0 the callback parameter is now allowed to be NULL,
                        in which case the request will run **synchronously**.

.. seealso:: The :c:type:`uv_req_t` API functions also apply.
