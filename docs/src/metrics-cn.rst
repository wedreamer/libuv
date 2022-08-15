
.. _metrics:

审计操作
======================

libuv 提供了一个度量 API 来跟踪事件循环在内核事件提供程序中空闲的时间量.

API
---

.. c:function:: uint64_t uv_metrics_idle_time(uv_loop_t* loop)

    在内核的事件提供程序中检索事件循环空闲的时间量（例如 ``epoll_wait``）。 调用是线程安全的.

    返回值是从 :c:type:`uv_loop_t` 配置为收集空闲时间开始，在内核的事件提供程序中花费的累计空闲时间.

    .. note::
        事件循环不会开始累积事件提供者的空闲时间，直到使用 :c:type:`UV_METRICS_IDLE_TIME` 调用 :c:type:`uv_loop_configure`.

    .. versionadded:: 1.39.0
