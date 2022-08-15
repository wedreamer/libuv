
.. _loop:

:c:type:`uv_loop_t` --- Event loop
==================================

事件循环是 libuv 功能的核心部分。 它负责轮询 i/o 和调度基于不同事件源运行的回调.


Data types
----------

.. c:type:: uv_loop_t

    Loop data type.

.. c:enum:: uv_run_mode

    Mode used to run the loop with :c:func:`uv_run`.

    ::

        typedef enum {
            UV_RUN_DEFAULT = 0,
            UV_RUN_ONCE,
            UV_RUN_NOWAIT
        } uv_run_mode;

.. c:type:: void (*uv_walk_cb)(uv_handle_t* handle, void* arg)

    Type definition for callback passed to :c:func:`uv_walk`.


Public members
^^^^^^^^^^^^^^

.. c:member:: void* uv_loop_t.data

    用户定义的任意数据的空间。 libuv 不使用也不接触这个字段。


API
---

.. c:function:: int uv_loop_init(uv_loop_t* loop)

    Initializes the given `uv_loop_t` structure.

.. c:function:: int uv_loop_configure(uv_loop_t* loop, uv_loop_option option, ...)

    .. versionadded:: 1.0.2

    设置其他循环选项。 除非另有说明，否则您通常应该在第一次调用 :c:func:`uv_run` 之前调用它。

    成功时返回 0，失败时返回 UV_E* 错误代码。 准备好处理 UV_ENOSYS； 这意味着平台不支持循环选项。

    Supported options:

    - UV_LOOP_BLOCK_SIGNAL: 在轮询新事件时阻止信号。 uv_loop_configure 的第二个参数是信号编号.

      此操作当前仅针对 SIGPROF 信号实现，以在使用采样分析器时抑制不必要的唤醒。 请求其他信号将失败并显示 UV_EINVAL.

    - UV_METRICS_IDLE_TIME: 累积事件循环在事件提供程序中花费的空闲时间量.

      此选项是使用 :c:func:`uv_metrics_idle_time` 所必需的.

    .. versionchanged:: 1.39.0 added the UV_METRICS_IDLE_TIME option.

.. c:function:: int uv_loop_close(uv_loop_t* loop)

    释放所有内部循环资源。 仅当循环执行完毕且所有打开的句柄和请求都已关闭时调用此函数，否则将返回 UV_EBUSY。 此函数返回后，用户可以释放为循环分配的内存.

.. c:function:: uv_loop_t* uv_default_loop(void)

    返回初始化的默认循环。 如果分配失败，它可能会返回 NULL.

    这个函数只是在整个应用程序中使用全局循环的一种便捷方式，默认循环与使用 :c:func:`uv_loop_init` 初始化的循环没有任何不同。 因此，默认循环可以（并且应该）用 :c:func:`uv_loop_close` 关闭，因此与其关联的资源被释放.

    .. warning::
        这个函数不是线程安全的.

.. c:function:: int uv_run(uv_loop_t* loop, uv_run_mode mode)

    此函数运行事件循环。 它会根据指定的模式采取不同的行动:

    - UV_RUN_DEFAULT: 运行事件循环，直到没有更多活动和引用的句柄或请求。 如果 :c:func:`uv_stop` 被调用并且仍然有活动的句柄或请求，则返回非零值。 在所有其他情况下返回零.
    - UV_RUN_ONCE: 轮询 i/o 一次。 请注意，如果没有挂起的回调，此函数将阻塞。 完成时返回零（没有活动的句柄或请求），如果预计会有更多回调，则返回非零（意味着您应该在将来的某个时间再次运行事件循环）.
    - UV_RUN_NOWAIT: 轮询 i/o 一次，但如果没有挂起的回调，则不要阻塞。 如果完成则返回零（没有活动的句柄或请求），或者如果预期有更多回调则返回非零（意味着您应该在将来的某个时间再次运行事件循环）.

    :c:func:`uv_run` 不可重入。 不能从回调中调用它.

.. c:function:: int uv_loop_alive(const uv_loop_t* loop)

    如果循环中有引用的活动句柄、活动请求或关闭句柄，则返回非零.

.. c:function:: void uv_stop(uv_loop_t* loop)

    停止事件循环，使 :c:func:`uv_run` 尽快结束。 这将在下一次循环迭代之前发生。如果在阻塞 i/o 之前调用此函数，则循环不会在此迭代中阻塞 i/o.

.. c:function:: size_t uv_loop_size(void)

    返回 `uv_loop_t` 结构的大小。 对于不想知道结构布局的 FFI 绑定编写者很有用.

.. c:function:: int uv_backend_fd(const uv_loop_t* loop)

    获取后端文件描述符。 仅支持 kqueue、epoll 和事件端口.

    这可以与 `uv_run(loop, UV_RUN_NOWAIT)` 结合使用，在一个线程中轮询并在另一个线程中运行事件循环的回调，请参阅 test/test-embed.c 示例.

    .. note::
        将 kqueue fd 嵌入另一个 kqueue pollset 并不适用于所有平台。 添加 fd 不是错误，但它从不生成事件.

.. c:function:: int uv_backend_timeout(const uv_loop_t* loop)

    获取轮询超时。 返回值以毫秒为单位，或 -1 表示没有超时.

.. c:function:: uint64_t uv_now(const uv_loop_t* loop)

    以毫秒为单位返回当前时间戳。 时间戳在事件循环开始时被缓存，详情和理由参见:c:func:`uv_update_time`.

    时间戳从某个任意时间点单调增加。 不要对起点做假设，你只会失望.

    .. note::
        Use :c:func:`uv_hrtime` if you need sub-millisecond granularity.

.. c:function:: void uv_update_time(uv_loop_t* loop)

    更新事件循环的 "now" 概念。 Libuv 在事件循环开始时缓存当前时间，以减少与时间相关的系统调用次数.

    你通常不需要调用这个函数，除非你有回调会阻塞事件循环更长的时间，其中“更长”有点主观，但可能在毫秒或更长的数量级。

.. c:function:: void uv_walk(uv_loop_t* loop, uv_walk_cb walk_cb, void* arg)

    遍历句柄列表：`walk_cb` 将使用给定的 `arg` 执行。

.. c:function:: int uv_loop_fork(uv_loop_t* loop)

    .. versionadded:: 1.12.0

    在 :man:`fork(2)` 系统调用之后重新初始化子进程中所需的任何内核状态。

    先前启动的观察者将继续在子进程中启动。

    有必要在您计划在子进程中继续使用的父进程中创建的每个事件循环上显式调用此函数，包括默认循环（即使您不继续在父进程中使用它）。 该函数必须在调用 :c:func:`uv_run` 或任何其他使用子循环的 API 函数之前调用。 不这样做将导致未定义的行为，可能包括传递给父子进程的重复事件或中止子进程.

    如果可能，最好在子进程中创建一个新循环，而不是重用在父进程中创建的循环。 fork 后子进程创建的新循环不应该使用这个函数.

    此函数未在 Windows 上实现，它返回 ``UV_ENOSYS``.

    .. caution::

       此功能是实验性的。 它可能包含错误，并且可能会更改或删除。 不保证 API 和 ABI 的稳定性.

    .. note::

        在 Mac OS X 上，如果父进程 *对于任何事件循环* 正在使用目录 FS 事件句柄，则子进程将不再能够使用最有效的 FSEvent 实现。 相反，在子进程中使用目录 FS 事件句柄将回退到用于文件和其他基于 kqueue 的系统的相同实现。

    .. caution::

       在 AIX 和 SunOS 上，在分叉时已经在父进程中启动的 FS 事件句柄 *不会* 在子进程中传递事件； 它们必须关闭并重新启动。 在所有其他平台上，它们将继续正常工作，无需任何进一步干预。

    .. caution::

       从 :c:func:`uv_backend_fd` 返回的任何先前值现在都无效。 必须再次调用该函数以确定正确的后端文件描述符.

.. c:function:: void* uv_loop_get_data(const uv_loop_t* loop)

    Returns `loop->data`.

    .. versionadded:: 1.19.0

.. c:function:: void* uv_loop_set_data(uv_loop_t* loop, void* data)

    Sets `loop->data` to `data`.

    .. versionadded:: 1.19.0
