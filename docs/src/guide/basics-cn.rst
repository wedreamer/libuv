libuv 基础
===============

libuv 强制执行一种 **异步**、 **事件驱动** 风格的编程。 它的核心工作是提供基于事件循环和回调的 I/O 通知和其他活动。 libuv 提供了诸如定时器、非阻塞等核心实用程序网络支持、异步文件系统访问、子进程等。

事件循环
-----------

在事件驱动编程中，应用程序表达对某些事件的兴趣并在它们发生时做出响应。 收集事件的责任来自操作系统或监视其他事件源的处理由libuv，并且用户可以注册回调以在事件发生时调用。事件循环通常 *永远* 运行。 在伪代码中：

.. code-block:: python

    while there are still events to process:
        e = get the next event
        if there is a callback associated with e:
            call the callback

一些事件的例子如下:

* 文件已准备好写入
* socket 已准备好读取数据
* 计时器已超时

此事件循环由 `uv_run()` 封装——使用 libuv 时的最终功能。

系统程序最常见的活动是处理输入和输出，而不是大量的数字运算。 使用常规的问题输入/输出函数（ ``read``、 ``fprintf`` 等）是
**阻止**。 实际写入硬盘或从网络读取，需要与处理器的速度相比，时间过长。 这在任务完成之前函数不会返回，因此您的程序正在执行没有什么。 对于需要高性能的程序，这是一个主要障碍因为其他活动和其他 I/O 操作一直在等待。

标准解决方案之一是使用线程。 每个阻塞 I/O 操作都是在单独的线程（或线程池）中启动。 当阻塞函数在线程中被调用，操作系统可以调度另一个线程运行，这实际上需要CPU。

libuv所遵循的方法使用了另一种风格，即 **异步，非阻塞** 风格。大多数现代操作系统都提供事件通知子系统。例如，对套接字的正常 ``read`` 调用将阻塞，直到发件人实际上发送了一些东西。相反，应用程序可以请求操作系统监视套接字并在队列。应用程序可以在方便时检查事件（也许做在最大限度地使用处理器之前计算一些数字）并抓住数据。它是 **异步**，因为应用程序对一个表示感兴趣点，然后在另一个点（时间和空间）使用数据。这是 **非阻塞** 因为应用程序进程可以自由地执行其他任务。这非常适合 libuv 的事件循环方法，因为操作系统
事件可以被视为另一个 libuv 事件。非阻塞确保其他事件可以在 [#]_ 出现时继续处理。

.. NOTE::

    I/O 如何在后台运行不是我们关心的问题，而是由于我们计算机硬件的工作方式，以线程为基本单元处理器、libuv 和操作系统通常会运行后台/工作线程和/或轮询以非阻塞方式执行任务。

Bert Belder，libuv 核心开发者之一有一个小视频解释 libuv 的架构及其背景。 如果您之前没有相关经验无论是 libuv 还是 libev，它都是一款快速实用的观察。

libuv 的事件循环在 `documentation
<https://docs.libuv.org/en/v1.x/design.html#the-i-o-loop>`_.

.. raw:: html

    <iframe width="560" height="315"
    src="https://www.youtube-nocookie.com/embed/nGn60vDSxQ4" frameborder="0"
    allowfullscreen></iframe>

Hello World
-----------

有了基础知识，让我们编写我们的第一个 libuv 程序。 它确实什么都没有，除了启动一个将立即退出的循环。

.. rubric:: helloworld/main.c
.. literalinclude:: ../../code/helloworld/main.c
    :language: c
    :linenos:

该程序立即退出，因为它没有要处理的事件。 一个libuv 必须告诉事件循环注意使用各种 API 的事件功能。

从 libuv v1.0 开始，用户应该在之前为循环分配内存用 ``uv_loop_init(uv_loop_t *)`` 初始化它。 这允许您插入自定义内存管理。 请记住使用取消初始化循环 ``uv_loop_close(uv_loop_t *)`` 然后删除存储。 例子从不关闭循环，因为程序在循环结束后退出并且系统将回收内存。 生产级项目，尤其是长期运行的系统程序，应注意正确发布。

Default loop
++++++++++++

libuv 提供了一个默认循环，可以使用 ``uv_default_loop()``。 如果你只想要一个，你应该使用这个循环环形。

.. rubric:: default-loop/main.c
.. literalinclude:: ../../code/default-loop/main.c
    :language: c
    :linenos:

.. note::

    node.js 使用默认循环作为其主循环。 如果您正在编写绑定你应该意识到这一点。

.. _libuv-error-handling:

错误处理
--------------

可能失败的初始化函数或同步函数在出错时返回负数。 可能失败的异步函数会将状态参数传递给它们的回调。 错误消息被定义为 ``UV_E*`` `constants`_。

.. _constants: https://docs.libuv.org/en/v1.x/errors.html#error-constants

您可以使用 ``uv_strerror(int)`` 和 ``uv_err_name(int)`` 函数获取分别描述错误或错误名称的 ``const char *``。

I/O 读取回调（例如文件和套接字）被传递一个参数 ``nread`` 。 如果 ``nread`` 小于 0，则存在错误（UV_EOF 是文件结束错误，您可能希望以不同的方式处理）。

句柄和请求
--------------------

libuv 由用户表达对特定事件的兴趣而工作。 这是通常通过为 I/O 设备、计时器或进程创建一个 **句柄** 来完成。句柄是命名为 ``uv_TYPE_t`` 的不透明结构，其中 type 表示手柄的用途。

.. rubric:: libuv watchers
.. code-block:: c

    /* Handle types. */
    typedef struct uv_loop_s uv_loop_t;
    typedef struct uv_handle_s uv_handle_t;
    typedef struct uv_dir_s uv_dir_t;
    typedef struct uv_stream_s uv_stream_t;
    typedef struct uv_tcp_s uv_tcp_t;
    typedef struct uv_udp_s uv_udp_t;
    typedef struct uv_pipe_s uv_pipe_t;
    typedef struct uv_tty_s uv_tty_t;
    typedef struct uv_poll_s uv_poll_t;
    typedef struct uv_timer_s uv_timer_t;
    typedef struct uv_prepare_s uv_prepare_t;
    typedef struct uv_check_s uv_check_t;
    typedef struct uv_idle_s uv_idle_t;
    typedef struct uv_async_s uv_async_t;
    typedef struct uv_process_s uv_process_t;
    typedef struct uv_fs_event_s uv_fs_event_t;
    typedef struct uv_fs_poll_s uv_fs_poll_t;
    typedef struct uv_signal_s uv_signal_t;

    /* Request types. */
    typedef struct uv_req_s uv_req_t;
    typedef struct uv_getaddrinfo_s uv_getaddrinfo_t;
    typedef struct uv_getnameinfo_s uv_getnameinfo_t;
    typedef struct uv_shutdown_s uv_shutdown_t;
    typedef struct uv_write_s uv_write_t;
    typedef struct uv_connect_s uv_connect_t;
    typedef struct uv_udp_send_s uv_udp_send_t;
    typedef struct uv_fs_s uv_fs_t;
    typedef struct uv_work_s uv_work_t;
    typedef struct uv_random_s uv_random_t;

    /* None of the above. */
    typedef struct uv_env_item_s uv_env_item_t;
    typedef struct uv_cpu_info_s uv_cpu_info_t;
    typedef struct uv_interface_address_s uv_interface_address_t;
    typedef struct uv_dirent_s uv_dirent_t;
    typedef struct uv_passwd_s uv_passwd_t;
    typedef struct uv_utsname_s uv_utsname_t;
    typedef struct uv_statfs_s uv_statfs_t;


句柄代表长寿命的对象。 此类句柄上的异步操作是使用 **requests** 标识。 请求是短暂的（通常用于跨只有一个回调）并且通常表示一个句柄上的一个 I/O 操作。请求用于保留启动和回调之间的上下文的个人行动。 例如，一个 UDP 套接字表示为一个 ``uv_udp_t``，而个人写入套接字使用一个 ``uv_udp_send_t`` 写入完成后传递给回调的结构。

句柄由相应的方法设置::

    uv_TYPE_init(uv_loop_t *, uv_TYPE_t *)

function.

回调是 libuv 在观察者发生事件时调用的函数感兴趣的事情已经发生了。 应用程序特定的逻辑通常是在回调中实现。 例如，一个 IO 观察者的回调将收到从文件中读取的数据，将在超时时触发计时器回调等上。

Idling
++++++

这是使用空闲句柄的示例。 回调被调用一次事件循环的每一轮。 空闲句柄的用例在 :doc:`utilities`。 让我们用一个空闲的watcher来看看watcher的生命周期看看 `uv_run()` 现在将如何阻塞，因为有一个观察者存在。 空闲的当达到计数并且 `uv_run()`` 退出时观察者停止，因为没有事件观察者处于活动状态。

.. rubric:: idle-basic/main.c
.. literalinclude:: ../../code/idle-basic/main.c
    :language: c
    :emphasize-lines: 6,10,14-17

存储上下文
+++++++++++++++

在基于回调的编程风格中，您经常需要传递一些 `context` ——应用程序特定信息——在调用站点和回调之间。 全部句柄和请求有一个 `void* data` 成员，您可以将其设置为上下文并在回调中回滚。 这是贯穿始终的通用模式 C 库生态系统。 另外 `uv_loop_t` 也有类似的数据成员。

----

.. [#] 当然取决于硬件的容量。
