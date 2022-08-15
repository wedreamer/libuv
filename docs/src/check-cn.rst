
.. _check:

:c:type:`uv_check_t` --- Check handle
=====================================

检查句柄将在每次循环迭代时运行一次给定的回调，就在轮询 i/o 之后。


Data types
----------

.. c:type:: uv_check_t

    Check handle type.

.. c:type:: void (*uv_check_cb)(uv_check_t* handle)

    传递给 :c:func:`uv_check_start` 的回调类型定义.


Public members
^^^^^^^^^^^^^^

N/A

.. seealso:: The :c:type:`uv_handle_t` members also apply.


API
---

.. c:function:: int uv_check_init(uv_loop_t* loop, uv_check_t* check)

    初始化句柄。 这个函数总是成功的。

    :returns: 0

.. c:function:: int uv_check_start(uv_check_t* check, uv_check_cb cb)

    使用给定的回调启动句柄。 此函数始终成功，除非 `cb` 为 `NULL`.

    :returns: 0 on success, or `UV_EINVAL` when `cb == NULL`.

.. c:function:: int uv_check_stop(uv_check_t* check)

    停止句柄，回调将不再被调用。这个函数总是成功.

    :returns: 0

.. seealso:: The :c:type:`uv_handle_t` API functions also apply.
