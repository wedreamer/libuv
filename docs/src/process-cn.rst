
.. _process:

:c:type:`uv_process_t` --- Process handle
=========================================

流程处理将产生一个新的过程，并允许用户控制它并使用流来建立通信渠道.


Data types
----------

.. c:type:: uv_process_t

    Process handle type.

.. c:type:: uv_process_options_t

    Options for spawning the process (passed to :c:func:`uv_spawn`.

    ::

        typedef struct uv_process_options_s {
            uv_exit_cb exit_cb;
            const char* file;
            char** args;
            char** env;
            const char* cwd;
            unsigned int flags;
            int stdio_count;
            uv_stdio_container_t* stdio;
            uv_uid_t uid;
            uv_gid_t gid;
        } uv_process_options_t;

.. c:type:: void (*uv_exit_cb)(uv_process_t*, int64_t exit_status, int term_signal)

    输入回调的类型定义传递在：c：类型：`uv_process_options_t`中，将指示导致该过程终止的退出状态和信号, 如果有的话.

.. c:type:: uv_process_flags

    标志要在：c：type：`uv_process_options_t`的标志字段上设置.

    ::

        enum uv_process_flags {
            /*
            * Set the child process' user id.
            */
            UV_PROCESS_SETUID = (1 << 0),
            /*
            * Set the child process' group id.
            */
            UV_PROCESS_SETGID = (1 << 1),
            /*
            * Do not wrap any arguments in quotes, or perform any other escaping, when
            * converting the argument list into a command line string. This option is
            * only meaningful on Windows systems. On Unix it is silently ignored.
            */
            UV_PROCESS_WINDOWS_VERBATIM_ARGUMENTS = (1 << 2),
            /*
            * Spawn the child process in a detached state - this will make it a process
            * group leader, and will effectively enable the child to keep running after
            * the parent exits. Note that the child process will still keep the
            * parent's event loop alive unless the parent process calls uv_unref() on
            * the child's process handle.
            */
            UV_PROCESS_DETACHED = (1 << 3),
            /*
            * Hide the subprocess window that would normally be created. This option is
            * only meaningful on Windows systems. On Unix it is silently ignored.
            */
            UV_PROCESS_WINDOWS_HIDE = (1 << 4),
            /*
            * Hide the subprocess console window that would normally be created. This 
            * option is only meaningful on Windows systems. On Unix it is silently
            * ignored.
            */
            UV_PROCESS_WINDOWS_HIDE_CONSOLE = (1 << 5),
            /*
            * Hide the subprocess GUI window that would normally be created. This 
            * option is only meaningful on Windows systems. On Unix it is silently
            * ignored.
            */
            UV_PROCESS_WINDOWS_HIDE_GUI = (1 << 6)
        };

.. c:type:: uv_stdio_container_t

    传递给子进程的每个 stdio 句柄或 fd 的容器.

    ::

        typedef struct uv_stdio_container_s {
            uv_stdio_flags flags;
            union {
                uv_stream_t* stream;
                int fd;
            } data;
        } uv_stdio_container_t;

.. c:enum:: uv_stdio_flags

    指定如何将 stdio 传输到子进程的标志.

    ::

        typedef enum {
            /*
            * The following four options are mutually-exclusive, and define
            * the operation to perform for the corresponding file descriptor
            * in the child process:
            */

            /*
            * No file descriptor will be provided (or redirected to
            * `/dev/null` if it is fd 0, 1 or 2).
            */
            UV_IGNORE = 0x00,

            /*
            * Open a new pipe into `data.stream`, per the flags below. The
            * `data.stream` field must point to a uv_pipe_t object that has
            * been initialized with `uv_pipe_init(loop, data.stream, ipc);`,
            * but not yet opened or connected.
            /*
            UV_CREATE_PIPE = 0x01,

            /*
            * The child process will be given a duplicate of the parent's
            * file descriptor given by `data.fd`.
            */
            UV_INHERIT_FD = 0x02,

            /*
            * The child process will be given a duplicate of the parent's
            * file descriptor being used by the stream handle given by
            * `data.stream`.
            */
            UV_INHERIT_STREAM = 0x04,

            /*
            * When UV_CREATE_PIPE is specified, UV_READABLE_PIPE and UV_WRITABLE_PIPE
            * determine the direction of flow, from the child process' perspective. Both
            * flags may be specified to create a duplex data stream.
            */
            UV_READABLE_PIPE = 0x10,
            UV_WRITABLE_PIPE = 0x20,

            /*
            * When UV_CREATE_PIPE is specified, specifying UV_NONBLOCK_PIPE opens the
            * handle in non-blocking mode in the child. This may cause loss of data,
            * if the child is not designed to handle to encounter this mode,
            * but can also be significantly more efficient.
            */
            UV_NONBLOCK_PIPE = 0x40
        } uv_stdio_flags;


Public members
^^^^^^^^^^^^^^

.. c:member:: int uv_process_t.pid

    生成的进程的 PID。它是在调用 :c:func:`uv_spawn` 后设置的.

.. note::
    :c:type:`uv_handle_t` 成员也适用.

.. c:member:: uv_exit_cb uv_process_options_t.exit_cb

    进程退出后调用的回调

.. c:member:: const char* uv_process_options_t.file

    指向要执行的程序的路径.

.. c:member:: char** uv_process_options_t.args

    命令行参数。 args[0] 应该是程序的路径。在 Windows 上，它使用 `CreateProcess` 将参数连接成一个字符串，这可能会导致一些奇怪的错误。请参阅 :c:type:`uv_process_flags 上的 `UV_PROCESS_WINDOWS_VERBATIM_ARGUMENTS` 标志.

.. c:member:: char** uv_process_options_t.env

    新流程的环境。如果为 NULL，则使用了父进程环境.

.. c:member:: const char* uv_process_options_t.cwd

    子进程的当前工作目录.

.. c:member:: unsigned int uv_process_options_t.flags

    控制 :c:func:`uv_spawn` 行为方式的各种标志。见:c:type:`uv_process_flags`.

.. c:member:: int uv_process_options_t.stdio_count
.. c:member:: uv_stdio_container_t* uv_process_options_t.stdio

    `stdio` 字段指向 :c:type:`uv_stdio_container_t` 结构的数组，这些结构描述了子进程可用的文件描述符。约定是stdio[0]指向stdin，fd 1 用于stdout，fd 2 是stderr。

    .. note::
        在 Windows 上，只有当子进程使用 MSVCRT 运行时，大于 2 的文件描述符才可用于子进程.

.. c:member:: uv_uid_t uv_process_options_t.uid
.. c:member:: uv_gid_t uv_process_options_t.gid

    Libuv 可以更改子进程的用户/组 ID。仅当在标志字段中设置了适当的位时才会发生这种情况.

    .. note::
        这在 Windows 上不受支持，:c:func:`uv_spawn` 将失败并将错误设置为 `UV_ENOTSUP`.

.. c:member:: uv_stdio_flags uv_stdio_container_t.flags

    指定如何将 stdio 容器传递给子级的标志.

.. c:member:: union @0 uv_stdio_container_t.data

    包含要传递给子进程的 "stream" 或 "fd" 的联合.


API
---

.. c:function:: void uv_disable_stdio_inheritance(void)

    禁用此进程从其父进程继承的文件描述符/句柄的继承。效果是这个进程产生的子进程不会意外继承这些句柄.

    建议在程序中尽早调用此函数，在继承的文件描述符可以关闭或复制之前.

    .. note::
        这个函数在尽力而为的基础上工作：不能保证 libuv 可以发现所有被继承的文件描述符。一般来说，它在 Windows 上比在 Unix 上做得更好.

.. c:function:: int uv_spawn(uv_loop_t* loop, uv_process_t* handle, const uv_process_options_t* options)

    初始化进程句柄并启动进程。如果进程成功生成，此函数将返回 0。否则，返回与它无法生成的原因相对应的负错误代码.

    未能生成的可能原因包括（但不限于）要执行的文件不存在，没有使用指定的 setuid 或 setgid 的权限，或者没有足够的内存来为新进程分配.

    .. versionchanged:: 1.24.0 Added `UV_PROCESS_WINDOWS_HIDE_CONSOLE` and
                        `UV_PROCESS_WINDOWS_HIDE_GUI` flags.

.. c:function:: int uv_process_kill(uv_process_t* handle, int signum)

    将指定的信号发送到给定的进程句柄。检查 :c:ref:`signal` 上的文档以获取信号支持，特别是在 Windows 上.

.. c:function:: int uv_kill(int pid, int signum)

    将指定的信号发送到给定的 PID。检查 :c:ref:`signal` 上的文档以获取信号支持，特别是在 Windows 上.

.. c:function:: uv_pid_t uv_process_get_pid(const uv_process_t* handle)

    Returns `handle->pid`.

    .. versionadded:: 1.19.0

.. seealso:: The :c:type:`uv_handle_t` API functions also apply.
