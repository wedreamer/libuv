
.. _tty:

:c:type:`uv_tty_t` --- TTY handle
=================================

TTY 句柄代表控制台的流。

:c:type:`uv_tty_t` is a 'subclass' of :c:type:`uv_stream_t`.


Data types
----------

.. c:type:: uv_tty_t

    TTY handle type.

.. c:enum:: uv_tty_mode_t

    .. versionadded:: 1.2.0

    TTY mode type:

    ::

      typedef enum {
          /* Initial/normal terminal mode */
          UV_TTY_MODE_NORMAL,
          /* Raw input mode (On Windows, ENABLE_WINDOW_INPUT is also enabled) */
          UV_TTY_MODE_RAW,
          /* Binary-safe I/O mode for IPC (Unix-only) */
          UV_TTY_MODE_IO
      } uv_tty_mode_t;

.. c:enum:: uv_tty_vtermstate_t

    控制台虚拟终端模式类型:

    ::

      typedef enum {
          /*
           * The console supports handling of virtual terminal sequences
           * (Windows10 new console, ConEmu)
           */
          UV_TTY_SUPPORTED,
          /* The console cannot process virtual terminal sequences.  (Legacy
           * console)
           */
          UV_TTY_UNSUPPORTED
      } uv_tty_vtermstate_t



Public members
^^^^^^^^^^^^^^

N/A

.. seealso:: The :c:type:`uv_stream_t` members also apply.


API
---

.. c:function:: int uv_tty_init(uv_loop_t* loop, uv_tty_t* handle, uv_file fd, int unused)

    使用给定的文件描述符初始化一个新的 TTY 流。通常文件描述符是:

    * 0 = stdin
    * 1 = stdout
    * 2 = stderr

    在 Unix 上，此函数将使用 ttyname_r(3) 确定终端 fd 的路径，打开它，如果传递的文件描述符指向 TTY, 则使用它。这让 libuv 将 tty 置于非阻塞模式，而不会影响共享 tty 的其他进程.

    此函数在不支持 ioctl TIOCGPTN 或 TIOCPTYGNAME 的系统上不是线程安全的，例如 OpenBSD 和 Solaris.

    .. note::
        如果重新打开 TTY 失败, libuv 会退回到阻止写入.

    .. versionchanged:: 1.23.1: the `readable` parameter is now unused and ignored.
                        The correct value will now be auto-detected from the kernel.

    .. versionchanged:: 1.9.0: the path of the TTY is determined by
                        :man:`ttyname_r(3)`. In earlier versions libuv opened
                        `/dev/tty` instead.

    .. versionchanged:: 1.5.0: trying to initialize a TTY stream with a file
                        descriptor that refers to a file returns `UV_EINVAL`
                        on UNIX.

.. c:function:: int uv_tty_set_mode(uv_tty_t* handle, uv_tty_mode_t mode)

    .. versionchanged:: 1.2.0: the mode is specified as a
                        :c:type:`uv_tty_mode_t` value.

    使用指定的终端模式设置 TTY.

.. c:function:: int uv_tty_reset_mode(void)

    程序退出时调用。将 TTY 设置重置为默认值，以便下一个进程接管.

    这个函数在 Unix 平台上是异步信号安全的，但是如果你在 :c:func:`uv_tty_set_mode` 中调用它，它可能会失败并出现错误代码 `UV_EBUSY`.

.. c:function:: int uv_tty_get_winsize(uv_tty_t* handle, int* width, int* height)

    Gets the current Window size. On success it returns 0.

.. seealso:: The :c:type:`uv_stream_t` API functions also apply.

.. c:function:: void uv_tty_set_vterm_state(uv_tty_vtermstate_t state)

    控制控制台虚拟终端序列是由 libuv 还是控制台处理.
    特别适用于启用对 ANSI X3.64 和 Xterm 256 颜色的 ConEmu 支持。否则通常会自动检测到 Windows10 控制台.

    此功能仅在 Windows 系统上有意义。在 Unix 上它被默默地忽略.

    .. versionadded:: 1.33.0

.. c:function:: int uv_tty_get_vterm_state(uv_tty_vtermstate_t* state)

    获取控制台虚拟终端序列是由 libuv 还是控制台处理的当前状态.

    这个函数在 Unix 上没有实现，它返回 ``UV_ENOTSUP``.

    .. versionadded:: 1.33.0

