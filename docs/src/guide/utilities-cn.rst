Utilities
=========

本章列出了对常见任务有用的工具和技术.
`libev 手册页`_ 已经涵盖了一些可以通过简单的 API 更改被 libuv 采用的模式。它还涵盖了 libuv API 的部分内容，这些部分不需要专门针对它们的整章.

Timers
------

定时器在定时器启动后经过一定时间后调用回调. 
libuv 计时器也可以设置为定期调用，而不是只调用一次.

简单的用法是初始化一个观察者并以 "超时" 和可选的 "重复" 启动它.
计时器可以随时停止.

.. code-block:: c

    uv_timer_t timer_req;

    uv_timer_init(loop, &timer_req);
    uv_timer_start(&timer_req, callback, 5000, 2000);

将启动一个重复计时器，它首先在 `uv_timer_start` 执行后启动 5 秒（ `timeout` ），然后每 2 秒重复一次（ ``repeat``）。利用:

.. code-block:: c

    uv_timer_stop(&timer_req);

停止计时器。这也可以在回调中安全使用.

可以随时修改重复间隔::

    uv_timer_set_repeat(uv_timer_t *timer, int64_t repeat);

这将在可能的情况下生效。如果从定时器回调中调用此函数，则意味着:

* 如果计时器不重复，则计时器已经停止. 再次使用 "uv_timer_start".
* 如果计时器正在重复，则下一次超时已被安排，因此在计时器切换到新间隔之前，将再次使用旧的重复间隔.

The utility function::

    int uv_timer_again(uv_timer_t *)

**仅适用于重复计时器**，相当于停止计时器，然后将初始 "超时" 和 "重复" 设置为旧的 "重复" 值来启动它。如果计时器尚未启动，它将失败（错误代码 ``UV_EINVAL``）并返回 -1.

一个实际的计时器示例在 :ref:`reference count section
<reference-count>`.

.. _reference-count:

Event loop reference count
--------------------------

只要有活动的句柄，事件循环就会运行。该系统通过让每个句柄在事件循环启动时增加引用计数并在停止时减少引用计数来工作. 也可以使用手动更改句柄的引用计数::

    void uv_ref(uv_handle_t*);
    void uv_unref(uv_handle_t*);

即使观察者处于活动状态, 这些函数也可用于允许循环退出, 或使用自定义对象保持循环处于活动状态.

后者可以与间隔计时器一起使用。您可能有一个每隔 X 秒运行一次的垃圾收集器，或者您的网络服务可能会定期向其他人发送心跳，但您不希望在所有干净的退出路径或错误场景中停止它们。或者您希望程序在所有其他观察者完成后退出。在这种情况下，只需在创建后立即取消引用计时器，这样如果它是唯一运行的观察者，那么 uv_run 仍然会退出.

这也用于 node.js, 其中一些 libuv 方法被冒泡到 JS API。每个 JS 对象都会创建一个 `uv_handle_t`（所有观察者的超类），并且可以引用/取消引用.

.. rubric:: ref-timer/main.c
.. literalinclude:: ../../code/ref-timer/main.c
    :language: c
    :linenos:
    :lines: 5-8, 17-
    :emphasize-lines: 9

我们初始化垃圾收集器计时器，然后立即 "取消引用" 它.
观察 9 秒后，当 fake_job 完成后，程序自动退出，即使垃圾收集器仍在运行.

Idler pattern
-------------

每个事件循环都会调用一次空闲句柄的回调。空闲回调可用于执行一些非常低优先级的活动。例如，您可以将每日应用程序性能的摘要发送给开发人员，以便在空闲期间进行分析，或者使用应用程序的 CPU 时间来执行 SETI 计算 :) 空闲观察器在 GUI 应用程序中也很有用。假设您正在使用事件循环进行文件下载。如果 TCP 套接字仍在建立中并且没有其他事件存在，您的事件循环将暂停（ **block** ），这意味着您的进度条将冻结并且用户将面临无响应的应用程序。在这种情况下，排队并空闲观察者以保持 UI 可操作.

.. rubric:: idle-compute/main.c
.. literalinclude:: ../../code/idle-compute/main.c
    :language: c
    :linenos:
    :lines: 5-9, 34-
    :emphasize-lines: 13

在这里, 我们初始化空闲观察者并将其与我们感兴趣的实际事件一起排队。现在将重复调用 "crunch_away", 直到用户键入某些内容并按 Return。然后它会在循环处理输入数据时被中断一小段时间, 之后它将继续再次调用空闲回调.

.. rubric:: idle-compute/main.c
.. literalinclude:: ../../code/idle-compute/main.c
    :language: c
    :linenos:
    :lines: 10-19

.. _baton:

Passing data to worker thread
-----------------------------

使用 uv_queue_work 时, 您通常需要将复杂数据传递给工作线程。解决方案是使用 "struct" 并设置 "uv_work_t.data" 指向它。一个细微的变化是将 ``uv_work_t`` 本身作为该结构的第一个成员(称为指挥棒 [#]_).
这允许在一次干净的调用中清理工作请求和所有数据.

.. code-block:: c
    :linenos:
    :emphasize-lines: 2

    struct ftp_baton {
        uv_work_t req;
        char *host;
        int port;
        char *username;
        char *password;
    }

.. code-block:: c
    :linenos:
    :emphasize-lines: 2

    ftp_baton *baton = (ftp_baton*) malloc(sizeof(ftp_baton));
    baton->req.data = (void*) baton;
    baton->host = strdup("my.webhost.com");
    baton->port = 21;
    // ...

    uv_queue_work(loop, &baton->req, ftp_session, ftp_cleanup);

这里我们创建接力棒并将任务排队.

现在任务函数可以提取它需要的数据:

.. code-block:: c
    :linenos:
    :emphasize-lines: 2, 12

    void ftp_session(uv_work_t *req) {
        ftp_baton *baton = (ftp_baton*) req->data;

        fprintf(stderr, "Connecting to %s\n", baton->host);
    }

    void ftp_cleanup(uv_work_t *req) {
        ftp_baton *baton = (ftp_baton*) req->data;

        free(baton->host);
        // ...
        free(baton);
    }

然后我们释放指挥棒，这也释放了观察者.

External I/O with polling
-------------------------

通常第三方库会处理他们自己的 I/O, 并在内部跟踪他们的套接字和其他文件。在这种情况下，无法使用标准流 I/O 操作，但该库仍然可以集成到 libuv 事件循环中。所需要的只是该库允许您访问底层文件描述符并提供根据您的应用程序决定以小增量处理任务的函数。一些库虽然不允许这样的访问，只提供一个标准的阻塞函数，它将执行整个 I/O 事务，然后才返回。在事件循环线程中使用这些是不明智的，请改用:ref:`threadpool`。当然，这也意味着失去对库的精细控制.

libuv 的 ``uv_poll`` 部分只是使用操作系统通知机制监视文件描述符。从某种意义上说, libuv 自己实现的所有 I/O 操作也都由类似 uv_poll 的代码支持。每当操作系统注意到正在轮询的文件描述符中的状态变化时, libuv 将调用相关的回调.

在这里，我们将介绍一个简单的下载管理器，它将使用 libcurl_ 下载文件。我们不会将所有控制权交给 libcurl, 而是使用 libuv 事件循环，并使用非阻塞、异步 multi_ 接口在 libuv 通知 I/O 准备就绪时进行下载.

.. _libcurl: https://curl.haxx.se/libcurl/
.. _multi: https://curl.haxx.se/libcurl/c/libcurl-multi.html

.. rubric:: uvwget/main.c - The setup
.. literalinclude:: ../../code/uvwget/main.c
    :language: c
    :linenos:
    :lines: 1-9,142-
    :emphasize-lines: 7,21,24-25

每个库与 libuv 集成的方式会有所不同。对于 libcurl, 我们可以注册两个回调。每当套接字的状态发生变化并且我们必须开始轮询它时, 都会调用套接字回调“handle_socket”。 ``start_timeout`` 被 libcurl 调用来通知我们下一个超时间隔，之后我们应该驱动 libcurl 前进，不管 I/O 状态如何.
这样 libcurl 就可以处理错误或执行其他任何操作以使下载移动.

Our downloader is to be invoked as::

    $ ./uvwget [url1] [url2] ...

所以我们将每个参数添加为 URL

.. rubric:: uvwget/main.c - Adding urls
.. literalinclude:: ../../code/uvwget/main.c
    :language: c
    :linenos:
    :lines: 39-56
    :emphasize-lines: 13-14

我们让 libcurl 直接将数据写入文件，但如果您愿意，还有更多可能.

``start_timeout`` 将被 libcurl 第一次立即调用，所以事情开始了。这只是启动了一个 libuv `timer <#timers>`_
每当超时时使用 ``CURL_SOCKET_TIMEOUT`` 驱动``curl_multi_socket_action``。 ``curl_multi_socket_action`` 是驱动 libcurl 的东西, 也是我们在套接字更改状态时调用的东西。但在我们开始之前, 我们需要在调用 "handle_socket" 时轮询套接字.

.. rubric:: uvwget/main.c - Setting up polling
.. literalinclude:: ../../code/uvwget/main.c
    :language: c
    :linenos:
    :lines: 102-140
    :emphasize-lines: 9,11,15,21,24

我们对套接字 fd ``s`` 和 ``action`` 感兴趣。对于每个套接字, 如果它不存在, 我们创建一个 "uv_poll_t" 句柄, 并使用“curl_multi_assign”将它与套接字关联。这样, 每当调用回调时, "socketp" 都会指向它.

在下载完成或失败的情况下, libcurl 请求删除轮询。所以我们停止并释放轮询句柄.

根据 libcurl 希望监视的事件, 我们开始使用 "UV_READABLE" 或 "UV_WRITABLE" 进行轮询。现在, 只要套接字准备好读取或写入, libuv 就会调用 poll 回调。在同一个句柄上多次调用 uv_poll_start 是可以接受的，它只会用新值更新事件掩码。 ``curl_perform`` 是这个程序的关键.

.. rubric:: uvwget/main.c - Driving libcurl.
.. literalinclude:: ../../code/uvwget/main.c
    :language: c
    :linenos:
    :lines: 81-95
    :emphasize-lines: 2,6-7,12

我们要做的第一件事是停止计时器，因为在间隔中已经取得了一些进展。然后根据触发回调的事件，我们设置正确的标志。然后我们调用 ``curl_multi_socket_action`` 并带有进展的套接字和通知发生了什么事件的标志。此时, libcurl 以小增量执行其所有内部任务，并将尝试尽快返回，这正是事件程序在其主线程中想要的。 libcurl 不断将有关传输进度的消息排队到自己的队列中。在我们的例子中，我们只对已完成的传输感兴趣。所以我们提取这些消息，并清理完成传输的句柄.

.. rubric:: uvwget/main.c - Reading transfer status.
.. literalinclude:: ../../code/uvwget/main.c
    :language: c
    :linenos:
    :lines: 58-79
    :emphasize-lines: 6,9-10,13-14

Check & Prepare watchers
------------------------

TODO

Loading libraries
-----------------

libuv 提供了一个跨平台 API 来动态加载 "shared libraries"_.
这可以用来实现你自己的插件/扩展/模块系统，并被 node.js 用来实现对绑定的 ``require()`` 支持。只要您的库导出正确的符号，用法就非常简单。加载第三方代码时要小心进行健全性和安全性检查，否则您的程序将出现不可预测的行为。这个例子实现了一个非常简单的插件系统，除了打印插件的名称之外什么都不做.

我们先来看看提供给插件作者的接口.

.. rubric:: plugin/plugin.h
.. literalinclude:: ../../code/plugin/plugin.h
    :language: c
    :linenos:

您可以类似地添加更多功能，插件作者可以使用这些功能在您的应用程序中做有用的事情 [#]_。使用此 API 的示例插件是:

.. rubric:: plugin/hello.c
.. literalinclude:: ../../code/plugin/hello.c
    :language: c
    :linenos:

我们的接口定义了所有插件都应该有一个 "initialize" 函数，该函数将被应用程序调用。这个插件被编译为一个共享库，可以通过运行我们的应用程序来加载::

    $ ./plugin libhello.dylib
    Loading libhello.dylib
    Registered plugin "Hello World!"

.. NOTE::

    共享库文件名会因平台而异。在 Linux 上是 libhello.so.

这是通过使用 uv_dlopen 首先加载共享库 libhello.dylib 来完成的。然后我们使用 uv_dlsym 访问初始化函数并调用它.

.. rubric:: plugin/main.c
.. literalinclude:: ../../code/plugin/main.c
    :language: c
    :linenos:
    :lines: 7-
    :emphasize-lines: 15, 18, 24

`uv_dlopen` 需要一个共享库的路径并设置不透明的 `uv_lib_t` 指针。成功返回 0, 错误返回 -1。使用 ``uv_dlerror`` 获取错误信息.

``uv_dlsym`` 在第三个参数的第二个参数中存储一个指向符号的指针。 ``init_plugin_function`` 是一个函数指针，指向我们在应用程序插件中寻找的那种函数.

.. _shared libraries: https://en.wikipedia.org/wiki/Shared_library#Shared_libraries

TTY
---

文本终端长期以来一直支持基本格式，具有 ”相当标准化的“ 命令集。程序经常使用这种格式来提高终端输出的可读性。例如 ``grep --color``.
libuv 提供了 `uv_tty_t` 抽象（一个流）和相关函数来在所有平台上实现 ANSI 转义码。我的意思是 libuv 将 ANSI 代码转换为 Windows 等效代码，并提供获取终端信息的函数.

.. _pretty standardised: https://en.wikipedia.org/wiki/ANSI_escape_sequences

首先要做的是用它读取/写入的文件描述符初始化一个``uv_tty_t``。这是通过::

    int uv_tty_init(uv_loop_t*, uv_tty_t*, uv_file fd, int unused)

``unused`` 参数现在被自动检测并忽略。它以前需要设置为在流上使用 `uv_read_start()`.

然后最好使用 `uv_tty_set_mode` 将模式设置为 *normal* 以启用大多数 TTY 格式化、流控制和其他设置。其他_模式也可用.

.. _Other: http://docs.libuv.org/en/v1.x/tty.html#c.uv_tty_mode_t

记得在程序退出时调用 uv_tty_reset_mode 来恢复终端的状态。只是礼貌而已。另一套良好的礼仪是注意重定向。如果用户将命令的输出重定向到文件, 则不应编写控制序列, 因为它们会妨碍可读性和 "grep"。要检查文件描述符是否确实是 TTY, 请使用文件描述符调用 "uv_guess_handle" 并将返回值与 "UV_TTY" 进行比较.

这是一个在红色背景上打印白色文本的简单示例:

.. rubric:: tty/main.c
.. literalinclude:: ../../code/tty/main.c
    :language: c
    :linenos:
    :emphasize-lines: 11-12,14,17,27

最终的 TTY 助手是 `uv_tty_get_winsize()`，它用于获取终端的宽度和高度，并在成功时返回 `0`。这是一个使用函数和字符位置转义码做一些动画的小程序.

.. rubric:: tty-gravity/main.c
.. literalinclude:: ../../code/tty-gravity/main.c
    :language: c
    :linenos:
    :emphasize-lines: 19,25,38

转义码是:

======  =======================
Code    Meaning
======  =======================
*2* J    Clear part of the screen, 2 is entire screen
H        Moves cursor to certain position, default top-left
*n* B    Moves cursor down by n lines
*n* C    Moves cursor right by n columns
m        Obeys string of display settings, in this case green background (40+2), white text (30+7)
======  =======================

正如您所看到的, 这对于生成格式良好的输出非常有用, 如果您喜欢的话, 甚至是基于控制台的街机游戏。对于更高级的控制，您可以尝试 `ncurses`_.

.. _ncurses: https://www.gnu.org/software/ncurses/ncurses.html

.. versionchanged:: 1.23.1: the `readable` parameter is now unused and ignored.
                    The appropriate value will now be auto-detected from the kernel.

----

.. [#] I was first introduced to the term baton in this context, in Konstantin
       Käfer's excellent slides on writing node.js bindings --
       https://kkaefer.com/node-cpp-modules/#baton
.. [#] mfp is My Fancy Plugin

.. _libev man page: http://pod.tst.eu/http://cvs.schmorp.de/libev/ev.pod#COMMON_OR_USEFUL_IDIOMS_OR_BOTH
