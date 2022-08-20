线程
=======

等一下？ 为什么我们在线程上？ 事件循环不应该是 **进行 *网络规模编程* 的方式**吗？ 嗯……不。 线程仍然是处理器完成工作的媒介。 因此，线程有时非常有用，即使您可能不得不涉足各种同步原语。

线程在内部用于伪造所有系统调用的异步特性。 libuv 还使用线程允许应用程序异步执行实际上是阻塞的任务，方法是生成一个线程并在完成时收集结果。

今天有两个主要的线程库: Windows 线程实现和 POSIX 的 :man:`pthreads(7)`。 libuv 的线程 API 类似于 pthreads API, 并且通常具有相似的语义。

libuv 的线程设施的一个显着方面是它是 libuv 中一个独立的部分。 虽然其他特性密切依赖于事件循环和回调原则，但线程是完全不可知的，它们会根据需要阻塞，直接通过返回值发出错误信号，并且如第一个示例 <thread-create-example> 所示 ，甚至不需要运行事件循环。

libuv 的线程 API 也非常有限，因为线程的语义和语法在所有平台上都是不同的，具有不同的完整性级别。

本章做出以下假设： **只有一个事件循环，在一个线程（主线程）中运行**。 没有其他线程与事件循环交互（使用 `uv_async_send` 除外）。

线程核心操作
----------------------

这里没有太多内容，你只需使用 uv_thread_create() 启动一个线程，然后使用 uv_thread_join() 等待它关闭。

.. _thread-create-example:

.. rubric:: thread-create/main.c
.. literalinclude:: ../../code/thread-create/main.c
    :language: c
    :linenos:
    :lines: 26-36
    :emphasize-lines: 3-7

.. tip::

    ``uv_thread_t`` 只是 Unix 上 ``pthread_t`` 的别名，但这是一个实现细节，避免依赖它始终为真.

第二个参数是作为线程入口点的函数，最后一个参数是 ``void *`` 参数，可用于将自定义参数传递给线程。 函数 "hare" 现在将在一个单独的线程中运行，由操作系统抢先调度:

.. rubric:: thread-create/main.c
.. literalinclude:: ../../code/thread-create/main.c
    :language: c
    :linenos:
    :lines: 6-14
    :emphasize-lines: 2

与允许目标线程使用第二个参数将值传回调用线程的 `pthread_join()` 不同， `uv_thread_join()` 不这样做。 要发送值，请使用 :ref:`inter-thread-communication`.

同步原语
--------------------------

本节特意简明扼要。 这本书不是关于线程的，所以我在这里只列出 libuv API 中的任何惊喜。 对于其余部分，您可以查看:man:`pthreads(7)` 手册页.

Mutexes
~~~~~~~

互斥函数是到 pthread 等效项的 **直接** 映射.

.. rubric:: libuv mutex functions
.. code-block:: c

    int uv_mutex_init(uv_mutex_t* handle);
    int uv_mutex_init_recursive(uv_mutex_t* handle);
    void uv_mutex_destroy(uv_mutex_t* handle);
    void uv_mutex_lock(uv_mutex_t* handle);
    int uv_mutex_trylock(uv_mutex_t* handle);
    void uv_mutex_unlock(uv_mutex_t* handle);

`uv_mutex_init()`、 `uv_mutex_init_recursive()` 和 `uv_mutex_trylock()` 函数将在成功时返回 0, 否则返回错误代码.

如果 `libuv` 已在启用调试的情况下编译， `uv_mutex_destroy()`、 `uv_mutex_lock()` 和 `uv_mutex_unlock()` 将在出错时 `abort()`。 类似地，如果错误是 *除了* ``EAGAIN`` 或 ``EBUSY`` 之外的任何错误， ``uv_mutex_trylock()`` 将中止.

支持递归互斥锁，但您不应依赖它们。 此外，它们不应该与 ``uv_cond_t`` 变量一起使用.

如果已锁定互斥锁的线程尝试再次锁定它，默认的 BSD 互斥锁实现将引发错误。 例如，像这样的构造::

    uv_mutex_init(a_mutex);
    uv_mutex_lock(a_mutex);
    uv_thread_create(thread_id, entry, (void *)a_mutex);
    uv_mutex_lock(a_mutex);
    // more things here

可用于等到另一个线程初始化一些东西然后解锁 `a_mutex` ，但如果在调试模式下会导致程序崩溃，或者在第二次调用 `uv_mutex_lock()` 时返回错误.

.. note::

    Windows 上的互斥锁总是递归的.

Locks
~~~~~

读写锁是一种更细粒度的访问机制。 两个阅读器可以同时访问共享内存。 当读者持有时，写者可能无法获得锁。 当写者持有锁时，读者或写者可能不会获得锁。 数据库中经常使用读写锁。 这是一个玩具示例.

.. rubric:: locks/main.c - simple rwlocks
.. literalinclude:: ../../code/locks/main.c
    :language: c
    :linenos:
    :emphasize-lines: 13,16,27,31,42,55

运行它并观察读者有时会如何重叠。 如果有多个写入器，调度程序通常会给它们更高的优先级，因此如果添加两个写入器，您会看到两个写入器往往在读取器再次有机会之前先完成.

我们在上面的例子中也使用了屏障，这样主线程就可以等待所有的读者和作者表明他们已经结束了.

Others
~~~~~~

libuv 还支持信号量_、 `条件变量`_ 和障碍_, 其 API 与它们的 pthread 对应物非常相似.

.. _semaphores: https://en.wikipedia.org/wiki/Semaphore_(programming)
.. _condition variables: https://en.wikipedia.org/wiki/Monitor_(synchronization)#Condition_variables_2
.. _barriers: https://en.wikipedia.org/wiki/Barrier_(computer_science)

此外, libuv 提供了一个方便的函数 `uv_once()`。 多个线程可以尝试使用给定的守卫和函数指针调用 `uv_once()`,  **只有第一个获胜，该函数将被调用一次且仅一次**::

    /* Initialize guard */
    static uv_once_t once_only = UV_ONCE_INIT;

    int i = 0;

    void increment() {
        i++;
    }

    void thread1() {
        /* ... work */
        uv_once(once_only, increment);
    }

    void thread2() {
        /* ... work */
        uv_once(once_only, increment);
    }

    int main() {
        /* ... spawn threads */
    }

After all threads are done, ``i == 1``.

.. _libuv-work-queue:

libuv v0.11.11 以后还添加了一个 ``uv_key_t`` 结构和 api_ 用于线程本地存储.

.. _api: http://docs.libuv.org/en/v1.x/threading.html#thread-local-storage

libuv work queue
----------------

`uv_queue_work()` 是一个便利函数，它允许应用程序在单独的线程中运行任务，并在任务完成时触发回调。 一个看似简单的函数，让 `uv_queue_work()` 诱人的是它允许潜在的任何第三方库与事件循环范例一起使用。 当您使用事件循环时， *必须确保在执行 I/O 时不会在循环线程中定期运行的函数阻塞或严重占用 CPU*，因为这意味着循环变慢并且事件不会被执行满负荷处理.

但是，如果您想要响应性（经典的 `每个客户端一个线程` 服务器模型），那么许多现有代码都具有与线程一起使用的阻塞函数（例如，在后台执行 I/O 的例程），并获得 他们使用事件循环库通常涉及滚动您自己的系统，在单独的线程中运行任务。 libuv 只是为此提供了一个方便的抽象。.

这是一个受 `node.js is cancer`_启发的简单示例。 我们将计算斐波那契数，沿途稍微休眠，但在单独的线程中运行它，这样阻塞和 CPU 绑定的任务不会阻止事件循环执行其他活动。.

.. rubric:: queue-work/main.c - lazy fibonacci
.. literalinclude:: ../../code/queue-work/main.c
    :language: c
    :linenos:
    :lines: 17-29

实际的任务功能很简单，没有任何迹象表明它将在单独的线程中运行。 ``uv_work_t`` 结构是线索。 您可以使用“void* data”字段通过它传递任意数据，并使用它与线程进行通信。 但是，如果您在两个线程都在运行时更改内容，请确保您使用了正确的锁.

The trigger is ``uv_queue_work``:

.. rubric:: queue-work/main.c
.. literalinclude:: ../../code/queue-work/main.c
    :language: c
    :linenos:
    :lines: 31-44
    :emphasize-lines: 10

线程函数将在一个单独的线程中启动，传递 uv_work_t 结构，一旦函数返回，将在事件循环运行的线程上调用 *after* 函数。它将传递相同的结构.

为了将包装器写入阻塞库，一个常见的 :ref:`pattern <baton>` 是使用接力棒来交换数据.

从 libuv 版本 `0.9.4` 开始，可以使用附加函数 `uv_cancel()`。 这允许您取消 libuv 工作队列上的任务。 只有 * 尚未开始 * 的任务才能被取消。 如果一个任务*已经开始执行，或者它已经完成执行*, `uv_cancel()` **将失败**.

如果用户请求终止， `uv_cancel()` 对于清理挂起的任务很有用。 例如，音乐播放器可能会排队多个目录以扫描音频文件。 如果用户终止程序，它应该快速退出，而不是等到所有挂起的请求都运行完毕.

让我们修改斐波那契示例来演示 `uv_cancel()`。 我们首先为终止设置了一个信号处理程序。

.. rubric:: queue-cancel/main.c
.. literalinclude:: ../../code/queue-cancel/main.c
    :language: c
    :linenos:
    :lines: 43-

当用户按下 `Ctrl+C` 触发信号时，我们会向所有工作人员发送 `uv_cancel()`。 ``uv_cancel()`` 将为那些已经执行或完成的返回 ``0``。

.. rubric:: queue-cancel/main.c
.. literalinclude:: ../../code/queue-cancel/main.c
    :language: c
    :linenos:
    :lines: 33-41
    :emphasize-lines: 6

对于成功取消的任务，调用 *after* 函数并将 ``status`` 设置为 ``UV_ECANCELED``。

.. rubric:: queue-cancel/main.c
.. literalinclude:: ../../code/queue-cancel/main.c
    :language: c
    :linenos:
    :lines: 28-31
    :emphasize-lines: 2

``uv_cancel()`` 也可以与 ``uv_fs_t`` 和 ``uv_getaddrinfo_t`` 请求一起使用。 对于文件系统系列函数， `uv_fs_t.errorno` 将被设置为 `UV_ECANCELED`。

.. TIP::

    一个设计良好的程序将有办法终止已经开始执行的长时间运行的工作人员。 这样的工作人员可以定期检查只有主进程设置为发出终止信号的变量。

.. _inter-thread-communication:

线程间通信
--------------------------

有时，您希望各个线程在 *运行时* 实际相互发送消息。 例如，您可能正在一个单独的线程中运行一些持续时间较长的任务（可能使用 `uv_queue_work`），但希望将进度通知给主线程。 这是一个让下载管理器通知用户正在运行的下载状态的简单示例。

.. rubric:: progress/main.c
.. literalinclude:: ../../code/progress/main.c
    :language: c
    :linenos:
    :lines: 7-8,35-
    :emphasize-lines: 2,11

异步线程通信 *在循环上*，因此尽管任何线程都可以是消息发送者，但只有带有 libuv 循环的线程才能成为接收者（或者更确切地说，循环是接收者）。 每当收到消息时，libuv 都会使用异步观察器调用回调（ ``print_progress`` ）。

.. warning::

    重要的是要意识到，由于消息发送是 *async*，回调可能会在另一个线程中调用 uv_async_send 之后立即调用，或者它可能会在一段时间后调用。 libuv 也可以组合多次调用 uv_async_send 并且只调用一次你的回调。 libuv 做出的唯一保证是——回调函数在调用 uv_async_send 之后至少被调用一次。 如果您没有挂起的 uv_async_send 调用，则不会调用回调。 如果你进行了两次或更多的调用，而 libuv 还没有机会运行回调，它 *可能* 调用你的回调 * 一次 * 用于 ``uv_async_send`` 的多次调用。 您的回调永远不会因为一个事件而被调用两次。

.. rubric:: progress/main.c
.. literalinclude:: ../../code/progress/main.c
    :language: c
    :linenos:
    :lines: 10-24
    :emphasize-lines: 7-8

在下载函数中，我们修改了进度指示器，并使用 uv_async_send 将消息排队等待传递。 请记住： `uv_async_send` 也是非阻塞的，并且会立即返回。

.. rubric:: progress/main.c
.. literalinclude:: ../../code/progress/main.c
    :language: c
    :linenos:
    :lines: 31-34

回调是标准的 libuv 模式，从观察者中提取数据。

最后，重要的是要记住清理观察者。

.. rubric:: progress/main.c
.. literalinclude:: ../../code/progress/main.c
    :language: c
    :linenos:
    :lines: 26-29
    :emphasize-lines: 3

在这个展示了 `data` 字段滥用的例子之后，bnoordhuis_ 指出使用 `data` 字段不是线程安全的， `uv_async_send()` 实际上只是为了唤醒 事件循环。 使用互斥锁或 rwlock 确保以正确的顺序执行访问。

.. note::

    mutexes 和 rwlocks **DO NOT** 在信号处理程序中工作，而 ``uv_async_send`` 可以。

需要 uv_async_send 的一种用例是与需要线程亲和功能的库进行互操作时。 例如，在 node.js 中，v8 引擎实例、上下文及其对象绑定到启动 v8 实例的线程。与来自另一个线程的 v8 数据结构交互可能会导致未定义的结果。 现在考虑一些绑定第三方库的 node.js 模块。 它可能是这样的：:

1. 在 node 中，第三方库设置了一个 JavaScript 回调，以便调用以获取更多信息::

    var lib = require('lib');
    lib.on_progress(function() {
        console.log("Progress");
    });

    lib.do();

    // do other stuff

2. ``lib.do`` 应该是非阻塞的，但第三方库是阻塞的，所以绑定使用 `uv_queue_work`.

3. 在单独的线程中完成的实际工作想要调用进度回调，但不能直接调用 v8 与 JavaScript 交互。 所以它使用 `uv_async_send`.

4. async 回调，在主循环线程中调用，也就是 v8 线程，然后与 v8 交互调用 JavaScript 回调.

----

.. _node.js is cancer: http://widgetsandshit.com/teddziuba/2011/10/node-js-is-cancer.html
.. _bnoordhuis: https://github.com/bnoordhuis
