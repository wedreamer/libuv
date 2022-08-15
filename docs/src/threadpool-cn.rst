
.. _threadpool:

Thread pool work scheduling
===========================

libuv 提供了一个线程池，可用于运行用户代码并在循环线程中获得通知。此线程池在内部用于运行所有文件系统操作，以及 getaddrinfo 和 getnameinfo 请求.

它的默认大小是 4，但可以在启动时通过将 ``UV_THREADPOOL_SIZE`` 环境变量设置为任何值来更改它（绝对最大值为 1024）.

.. versionchanged:: 1.30.0 the maximum UV_THREADPOOL_SIZE allowed was increased from 128 to 1024.

线程池是全局的，并且在所有事件循环中共享。当特定函数使用线程池时（即使用 :c:func:`uv_queue_work` 时），libuv 预分配并初始化 `UV_THREADPOOL_SIZE` 允许的最大线程数。这会导致相对较小的内存开销（128 个线程约为 1MB），但会提高运行时线程的性能.

.. note::
    请注意，即使使用了在所有事件循环之间共享的全局线程池，这些函数也不是线程安全的.


Data types
----------

.. c:type:: uv_work_t

    Work request type.

.. c:type:: void (*uv_work_cb)(uv_work_t* req)

    回调传递给:c:func:`uv_queue_work`，它将在线程池上运行.

.. c:type:: void (*uv_after_work_cb)(uv_work_t* req, int status)

    回调传递给:c:func:`uv_queue_work`，在线程池上的工作完成后将在循环线程上调用。如果使用 :c:func:`uv_cancel` 取消工作，`status` 将是 `UV_ECANCELED`.


Public members
^^^^^^^^^^^^^^

.. c:member:: uv_loop_t* uv_work_t.loop

    启动此请求的循环以及将报告完成的位置.
    Readonly.

.. seealso:: The :c:type:`uv_req_t` members also apply.


API
---

.. c:function:: int uv_queue_work(uv_loop_t* loop, uv_work_t* req, uv_work_cb work_cb, uv_after_work_cb after_work_cb)

    初始化一个工作请求，它将在线程池中的一个线程中运行给定的 `work_cb`。一旦 `work_cb` 完成，`after_work_cb` 将在循环线程上被调用.

    可以使用 :c:func:`uv_cancel` 取消此请求.

.. seealso:: The :c:type:`uv_req_t` API functions also apply.
