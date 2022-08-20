文件系统
==========

简单的文件系统读/写是使用 ``uv_fs_*`` 函数和 ``uv_fs_t`` 结构。

.. note::

    libuv 文件系统操作不同于: doc: `socket 操作 <网络>`。 套接字操作使用提供的非阻塞操作由操作系统处理。 而文件系统操作使用阻塞函数在内部，但在 ``线程池`` 中调用这些函数并通知应用程序交互时向事件循环注册的观察者必需的。

.. _thread pool: https://docs.libuv.org/en/v1.x/threadpool.html#thread-pool-work-scheduling

所有文件系统函数都有两种形式—— *sync* 和 *async*。

如果回调为空。 函数的返回值为 :ref:`libuv error code <libuv-error-handling>`。 这通常只对同步调用有用。 *asynchronous* 模式在传递回调并返回时调用值为 0。

读/写文件
---------------------

使用文件描述符获得

.. code-block:: c

    int uv_fs_open(uv_loop_t* loop, uv_fs_t* req, const char* path, int flags, int mode, uv_fs_cb cb)

``flags`` 和 ``mode`` 是标准的 `Unix 标志 <https://man7.org/linux/man-pages/man2/open.2.html>`_. libuv 负责转换为适当的 Windows 标志。

文件描述符使用关闭

.. code-block:: c

    int uv_fs_close(uv_loop_t* loop, uv_fs_t* req, uv_file file, uv_fs_cb cb)


文件系统操作回调具有签名:

.. code-block:: c

    void callback(uv_fs_t* req);

让我们看一下 ``cat`` 的简单实现。 我们从注册开始打开文件时的回调：

.. rubric:: uvcat/main.c - opening a file
.. literalinclude:: ../../code/uvcat/main.c
    :language: c
    :linenos:
    :lines: 41-53
    :emphasize-lines: 4, 6-7

``uv_fs_t`` 的 ``result`` 字段是文件描述符，如果 ``uv_fs_open`` 回调。 如果文件成功打开，我们开始读取它。

.. rubric:: uvcat/main.c - read callback
.. literalinclude:: ../../code/uvcat/main.c
    :language: c
    :linenos:
    :lines: 26-39
    :emphasize-lines: 2,8,12

在读取调用的情况下，您应该传递一个 *initialized* 缓冲区，它将在触发读取回调之前填充数据。 `uv_fs_*` 操作几乎直接映射到某些 POSIX 函数，因此指示 EOF 在这种情况下， ``结果``为 0。在流或管道的情况下,  ``UV_EOF`` 常量将作为状态传递。

在这里，您会看到编写异步程序时的常见模式。 这 ``uv_fs_close()`` 调用是同步执行的。 *通常的任务是一次性，或作为启动或关闭阶段的一部分执行同步，因为我们对程序运行时的快速 I/O 感兴趣关于其主要任务和处理多个 I/O 源*。 对于单人任务性能差异通常可以忽略不计，并可能导致代码更简单。

使用 ``uv_fs_write()`` 写入文件系统同样简单。 *您的回调将在写入完成后触发*。 在我们的例子中，回调只需驱动下一次读取。 因此，读写通过同步进行回调。

.. rubric:: uvcat/main.c - write callback
.. literalinclude:: ../../code/uvcat/main.c
    :language: c
    :linenos:
    :lines: 17-24
    :emphasize-lines: 6

.. warning::

    由于文件系统和磁盘驱动器的性能配置方式， ``成功`` 的写入可能尚未提交到磁盘。

We set the dominos rolling in ``main()``:

.. rubric:: uvcat/main.c
.. literalinclude:: ../../code/uvcat/main.c
    :language: c
    :linenos:
    :lines: 55-
    :emphasize-lines: 2

.. warning::

    `uv_fs_req_cleanup()` 函数必须始终在文件系统上调用请求释放 libuv 中的内部内存分配。

文件系统操作
---------------------

所有标准文件系统操作，如 ``unlink`` 、 ``rmdir``、 ``stat`` 都是异步支持并具有直观的参数顺序。 他们遵循与 read/write/open 调用相同的模式，在
``uv_fs_t.result`` 字段。 完整名单：

.. rubric:: Filesystem operations
.. code-block:: c

    int uv_fs_close(uv_loop_t* loop, uv_fs_t* req, uv_file file, uv_fs_cb cb);
    int uv_fs_open(uv_loop_t* loop, uv_fs_t* req, const char* path, int flags, int mode, uv_fs_cb cb);
    int uv_fs_read(uv_loop_t* loop, uv_fs_t* req, uv_file file, const uv_buf_t bufs[], unsigned int nbufs, int64_t offset, uv_fs_cb cb);
    int uv_fs_unlink(uv_loop_t* loop, uv_fs_t* req, const char* path, uv_fs_cb cb);
    int uv_fs_write(uv_loop_t* loop, uv_fs_t* req, uv_file file, const uv_buf_t bufs[], unsigned int nbufs, int64_t offset, uv_fs_cb cb);
    int uv_fs_copyfile(uv_loop_t* loop, uv_fs_t* req, const char* path, const char* new_path, int flags, uv_fs_cb cb);
    int uv_fs_mkdir(uv_loop_t* loop, uv_fs_t* req, const char* path, int mode, uv_fs_cb cb);
    int uv_fs_mkdtemp(uv_loop_t* loop, uv_fs_t* req, const char* tpl, uv_fs_cb cb);
    int uv_fs_mkstemp(uv_loop_t* loop, uv_fs_t* req, const char* tpl, uv_fs_cb cb);
    int uv_fs_rmdir(uv_loop_t* loop, uv_fs_t* req, const char* path, uv_fs_cb cb);
    int uv_fs_scandir(uv_loop_t* loop, uv_fs_t* req, const char* path, int flags, uv_fs_cb cb);
    int uv_fs_scandir_next(uv_fs_t* req, uv_dirent_t* ent);
    int uv_fs_opendir(uv_loop_t* loop, uv_fs_t* req, const char* path, uv_fs_cb cb);
    int uv_fs_readdir(uv_loop_t* loop, uv_fs_t* req, uv_dir_t* dir, uv_fs_cb cb);
    int uv_fs_closedir(uv_loop_t* loop, uv_fs_t* req, uv_dir_t* dir, uv_fs_cb cb);
    int uv_fs_stat(uv_loop_t* loop, uv_fs_t* req, const char* path, uv_fs_cb cb);
    int uv_fs_fstat(uv_loop_t* loop, uv_fs_t* req, uv_file file, uv_fs_cb cb);
    int uv_fs_rename(uv_loop_t* loop, uv_fs_t* req, const char* path, const char* new_path, uv_fs_cb cb);
    int uv_fs_fsync(uv_loop_t* loop, uv_fs_t* req, uv_file file, uv_fs_cb cb);
    int uv_fs_fdatasync(uv_loop_t* loop, uv_fs_t* req, uv_file file, uv_fs_cb cb);
    int uv_fs_ftruncate(uv_loop_t* loop, uv_fs_t* req, uv_file file, int64_t offset, uv_fs_cb cb);
    int uv_fs_sendfile(uv_loop_t* loop, uv_fs_t* req, uv_file out_fd, uv_file in_fd, int64_t in_offset, size_t length, uv_fs_cb cb);
    int uv_fs_access(uv_loop_t* loop, uv_fs_t* req, const char* path, int mode, uv_fs_cb cb);
    int uv_fs_chmod(uv_loop_t* loop, uv_fs_t* req, const char* path, int mode, uv_fs_cb cb);
    int uv_fs_utime(uv_loop_t* loop, uv_fs_t* req, const char* path, double atime, double mtime, uv_fs_cb cb);
    int uv_fs_futime(uv_loop_t* loop, uv_fs_t* req, uv_file file, double atime, double mtime, uv_fs_cb cb);
    int uv_fs_lutime(uv_loop_t* loop, uv_fs_t* req, const char* path, double atime, double mtime, uv_fs_cb cb);
    int uv_fs_lstat(uv_loop_t* loop, uv_fs_t* req, const char* path, uv_fs_cb cb);
    int uv_fs_link(uv_loop_t* loop, uv_fs_t* req, const char* path, const char* new_path, uv_fs_cb cb);
    int uv_fs_symlink(uv_loop_t* loop, uv_fs_t* req, const char* path, const char* new_path, int flags, uv_fs_cb cb);
    int uv_fs_readlink(uv_loop_t* loop, uv_fs_t* req, const char* path, uv_fs_cb cb);
    int uv_fs_realpath(uv_loop_t* loop, uv_fs_t* req, const char* path, uv_fs_cb cb);
    int uv_fs_fchmod(uv_loop_t* loop, uv_fs_t* req, uv_file file, int mode, uv_fs_cb cb);
    int uv_fs_chown(uv_loop_t* loop, uv_fs_t* req, const char* path, uv_uid_t uid, uv_gid_t gid, uv_fs_cb cb);
    int uv_fs_fchown(uv_loop_t* loop, uv_fs_t* req, uv_file file, uv_uid_t uid, uv_gid_t gid, uv_fs_cb cb);
    int uv_fs_lchown(uv_loop_t* loop, uv_fs_t* req, const char* path, uv_uid_t uid, uv_gid_t gid, uv_fs_cb cb);
    int uv_fs_statfs(uv_loop_t* loop, uv_fs_t* req, const char* path, uv_fs_cb cb);


.. _buffers-and-streams:

缓冲区和流
-------------------

libuv 中的基本 I/O 句柄是流（ ``uv_stream_t`` ）。 TCP 套接字、UDP用于文件 I/O 和 IPC 的套接字和管道都被视为流子类。

使用每个子类的自定义函数初始化流，然后进行操作使用时

.. code-block:: c

    int uv_read_start(uv_stream_t*, uv_alloc_cb alloc_cb, uv_read_cb read_cb);
    int uv_read_stop(uv_stream_t*);
    int uv_write(uv_write_t* req, uv_stream_t* handle,
                 const uv_buf_t bufs[], unsigned int nbufs, uv_write_cb cb);

基于流的函数比文件系统函数更易于使用，并且当 `uv_read_start()` 调用时, libuv 将自动继续从流中读取调用一次，直到 `uv_read_stop()` 被调用。

数据的离散单元是缓冲区—— ``uv_buf_t``。 这简直指向字节的指针（ ``uv_buf_t.base`` ）和长度的集合（ ``uv_buf_t.len`` ）。 `uv_buf_t` 是轻量级的, 并按值传递。需要管理的是必须分配的实际字节并由应用程序释放。

.. ERROR::

    **这个程序并不总是有效，需要更好的东西**

为了演示流，我们需要使用 `uv_pipe_t`。 这允许流式传输本地文件 [#]_. 这是一个使用 libuv 的简单 tee 实用程序。 执行所有操作异步显示事件 I/O 的强大功能。 两次写入不会分别阻塞其他，但我们必须小心复制缓冲区数据以确保我们不会释放一个缓冲区，直到它被写入。

该程序将被执行为::

    ./uvtee <output_file>

我们开始在我们需要的文件上打开管道。 指向文件的 libuv 管道是默认打开为双向。

.. rubric:: uvtee/main.c - read on pipes
.. literalinclude:: ../../code/uvtee/main.c
    :language: c
    :linenos:
    :lines: 62-80
    :emphasize-lines: 4,5,15

`uv_pipe_init()` 的第三个参数应该设置为 1 用于使用命名的 IPC 管道。 这在 :doc:`processes` 中有介绍。 `uv_pipe_open()` 调用将管道与文件描述符相关联，在本例中为 ``0``（标准
输入）。

我们开始监控 ``stdin``。 ``alloc_buffer`` 回调被调用为新的需要缓冲区来保存传入的数据。 ``read_stdin`` 将被调用这些缓冲区。

.. rubric:: uvtee/main.c - reading buffers
.. literalinclude:: ../../code/uvtee/main.c
    :language: c
    :linenos:
    :lines: 19-22,44-60

标准的 malloc 在这里就足够了，但是你可以使用任何内存分配方案。 例如, node.js 使用它自己的 slab 分配器来关联带有 V8 对象的缓冲区。

读取回调 ``nread`` 参数在任何错误时都小于 0。 这个错误可能是 EOF, 在这种情况下，我们使用通用关闭关闭所有流函数 `uv_close()` 根据其内部类型处理句柄。否则 ``nread`` 是一个非负数，我们可以尝试写输出流的许多字节。 最后记住缓冲区分配和释放是应用程序的责任，所以我们释放数据。

如果分配回调失败，它可能会返回长度为零的缓冲区分配内存。 在这种情况下，读取回调被错误调用 UV_ENOBUFS。 libuv 将继续尝试读取流，所以你如果你想在分配失败时停止，必须显式调用 uv_close() 。

读回调可以用 ``nread = 0`` 调用，表明在这个点没有什么可读的。 大多数应用程序会忽略这一点。

.. rubric:: uvtee/main.c - Write to pipe
.. literalinclude:: ../../code/uvtee/main.c
    :language: c
    :linenos:
    :lines: 9-13,23-42

``write_data()`` 复制从读取中获得的缓冲区。 这个缓冲区不会传递到写完成时触发的写回调。 至解决这个问题，我们在 write_req_t 中包装一个写请求和一个缓冲区，然后在回调中解开它。 我们制作一个副本，这样我们就可以释放两个缓冲区对 ``write_data`` 的两次调用是相互独立的。 虽然可以接受对于这样的演示程序，您可能需要更智能的内存管理，像引用计数缓冲区或任何主要应用程序中的缓冲区池。

.. WARNING::

    如果您的程序打算与其他程序一起使用，它可能会故意或在不知不觉中正在写入管道。 这使得它容易被 `aborting on 接收 SIGPIPE`_。 插入是个好主意::

        signal(SIGPIPE, SIG_IGN)

    在应用程序的初始化阶段.

.. _aborting on receiving a SIGPIPE: http://pod.tst.eu/http://cvs.schmorp.de/libev/ev.pod#The_special_problem_of_SIGPIPE

文件更改事件
------------------

所有现代操作系统都提供 API 来监视单个文件或目录并在文件被修改时得到通知。 libuv 包装常见文件更改通知库 [#fsnotify]_。 这是其中之一 libuv 的不一致部分。 文件更改通知系统本身跨平台变化很大，所以让一切在任何地方都能正常工作难的。 为了演示，我将构建一个运行的简单实用程序任何被监视文件更改时的命令::

    ./onchange <command> <file1> [file2] ...

.. note::

    目前这个例子只适用于 OSX 和 Windows。参考 `uv_fs_event_start`_ 函数的注释.

.. _notes of uv_fs_event_start: https://docs.libuv.org/en/v1.x/fs_event.html#c.uv_fs_event_start

文件更改通知使用启动 ``uv_fs_event_init()``:

.. rubric:: onchange/main.c - The setup
.. literalinclude:: ../../code/onchange/main.c
    :language: c
    :linenos:
    :lines: 26-
    :emphasize-lines: 15

第三个参数是要监控的实际文件或目录。 最后参数 ``标志`` 可以是:

.. code-block:: c

    /*
     * Flags to be passed to uv_fs_event_start().
     */
    enum uv_fs_event_flags {
        UV_FS_EVENT_WATCH_ENTRY = 1,
        UV_FS_EVENT_STAT = 2,
        UV_FS_EVENT_RECURSIVE = 4
    };

``UV_FS_EVENT_WATCH_ENTRY`` 和 ``UV_FS_EVENT_STAT`` 不做任何事情（还）。 ``UV_FS_EVENT_RECURSIVE`` 也将开始监视子目录支持的平台。

回调将接收以下参数：

  #. ``uv_fs_event_t *handle`` - The handle. The ``path`` field of the handle
     is the file on which the watch was set.
  #. ``const char *filename`` - If a directory is being monitored, this is the
     file which was changed. Only non-``null`` on Linux and Windows. May be ``null``
     even on those platforms.
  #. ``int events`` - one of ``UV_RENAME`` or ``UV_CHANGE``, or a bitwise OR of
     both.
  #. ``int status`` - If ``status < 0``, there is an :ref:`libuv error<libuv-error-handling>`.

在我们的示例中，我们只需打印参数并使用以下命令运行命令
``system()``.

.. rubric:: onchange/main.c - file change notification callback
.. literalinclude:: ../../code/onchange/main.c
    :language: c
    :linenos:
    :lines: 9-24

----

.. [#fsnotify] inotify on Linux, FSEvents on Darwin, kqueue on BSDs,
               ReadDirectoryChangesW on Windows, event ports on Solaris, unsupported on Cygwin
.. [#] see :ref:`pipes`
