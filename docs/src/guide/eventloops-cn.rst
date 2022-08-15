事件循环高级
====================

libuv 提供了对事件循环的相当大的用户控制，您可以实现通过处理多个循环得到有趣的结果。 你也可以嵌入 libuv 的事件循环到另一个基于事件循环的库中——想象一个基于 Qt 的 UI，以及
Qt 的事件循环驱动一个执行密集系统级别的 libuv 后端任务。

终止一个事件循环
~~~~~~~~~~~~~~~~~~~~~~

`uv_stop()` 可用于停止事件循环。 最早的循环将停止运行是*在下一次迭代*，可能更晚。 这意味着事件准备在此循环迭代中处理的仍将是已处理，因此 `uv_stop()` 不能用作终止开关。 当 `uv_stop()`
被调用时，循环 **不会** 在此迭代中阻塞 i/o。 的语义这些东西可能有点难以理解，所以让我们看看``uv_run()`` 所有控制流发生的地方。

.. rubric:: src/unix/core.c - uv_run
.. literalinclude:: ../../../src/unix/core.c
    :language: c
    :linenos:
    :lines: 304-324
    :emphasize-lines: 10,19,21

`stop_flag` 由 `uv_stop()` 设置。 现在调用所有 libuv 回调在事件循环中，这就是为什么在其中调用 `uv_stop()` 仍然会导致循环的这种迭代发生。 首先 libuv 更新计时器，然后运行挂起的计时器、空闲和准备回调，并调用任何挂起的 I/O 回调。 如果你在其中任何一个中调用 `uv_stop()`， `stop_flag` 将被设置。 这会导致 `uv_backend_timeout()` 返回 `0`，即为什么循环不会阻塞 I/O。 另一方面，如果你打电话给 ``uv_stop()`` 在检查处理程序之一中，I/O 已经完成并且没有做作的。

`uv_stop()` 用于在计算结果时关闭循环或
有一个错误，而不必确保所有处理程序都停止一个
一个。

这是一个简单的例子，它停止循环并演示当前如何循环的迭代仍然发生。

.. rubric:: uvstop/main.c
.. literalinclude:: ../../code/uvstop/main.c
    :language: c
    :linenos:
    :emphasize-lines: 11

