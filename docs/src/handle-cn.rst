
.. _handle:

:c:type:`uv_handle_t` --- Base handle
=====================================

`uv_handle_t` is the base type for all libuv handle types.

结构是对齐的，因此任何 libuv 句柄都可以转换为 `uv_handle_t`。 此处定义的所有 API 函数都适用于任何句柄类型.

Libuv 手柄不可移动。 传递给函数的处理结构的指针必须在请求的操作期间保持有效。 使用堆栈分配的句柄时要小心.

Data types
----------

.. c:type:: uv_handle_t

    The base libuv handle type.

.. c:enum:: uv_handle_type

    The kind of the libuv handle.

    ::

        typedef enum {
          UV_UNKNOWN_HANDLE = 0,
          UV_ASYNC,
          UV_CHECK,
          UV_FS_EVENT,
          UV_FS_POLL,
          UV_HANDLE,
          UV_IDLE,
          UV_NAMED_PIPE,
          UV_POLL,
          UV_PREPARE,
          UV_PROCESS,
          UV_STREAM,
          UV_TCP,
          UV_TIMER,
          UV_TTY,
          UV_UDP,
          UV_SIGNAL,
          UV_FILE,
          UV_HANDLE_TYPE_MAX
        } uv_handle_type;

.. c:type:: uv_any_handle

    Union of all handle types.

.. c:type:: void (*uv_alloc_cb)(uv_handle_t* handle, size_t suggested_size, uv_buf_t* buf)

    传递给 :c:func:`uv_read_start` 和 :c:func:`uv_udp_recv_start` 的回调类型定义。 用户必须分配内存并填充提供的 :c:type:`uv_buf_t` 结构。 如果 NULL 被指定为缓冲区的基数或 0 作为其长度，则会在 :c:type:`uv_udp_recv_cb` 或 :c:type:`uv_read_cb` 回调中触发 `UV_ENOBUFS` 错误.

    每个缓冲区只使用一次，用户负责在 :c:type:`uv_udp_recv_cb` 或 :c:type:`uv_read_cb` 回调中释放它.

    提供了建议的大小 (在大多数情况下目前为 65536), 但这只是一个指示，与要读取的未决数据有任何关系。 用户可以自由分配他们决定的内存量.

    例如，具有自定义分配方案（例如使用空闲列表、分配池或基于平板的分配器）的应用程序可能会决定使用与它们已有的内存块匹配的不同大小.

    Example:

    ::

        static void my_alloc_cb(uv_handle_t* handle, size_t suggested_size, uv_buf_t* buf) {
          buf->base = malloc(suggested_size);
          buf->len = suggested_size;
        }

.. c:type:: void (*uv_close_cb)(uv_handle_t* handle)

    Type definition for callback passed to :c:func:`uv_close`.


Public members
^^^^^^^^^^^^^^

.. c:member:: uv_loop_t* uv_handle_t.loop

    Pointer to the :c:type:`uv_loop_t` the handle is running on. Readonly.

.. c:member:: uv_handle_type uv_handle_t.type

    The :c:type:`uv_handle_type`, indicating the type of the underlying handle. Readonly.

.. c:member:: void* uv_handle_t.data

    Space for user-defined arbitrary data. libuv does not use this field.


API
---

.. c:macro:: UV_HANDLE_TYPE_MAP(iter_macro)

    扩展为每个句柄类型的一系列 "iter_macro" 调用的宏。 `iter_macro` 使用两个参数调用：没有 `UV_` 前缀的 `uv_handle_type` 元素的名称，以及没有 `uv_` 前缀和 `_t` 后缀的相应结构类型的名称.

.. c:function:: int uv_is_active(const uv_handle_t* handle)

    如果句柄处于活动状态，则返回非零，如果不活动，则返回零。 "活动" 的含义取决于句柄的类型:

    - uv_async_t 句柄始终处于活动状态并且不能被停用，除非使用 uv_close() 关闭它.

    - uv_pipe_t、uv_tcp_t、uv_udp_t 等句柄——基本上是处理 i/o 的任何句柄——在它执行涉及 i/o 的操作时处于活动状态，例如读取、写入、连接、接受新连接等.

    - uv_check_t、uv_idle_t、uv_timer_t 等句柄在通过调用 uv_check_start()、uv_idle_start() 等启动时处于活动状态.

    经验法则：如果 `uv_foo_t` 类型的句柄具有 `uv_foo_start()` 函数，则从调用该函数的那一刻起它就处于活动状态。 同样，`uv_foo_stop()` 再次停用句柄.

.. c:function:: int uv_is_closing(const uv_handle_t* handle)

    如果句柄正在关闭或关闭, 则返回非零, 否则返回零.

    .. note::
        该函数只能在句柄初始化和关闭回调到达之间使用.

.. c:function:: void uv_close(uv_handle_t* handle, uv_close_cb close_cb)

    请求句柄关闭。 `close_cb` 将在此调用后异步调用。 在释放内存之前，必须在每个句柄上调用它。 而且，内存只能在`close_cb`或返回后才能释放.

    包装文件描述符的句柄会立即关闭，但 `close_cb` 仍将推迟到事件循环的下一次迭代.
    它使您有机会释放与句柄相关的任何资源.

    正在进行的请求，如 uv_connect_t 或 uv_write_t, 将被取消, 并以 status=UV_ECANCELED 异步调用它们的回调。

.. c:function:: void uv_ref(uv_handle_t* handle)

    引用给定的句柄。 引用是幂等的，也就是说，如果已经引用了句柄，再次调用此函数将无效。

    See :ref:`refcount`.

.. c:function:: void uv_unref(uv_handle_t* handle)

    取消引用给定的句柄。 引用是幂等的，也就是说，如果一个句柄没有被引用，再次调用这个函数将没有任何效果。

    See :ref:`refcount`.

.. c:function:: int uv_has_ref(const uv_handle_t* handle)

    如果引用了句柄，则返回非零，否则返回零。

    See :ref:`refcount`.

.. c:function:: size_t uv_handle_size(uv_handle_type type)

    返回给定句柄类型的大小。 对于不想知道结构布局的 FFI 绑定编写者很有用.


Miscellaneous API functions
---------------------------

以下 API 函数采用 :c:type:`uv_handle_t` 参数，但它们仅适用于某些句柄类型.

.. c:function:: int uv_send_buffer_size(uv_handle_t* handle, int* value)

    获取或设置操作系统用于套接字的发送缓冲区的大小.

    如果 `*value` == 0, 那么它将设置 `*value` 为当前发送缓冲区大小。
    如果 `*value` > 0 那么它将使用 `*value` 设置新的发送缓冲区大小.

    成功时，返回零。 出错时，返回否定结果.

    此函数适用于 Unix 上的 TCP、管道和 UDP 句柄以及 Windows 上的 TCP 和 UDP 句柄.

    .. note::
        Linux 将设置双倍大小并返回原始设置值的两倍大小.

.. c:function:: int uv_recv_buffer_size(uv_handle_t* handle, int* value)

    获取或设置操作系统用于套接字的接收缓冲区的大小.

    如果 `*value` == 0, 那么它将设置 `*value` 为当前接收缓冲区大小.
    如果 `*value` > 0 那么它将使用 `*value` 设置新的接收缓冲区大小.

    成功时，返回零。 出错时，返回否定结果。

    此函数适用于 Unix 上的 TCP、管道和 UDP 句柄以及 Windows 上的 TCP 和 UDP 句柄.

    .. note::
        Linux 将设置两倍大小并返回原始设置值的两倍大小.

.. c:function:: int uv_fileno(const uv_handle_t* handle, uv_os_fd_t* fd)

    等效于获取与平台相关的文件描述符.

    支持以下句柄: TCP、管道、TTY、UDP 和 poll。 传递任何其他句柄类型将失败并显示 `UV_EINVAL`.

    如果句柄还没有附加的文件描述符或句柄本身已关闭，此函数将返回 `UV_EBADF`.

    .. warning::
        使用此功能时要非常小心。 libuv 假定它在控制文件描述符，因此对其进行任何更改都可能导致故障.

.. c:function:: uv_loop_t* uv_handle_get_loop(const uv_handle_t* handle)

    Returns `handle->loop`.

    .. versionadded:: 1.19.0

.. c:function:: void* uv_handle_get_data(const uv_handle_t* handle)

    Returns `handle->data`.

    .. versionadded:: 1.19.0

.. c:function:: void* uv_handle_set_data(uv_handle_t* handle, void* data)

    Sets `handle->data` to `data`.

    .. versionadded:: 1.19.0

.. c:function:: uv_handle_type uv_handle_get_type(const uv_handle_t* handle)

    Returns `handle->type`.

    .. versionadded:: 1.19.0

.. c:function:: const char* uv_handle_type_name(uv_handle_type type)

    Returns the name for the equivalent struct for a given handle type,
    e.g. `"pipe"` (as in :c:type:`uv_pipe_t`) for `UV_NAMED_PIPE`.

    If no such handle type exists, this returns `NULL`.

    .. versionadded:: 1.19.0

.. _refcount:

Reference counting
------------------

libuv 事件循环（如果在默认模式下运行）将一直运行，直到没有活动的 `and` 引用句柄。 用户可以通过取消引用活动的句柄来强制循环提前退出, 例如在调用 :c:func:`uv_timer_start` 之后调用 :c:func:`uv_unref`.

句柄可以被引用或不被引用，引用计数方案不使用计数器，因此两个操作都是幂等的.

默认情况下，所有句柄都在活动时被引用，请参阅 :c:func:`uv_is_active` 以获得有关 `活动` 所涉及的更详细说明
