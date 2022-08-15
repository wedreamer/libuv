
.. _timer:

:c:type:`uv_timer_t` --- Timer handle
=====================================

计时器句柄用于安排将来调用的回调.


Data types
----------

.. c:type:: uv_timer_t

    Timer handle type.

.. c:type:: void (*uv_timer_cb)(uv_timer_t* handle)

    Type definition for callback passed to :c:func:`uv_timer_start`.


Public members
^^^^^^^^^^^^^^

N/A

.. seealso:: The :c:type:`uv_handle_t` members also apply.


API
---

.. c:function:: int uv_timer_init(uv_loop_t* loop, uv_timer_t* handle)

    Initialize the handle.

.. c:function:: int uv_timer_start(uv_timer_t* handle, uv_timer_cb cb, uint64_t timeout, uint64_t repeat)

    启动计时器。 `timeout` 和 `repeat` 以毫秒为单位。

    如果 `timeout` 为零，则回调在下一次事件循环迭代时触发.
    如果 `repeat` 不为零，则回调在 `timeout` 毫秒后首先触发，然后在 `repeat` 毫秒后重复.

    .. note::
        不更新事件循环的“现在”概念。更多信息参见:c:func:`uv_update_time`.

        如果计时器已经处于活动状态，则只需对其进行更新.

.. c:function:: int uv_timer_stop(uv_timer_t* handle)

    停止计时器，不再调用回调.

.. c:function:: int uv_timer_again(uv_timer_t* handle)

    停止计时器，如果它正在重复，则使用重复值作为超时重新启动它。如果计时器在返回 UV_EINVAL 之前从未启动过.

.. c:function:: void uv_timer_set_repeat(uv_timer_t* handle, uint64_t repeat)

    以毫秒为单位设置重复间隔值。计时器将被安排在给定的时间间隔上运行，无论回调执行持续时间如何，并且在时间片溢出的情况下将遵循正常的计时器语义.

    例如，如果一个 50ms 的重复计时器首先运行 17ms，它将被安排在 33ms 之后再次运行。如果其他任务在第一次定时器回调之后消耗超过 33ms，则回调将尽快运行.

    .. note::
        如果重复值是从计时器回调中设置的，它不会立即生效.
        如果计时器之前不重复，它将被停止。如果它是重复的，那么旧的重复值将被用于安排下一次超时.

.. c:function:: uint64_t uv_timer_get_repeat(const uv_timer_t* handle)

    获取定时器重复值.

.. c:function:: uint64_t uv_timer_get_due_in(const uv_timer_t* handle)

    获取计时器到期值或 0 如果它已过期。时间是相对于:c:func:`uv_now()`.

    .. versionadded:: 1.40.0

.. seealso:: The :c:type:`uv_handle_t` API functions also apply.
