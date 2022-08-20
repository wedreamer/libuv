
.. _idle:

:c:type:`uv_idle_t` --- Idle handle
===================================

空闲句柄将在每次循环迭代时运行给定的回调一次, 就在 :c:type:`uv_prepare_t` 句柄之前.

.. note::
    与准备句柄的显着区别是，当有活动的空闲句柄时，循环将执行零超时轮询，而不是阻塞 i/o.

.. warning::
    尽管有这个名字，空闲句柄会在每次循环迭代时调用它们的回调，而不是当循环实际上是 "idle".


Data types
----------

.. c:type:: uv_idle_t

    Idle handle type.

.. c:type:: void (*uv_idle_cb)(uv_idle_t* handle)

    Type definition for callback passed to :c:func:`uv_idle_start`.


Public members
^^^^^^^^^^^^^^

N/A

.. seealso:: The :c:type:`uv_handle_t` members also apply.


API
---

.. c:function:: int uv_idle_init(uv_loop_t* loop, uv_idle_t* idle)

    Initialize the handle. This function always succeeds.

    :returns: 0

.. c:function:: int uv_idle_start(uv_idle_t* idle, uv_idle_cb cb)

    Start the handle with the given callback. This function always succeeds,
    except when `cb` is `NULL`.

    :returns: 0 on success, or `UV_EINVAL` when `cb == NULL`.

.. c:function:: int uv_idle_stop(uv_idle_t* idle)

    停止句柄，回调将不再被调用。这个函数总是成功的。

    :returns: 0

.. seealso:: The :c:type:`uv_handle_t` API functions also apply.
