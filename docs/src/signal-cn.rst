
.. _signal:

:c:type:`uv_signal_t` --- Signal handle
=======================================

信号句柄在每个事件循环的基础上实现 Unix 风格的信号处理.

Windows notes
-------------

模拟某些信号的接收:

* SIGINT 通常在用户按下 CTRL+C 时传递。但是，就像在 Unix 上一样，它不会在启用终端原始模式时生成.

* 当用户按下 CTRL + BREAK 时传递 SIGBREAK.

* SIGHUP 在用户关闭控制台窗口时生成。在 SIGHUP 上，程序有大约 10 秒的时间来执行清理。之后Windows将无条件终止它.

* 每当 libuv 检测到控制台已调整大小时，都会引发 SIGWINCH。当 libuv 应用程序在控制台模拟器下运行时，或者当 32 位 libuv 应用程序在 64 位系统上运行时，将模拟 SIGWINCH。在这种情况下，SIGWINCH 信号可能并不总是及时传递。对于可写的 :c:type:`uv_tty_t` 句柄，libuv 只会在光标移动时检测大小变化。当使用可读的:c:type:`uv_tty_t`句柄时，只有当句柄处于原始模式并且正在被读取时，才会检测到控制台缓冲区的大小调整。

* 可以成功创建其他信号的观察者，但永远不会收到这些信号。这些信号是： `SIGILL`、 `SIGABRT`、 `SIGFPE`、 `SIGSEGV`、 `SIGTERM`和 `SIGKILL`。

* libuv 未检测到调用 raise() 或 abort() 以编程方式引发信号；这些不会触发信号观察器。

.. versionchanged:: 1.15.0 SIGWINCH support on Windows was improved.
.. versionchanged:: 1.31.0 32-bit libuv SIGWINCH support on 64-bit Windows was
                           rolled back to old implementation.

Unix notes
----------

* SIGKILL 和 SIGSTOP 是无法捕捉到的。

* 通过 libuv 处理 SIGBUS、SIGFPE、SIGILL 或 SIGSEGV 会导致未定义的行为。

* 如果由 `abort()` 生成，则 SIGABRT 不会被 libuv 捕获，例如通过`assert()`。

* 在 Linux 上，NPTL pthreads 库使用 SIGRT0 和 SIGRT1（信号 32 和 33）来管理线程。为这些信号安装观察者会导致不可预测的行为，强烈建议不要这样做。 libuv 的未来版本可能会简单地拒绝它们。


Data types
----------

.. c:type:: uv_signal_t

    Signal handle type.

.. c:type:: void (*uv_signal_cb)(uv_signal_t* handle, int signum)

    传递给 :c:func:`uv_signal_start` 的回调类型定义。


Public members
^^^^^^^^^^^^^^

.. c:member:: int uv_signal_t.signum

    此句柄正在监视的信号。只读.

.. seealso:: The :c:type:`uv_handle_t` members also apply.


API
---

.. c:function:: int uv_signal_init(uv_loop_t* loop, uv_signal_t* signal)

    Initialize the handle.

.. c:function:: int uv_signal_start(uv_signal_t* signal, uv_signal_cb cb, int signum)

    使用给定的回调启动句柄，观察给定的信号.

.. c:function:: int uv_signal_start_oneshot(uv_signal_t* signal, uv_signal_cb cb, int signum)

    .. versionadded:: 1.12.0

    与:c:func:`uv_signal_start` 功能相同，但信号处理程序在接收到信号的那一刻被重置.

.. c:function:: int uv_signal_stop(uv_signal_t* signal)

    停止句柄，回调将不再被调用.

.. seealso:: The :c:type:`uv_handle_t` API functions also apply.
