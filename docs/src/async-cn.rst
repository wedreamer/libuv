
.. _async:

:c:type:`uv_async_t` --- Async handle
=====================================

异步句柄允许用户 "唤醒" 事件循环并获得从另一个线程调用的回调。


Data types
----------

.. c:type:: uv_async_t

    Async handle type.

.. c:type:: void (*uv_async_cb)(uv_async_t* handle)

    Type definition for callback passed to :c:func:`uv_async_init`.


Public members
^^^^^^^^^^^^^^

N/A

.. seealso:: The :c:type:`uv_handle_t` members also apply.


API
---

.. c:function:: int uv_async_init(uv_loop_t* loop, uv_async_t* async, uv_async_cb async_cb)

    初始化句柄。 允许 NULL 回调。

    :returns: 0 on success, or an error code < 0 on failure.

    .. note::
        与其他句柄初始化函数不同，它立即启动句柄。

.. c:function:: int uv_async_send(uv_async_t* async)

    唤醒事件循环并调用异步句柄的回调。

    :returns: 0 on success, or an error code < 0 on failure.

    .. note::
        从任何线程调用此函数都是安全的。 回调将在循环线程上调用。

    .. note::
        :c:func:`uv_async_send` is `async-signal-safe <https://man7.org/linux/man-pages/man7/signal-safety.7.html>`_.
        从信号处理程序调用此函数是安全的。

    .. warning::
        libuv 将合并对 :c:func:`uv_async_send` 的调用，也就是说，并非对它的每次调用都会产生回调的执行。 例如：如果 :c:func:`uv_async_send` 在调用回调之前连续调用 5 次，则回调只会被调用一次。 如果在调用回调后再次调用 uv_async_send ，则会再次调用它。

.. seealso::
    The :c:type:`uv_handle_t` API functions also apply.
