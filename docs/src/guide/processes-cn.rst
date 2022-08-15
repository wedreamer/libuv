进程
=========

libuv 提供了相当多的子进程管理，抽象了平台差异并允许使用流或命名管道与子进程进行通信。

Unix 中的一个常见习语是每个进程都做一件事并且做得很好。 在这种情况下，一个进程通常会使用多个子进程来完成任务（类似于在 shell 中使用管道）。 与具有线程和共享内存的多进程模型相比，具有消息的多进程模型也可能更容易推理。

反对基于事件的程序的一个常见问题是它们无法利用现代计算机中的多核。 在多线程程序中，内核可以执行调度并将不同的线程分配给不同的内核，从而提高性能。 但是一个事件循环只有一个线程。 解决方法可以是启动多个进程，每个进程运行一个事件循环，每个进程被分配到一个单独的 CPU 内核。

产生子进程
------------------------

最简单的情况是您只想启动一个进程并知道它何时退出。 这是使用 `uv_spawn` 实现的.

.. rubric:: spawn/main.c
.. literalinclude:: ../../code/spawn/main.c
    :language: c
    :linenos:
    :lines: 6-8,15-
    :emphasize-lines: 11,13-17

.. NOTE::

    ``options`` 被隐式初始化为零，因为它是一个全局变量。 如果您将“选项”更改为局部变量，请记住将其初始化为空出所有未使用的字段::

        uv_process_options_t options = {0};

`uv_process_t` 结构仅充当句柄，所有选项都通过 `uv_process_options_t` 设置。 要简单地启动一个进程，您只需要设置 ``file`` 和 ``args`` 字段。 ``file`` 是要执行的程序。 由于 `uv_spawn` 在内部使用 :man:`execvp(3)`，因此无需提供完整路径。 最后，根据基本约定，**参数数组必须比参数数量大一，最后一个元素为 NULL**

在调用 `uv_spawn` 之后，`uv_process_t.pid` 将包含子进程的进程 ID.

将使用 *exit status* 和导致退出的 *signal* 类型调用退出回调.

.. rubric:: spawn/main.c
.. literalinclude:: ../../code/spawn/main.c
    :language: c
    :linenos:
    :lines: 9-12
    :emphasize-lines: 3

进程退出后关闭进程观察器是 **必需**.

改变进程参数
---------------------------

在启动子进程之前，您可以使用 uv_process_options_t 中的字段来控制执行环境。

更改执行目录
++++++++++++++++++++++++++

设置 `uv_process_options_t.cwd` 到对应目录.

设置环境变量
+++++++++++++++++++++++++

`uv_process_options_t.env` 是一个以 null 结尾的字符串数组，每个 `VAR=VALUE` 的形式用于设置进程的环境变量。 将此设置为 `NULL` 以从父（this）进程继承环境

配置标志
++++++++++++

将 uv_process_options_t.flags 设置为以下标志的按位或，修改子进程行为:

* ``UV_PROCESS_SETUID`` - sets the child's execution user ID to ``uv_process_options_t.uid``.
* ``UV_PROCESS_SETGID`` - sets the child's execution group ID to ``uv_process_options_t.gid``.

仅在 Unix 上支持更改 UID/GID，在 Windows 上 `uv_spawn` 将失败并使用 `UV_ENOTSUP`。

* ``UV_PROCESS_WINDOWS_VERBATIM_ARGUMENTS`` - 在 Windows 上不会对 `uv_process_options_t.args` 进行引用或转义。 在 Unix 上被忽略.
* ``UV_PROCESS_DETACHED`` - 在新会话中启动子进程，该会话将在父进程退出后继续运行。 请参阅下面的示例.

进程分离
-------------------

传递标志 `UV_PROCESS_DETACHED` 可用于启动独立于父进程的守护进程或子进程，以便父进程退出不会影响它

.. rubric:: detach/main.c
.. literalinclude:: ../../code/detach/main.c
    :language: c
    :linenos:
    :lines: 9-30
    :emphasize-lines: 12,19

请记住，句柄仍在监视孩子，因此您的程序不会退出。 如果你想要更多 *fire-and-forget*，请使用 `uv_unref()`。

向进程发信号
----------------------------

libuv 包装了 Unix 上的标准 ``kill(2)`` 系统调用，并在 Windows 上实现了一个具有相似语义的系统调用，但有一个警告*：所有 ``SIGTERM``、``SIGINT`` 和 ``SIGKILL`` ，导致进程终止。 `uv_kill` 的签名是::

    uv_err_t uv_kill(int pid, int signum);

对于使用 libuv 启动的进程，您可以使用 ``uv_process_kill`` 代替，它接受 ``uv_process_t`` 作为第一个参数，而不是 pid。 在这种情况下，**记得在观察者上调用** `uv_close`.

信号
-------

libuv 还提供了 Unix 信号的包装器，并带有 `一些 Windows 支持 <http://docs.libuv.org/en/v1.x/signal.html#signal>`_.

使用 uv_signal_init() 初始化句柄并将其与循环相关联。 要侦听该处理程序上的特定信号，请将 uv_signal_start() 与处理程序函数一起使用。 每个处理程序只能与一个信号编号相关联，随后对 uv_signal_start() 的调用会覆盖之前的关联。 使用 `uv_signal_stop()` 停止观看。 这是一个小例子，展示了各种可能性:

.. rubric:: signal/main.c
.. literalinclude:: ../../code/signal/main.c
    :language: c
    :linenos:
    :emphasize-lines: 17-18,27-28

.. NOTE::

    `uv_run(loop, UV_RUN_NOWAIT)` 类似于 `uv_run(loop, UV_RUN_ONCE)`，因为它只处理一个事件。 如果没有未决事件，UV_RUN_ONCE 会阻塞，而 UV_RUN_NOWAIT 将立即返回。 我们使用NOWAIT，这样一个循环就不会因为另一个没有待处理的活动而被饿死.

向进程发送 `SIGUSR1`，您会发现处理程序被调用了 4 次，每个 `uv_signal_t` 调用一次。 处理程序只是停止每个句柄，以便程序退出。 这种对所有处理程序的分派非常有用。 使用多个事件循环的服务器可以确保在终止之前安全保存所有数据，只需通过每个循环添加一个 `SIGINT` 的观察者.

子进程 IO
-----------------

一个正常的、新生成的进程有自己的一组文件描述符，0、1 和 2 分别是 `stdin`、 `stdout` 和 `stderr`。 有时您可能希望与孩子共享文件描述符。 例如，也许您的应用程序启动了一个子命令，并且您希望任何错误都进入日志文件，但忽略 ``stdout`` 。 为此，您希望子级的“stderr”与父级的 stderr 相同。 在这种情况下，libuv 支持 *inheriting* 文件描述符。 在这个示例中，我们调用了测试程序，即:

.. rubric:: proc-streams/test.c
.. literalinclude:: ../../code/proc-streams/test.c
    :language: c

实际的程序 `proc-streams` 运行它，同时只共享 `stderr`。 子进程的文件描述符是使用 `uv_process_options_t` 中的 `stdio` 字段设置的。 首先将 `stdio_count` 字段设置为正在设置的文件描述符的数量。 ``uv_process_options_t.stdio`` 是 ``uv_stdio_container_t`` 的数组，它是:

.. code-block:: c

    typedef struct uv_stdio_container_s {
        uv_stdio_flags flags;

        union {
            uv_stream_t* stream;
            int fd;
        } data;
    } uv_stdio_container_t;

其中标志可以有多个值。 如果不会，请使用 `UV_IGNORE` 用过的。 如果前三个 `stdio` 字段被标记为 `UV_IGNORE`，它们将重定向到 /dev/null.

由于我们想传递现有的描述符，我们将使用 `UV_INHERIT_FD`。 然后我们将 `fd` 设置为 `stderr`.

.. rubric:: proc-streams/main.c
.. literalinclude:: ../../code/proc-streams/main.c
    :language: c
    :linenos:
    :lines: 15-17,27-
    :emphasize-lines: 6,10,11,12

如果你运行 proc-stream 你会看到只有 `This is stderr` 这一行会被显示。 尝试将 `stdout` 标记为被继承并查看输出.

将此重定向应用于流非常简单。 通过将 `flags` 设置为 `UV_INHERIT_STREAM` 并将 `data.stream` 设置为父进程中的流，子进程可以将该流视为标准I/O。 这可以用来实现类似 CGI_ 的东西.

.. _CGI: https://en.wikipedia.org/wiki/Common_Gateway_Interface

示例 CGI 脚本/可执行文件是:

.. rubric:: cgi/tick.c
.. literalinclude:: ../../code/cgi/tick.c
    :language: c

CGI 服务器结合了本章的概念和 :doc:`networking` ，因此每个客户端都会发送十个滴答声，然后关闭该连接.

.. rubric:: cgi/main.c
.. literalinclude:: ../../code/cgi/main.c
    :language: c
    :linenos:
    :lines: 49-63
    :emphasize-lines: 10

这里我们简单地接受 TCP 连接并将套接字 (*stream*) 传递给 ``invoke_cgi_script``.

.. rubric:: cgi/main.c
.. literalinclude:: ../../code/cgi/main.c
    :language: c
    :linenos:
    :lines: 16, 25-45
    :emphasize-lines: 8-9,18,20

CGI 脚本的 `stdout` 被设置为套接字，这样无论我们的滴答脚本打印什么，都会被发送到客户端。 通过使用进程，我们可以将读/写缓冲卸载到操作系统，因此在方便方面这非常好。 请注意，创建流程是一项昂贵的任务.

.. _pipes:

父子进程间通信
----------------

父子可以通过设置 `uv_stdio_container_t.flags` 到 `UV_CREATE_PIPE` 和 `UV_READABLE_PIPE` 或 `UV_WRITABLE_PIPE` 的按位组合创建的管道进行一种或两种方式的通信。 读/写标志是从子进程的角度来看的。 在这种情况下， `uv_stream_t* stream`字段必须设置为指向一个已初始化、未打开的 `uv_pipe_t` 实例.

新的 stdio 管道
+++++++++++++++

`uv_pipe_t` 结构不仅仅代表 `pipe(7)`_ （或 `|`），还支持任何流文件类对象。 在 Windows 上，该描述的唯一对象是“命名管道”_。 在 Unix 上，它可以是任何 `Unix Domain Socket`_，或派生自 `mkfifo(1)`_，或者它实际上可以是 `pipe(7)`_。 当 `uv_spawn` 由于 `UV_CREATE_PIPE` 标志初始化 `uv_pipe_t` 时，它选择创建 `socketpair(2)`_.

这是为了允许多个 libuv 进程通过 IPC 通信。 这将在下面讨论.

.. _pipe(7): https://man7.org/linux/man-pages/man7/pipe.7.html
.. _mkfifo(1): https://man7.org/linux/man-pages/man1/mkfifo.1.html
.. _socketpair(2): https://man7.org/linux/man-pages/man2/socketpair.2.html
.. _Unix Domain Socket: https://man7.org/linux/man-pages/man7/unix.7.html
.. _Named Pipe: https://docs.microsoft.com/en-us/windows/win32/ipc/named-pipes


任意进程 IPC
+++++++++++++++++++++

由于域套接字 [#]_ 可以在文件系统中具有众所周知的名称和位置，因此它们可以用于不相关进程之间的 IPC。 开源桌面环境使用的 D-BUS_ 系统使用域套接字进行事件通知。 当联系人上线或检测到新硬件时，各种应用程序可以做出反应。 MySQL 服务器还运行一个域套接字，客户端可以在该套接字上与之交互。

.. _D-BUS: https://www.freedesktop.org/wiki/Software/dbus

使用域套接字时，通常遵循客户端-服务器模式，套接字的创建者/所有者充当服务器。 在初始设置之后，消息传递与 TCP 没有什么不同，因此我们将重用 echo server 示例.

.. rubric:: pipe-echo-server/main.c
.. literalinclude:: ../../code/pipe-echo-server/main.c
    :language: c
    :linenos:
    :lines: 70-
    :emphasize-lines: 5,10,14

我们将套接字命名为 `echo.sock`，这意味着它将在本地创建目录。 这个套接字现在的行为与 TCP 套接字没有什么不同，只要流 API 是相关的。 您可以使用 `socat`_ 测试此服务器::

    $ socat - /path/to/socket

想要连接到域套接字的客户端将使用::

    void uv_pipe_connect(uv_connect_t *req, uv_pipe_t *handle, const char *name, uv_connect_cb cb);

其中 ``name`` 将是 ``echo.sock`` 或类似的。 在 Unix 系统上，``name`` 必须指向一个有效的文件（例如 ``/tmp/echo.sock``）。 在 Windows 上，``name`` 遵循 ``\\?\pipe\echo.sock`` 格式.

.. _socat: http://www.dest-unreach.org/socat/

通过管道发送文件描述符
+++++++++++++++++++++++++++++++++++

域套接字很酷的一点是，文件描述符可以通过域套接字在进程之间进行交换。 这允许进程将其 I/O 移交给其他进程。 应用程序包括负载平衡服务器、工作进程和其他优化 CPU 使用的方法。 libuv 目前仅支持通过管道发送 **TCP 套接字或其他管道**.

为了演示，我们将查看一个回显服务器实现，它以循环方式将客户端传递给工作进程。 这个程序有点牵强，虽然书中只包含片段，但建议阅读完整代码才能真正理解它.

worker 进程非常简单，因为文件描述符是由 master 交给它的.

.. rubric:: multi-echo-server/worker.c
.. literalinclude:: ../../code/multi-echo-server/worker.c
    :language: c
    :linenos:
    :lines: 7-9,81-
    :emphasize-lines: 6-8

``queue`` 是连接到另一端的主进程的管道，新的文件描述符沿着该管道发送。 将 uv_pipe_init 的 ipc 参数设置为 1 以指示此管道将用于进程间通信，这一点很重要！ 由于 master 会将文件句柄写入 worker 的标准输入，我们使用 `uv_pipe_open` 将管道连接到 `stdin`.

.. rubric:: multi-echo-server/worker.c
.. literalinclude:: ../../code/multi-echo-server/worker.c
    :language: c
    :linenos:
    :lines: 51-79
    :emphasize-lines: 10,15,20

首先我们调用 uv_pipe_pending_count() 来确保有一个句柄可供读取。 如果您的程序可以处理不同类型的句柄，可以使用 uv_pipe_pending_type() 来确定类型。 尽管 ``accept`` 在这段代码中看起来很奇怪，但它实际上是有道理的。 ``accept`` 传统上所做的是从另一个文件描述符（侦听套接字）获取文件描述符（客户端）。 这正是我们在这里所做的。 从 ``queue`` 获取文件描述（ ``client`` ）。 从这一点开始，工人做标准的回声服务器的东西.

现在转到 master，让我们看看如何启动 worker 以允许负载平衡.

.. rubric:: multi-echo-server/main.c
.. literalinclude:: ../../code/multi-echo-server/main.c
    :language: c
    :linenos:
    :lines: 9-13

``child_worker`` 结构包装了进程，以及主进程和单个进程之间的管道.

.. rubric:: multi-echo-server/main.c
.. literalinclude:: ../../code/multi-echo-server/main.c
    :language: c
    :linenos:
    :lines: 51,61-95
    :emphasize-lines: 17,20-21

在设置工人时，我们使用漂亮的 libuv 函数 uv_cpu_info 来获取 CPU 的数量，以便我们可以启动相同数量的工人。 同样重要的是初始化作为 IPC 通道的管道，第三个参数为 1。然后我们指出子进程的 `stdin` 是一个可读的管道（从子进程的角度来看）。 到这里为止，一切都很简单。 工作人员已启动并等待将文件描述符写入其标准输入.

在 `on_new_connection` （TCP 基础设施在 `main()` 中初始化），我们接受客户端套接字并将其传递给循环中的下一个工作程序.

.. rubric:: multi-echo-server/main.c
.. literalinclude:: ../../code/multi-echo-server/main.c
    :language: c
    :linenos:
    :lines: 31-49
    :emphasize-lines: 9,12-13

`uv_write2` 调用处理所有的抽象，它只是将句柄（`client`）作为正确的参数传递的问题。 有了这个，我们的多进程回显服务器就可以运行了.

感谢 Kyle 指出 `uv_write2()` 需要一个非空缓冲区，即使在发送句柄时也是如此.

.. _pointing out: https://github.com/nikhilm/uvbook/issues/56

----

.. [#] In this section domain sockets stands in for named pipes on Windows as
    well.
