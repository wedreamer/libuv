
.. _stream:

:c:type:`uv_stream_t` --- Stream handle
=======================================

流句柄提供双工通信通道的抽象.
:c:type:`uv_stream_t` 是一个抽象类型，libuv 提供了 :c:type:`uv_tcp_t`、:c:type:`uv_pipe_t` 和 :c:type:`uv_tty_t` 三种流实现.


Data types
----------

.. c:type:: uv_stream_t

    Stream handle type.

.. c:type:: uv_connect_t

    Connect request type.

.. c:type:: uv_shutdown_t

    Shutdown request type.

.. c:type:: uv_write_t

    写请求类型。重用此类对象时必须特别注意。当流处于非阻塞模式时，使用 uv_write 发送的写入请求将被排队。此时重用对象是未定义的行为。只有在传递给 `uv_write` 的回调被触发后才能安全地重用 `uv_write_t` 对象.

.. c:type:: void (*uv_read_cb)(uv_stream_t* stream, ssize_t nread, const uv_buf_t* buf)

    在流上读取数据时调用的回调.

    如果有可用数据，则 `nread` > 0 或错误时 < 0。当我们到达 EOF 时， `nread` 将被设置为 `UV_EOF`。当 `nread` < 0 时， `buf` 参数可能没有指向有效的缓冲区；在这种情况下， `buf.len` 和 `buf.base` 都设置为 0。

    .. note::
        `nread` 可能为 0，它 *不* 表示错误或 EOF。这相当于 ``read(2)`` 下的 ``EAGAIN`` 或 ``EWOULDBLOCK``.

    当错误发生时，被调用者负责通过调用 :c:func:`uv_read_stop` 或 :c:func:`uv_close` 来停止/关闭流。尝试再次从流中读取是未定义的.

    被调用者负责释放缓冲区，libuv 不重用它.
    错误时缓冲区可能是空缓冲区（其中 `buf->base` == NULL 和 `buf->len` == 0）.

.. c:type:: void (*uv_write_cb)(uv_write_t* req, int status)

    数据写入流后调用的回调。 `status` 成功时为 0，否则为 < 0.

.. c:type:: void (*uv_connect_cb)(uv_connect_t* req, int status)

    由 :c:func:`uv_connect` 启动的连接完成后调用的回调。 `status` 成功时为 0，否则为 < 0.

.. c:type:: void (*uv_shutdown_cb)(uv_shutdown_t* req, int status)

    关闭请求完成后调用的回调。 `status` 成功时为 0，否则为 < 0.

.. c:type:: void (*uv_connection_cb)(uv_stream_t* server, int status)

    流服务器收到传入连接时调用的回调.
    用户可以通过调用 :c:func:`uv_accept` 来接受连接.
    `status` 成功时为 0，否则为 < 0.


Public members
^^^^^^^^^^^^^^

.. c:member:: size_t uv_stream_t.write_queue_size

    包含等待发送的排队字节数。只读.

.. c:member:: uv_stream_t* uv_connect_t.handle

    指向运行此连接请求的流的指针.

.. c:member:: uv_stream_t* uv_shutdown_t.handle

    指向正在运行此关闭请求的流的指针.

.. c:member:: uv_stream_t* uv_write_t.handle

    指向运行此写入请求的流的指针.

.. c:member:: uv_stream_t* uv_write_t.send_handle

    指向使用此写入请求发送的流的指针.

.. seealso:: The :c:type:`uv_handle_t` members also apply.


API
---

.. c:function:: int uv_shutdown(uv_shutdown_t* req, uv_stream_t* handle, uv_shutdown_cb cb)

    关闭双工流的传出（写入）端。它等待挂起的写请求完成。 `handle` 应该引用一个初始化的流。 `req` 应该是一个未初始化的关闭请求结构。关闭完成后调用 `cb`.

.. c:function:: int uv_listen(uv_stream_t* stream, int backlog, uv_connection_cb cb)

    开始监听传入连接。 `backlog` 表示内核可能排队的连接数，与:man:`listen(2)` 相同。当收到新的传入连接时，将调用 :c:type:`uv_connection_cb` 回调.

.. c:function:: int uv_accept(uv_stream_t* server, uv_stream_t* client)

    此调用与 :c:func:`uv_listen` 结合使用以接受传入连接。收到一个:c:type:`uv_connection_cb`后调用这个函数来接受连接。在调用此函数之前，必须初始化客户端句柄。 < 0 返回值表示错误.

    当 :c:type:`uv_connection_cb` 回调被调用时，保证此函数将在第一次成功完成。如果您尝试多次使用它，它可能会失败。建议每次 :c:type:`uv_connection_cb` 调用只调用一次该函数.

    .. note::
        `server` 和 `client` 必须是在同一循环上运行的句柄.

.. c:function:: int uv_read_start(uv_stream_t* stream, uv_alloc_cb alloc_cb, uv_read_cb read_cb)

    从传入流中读取数据。 :c:type:`uv_read_cb` 回调将被多次执行，直到没有更多数据要读取或 :c:func:`uv_read_stop` 被调用.

    .. versionchanged:: 1.38.0 :c:func:`uv_read_start()` 现在在调用两次时始终返回“UV_EALREADY”，在流关闭时返回“UV_EINVAL”。对于较旧的 libuv 版本，它在 Windows 上返回 `UV_EALREADY` 而不是 UNIX，在 UNIX 上返回 `UV_EINVAL` 而不是 Windows.

.. c:function:: int uv_read_stop(uv_stream_t*)

    停止从流中读取数据。 :c:type:`uv_read_cb` 回调将不再被调用.

    此函数是幂等的，可以在停止的流上安全地调用.

    此功能将始终成功；因此，检查它的返回值是不必要的。非零返回表示完成释放资源可能在 Windows 上该 TTY 上的下一个输入事件上挂起，并不表示失败.

.. c:function:: int uv_write(uv_write_t* req, uv_stream_t* handle, const uv_buf_t bufs[], unsigned int nbufs, uv_write_cb cb)

    将数据写入流。缓冲区是按顺序写入的。例子:

    ::

        void cb(uv_write_t* req, int status) {
            /* Logic which handles the write result */
        }

        uv_buf_t a[] = {
            { .base = "1", .len = 1 },
            { .base = "2", .len = 1 }
        };

        uv_buf_t b[] = {
            { .base = "3", .len = 1 },
            { .base = "4", .len = 1 }
        };

        uv_write_t req1;
        uv_write_t req2;

        /* writes "1234" */
        uv_write(&req1, stream, a, 2, cb);
        uv_write(&req2, stream, b, 2, cb);

    .. note::
        缓冲区指向的内存必须保持有效，直到回调被调用.
        这也适用于:c:func:`uv_write2`.

.. c:function:: int uv_write2(uv_write_t* req, uv_stream_t* handle, const uv_buf_t bufs[], unsigned int nbufs, uv_stream_t* send_handle, uv_write_cb cb)

    用于通过管道发送句柄的扩展写入函数。管道必须用 `ipc` == 1 初始化.

    .. note::
        `send_handle` 必须是 Unix 上的 TCP、管道和 UDP 句柄，或 Windows 上的 TCP 句柄，它是服务器或连接（侦听或连接状态）。绑定的套接字或管道将被假定为服务器.

.. c:function:: int uv_try_write(uv_stream_t* handle, const uv_buf_t bufs[], unsigned int nbufs)

    与 :c:func:`uv_write` 相同，但如果不能立即完成写入请求，则不会排队.

    将返回:

    * > 0: 写入的字节数（可以小于提供的缓冲区大小）.
    * < 0: 负错误代码（如果不能立即发送数据，则返回 ``UV_EAGAIN``）.

.. c:function:: int uv_try_write2(uv_stream_t* handle, const uv_buf_t bufs[], unsigned int nbufs, uv_stream_t* send_handle)

    与 :c:func:`uv_try_write` 和扩展写入函数相同，用于通过管道发送句柄，如 c:func:`uv_write2`.

    Windows 不支持尝试发送句柄，它会返回“UV_EAGAIN”.

    .. versionadded:: 1.42.0
    
.. c:function:: int uv_is_readable(const uv_stream_t* handle)

    如果流可读，则返回 1，否则返回 0.

.. c:function:: int uv_is_writable(const uv_stream_t* handle)

    如果流是可写的，则返回 1，否则返回 0.

.. c:function:: int uv_stream_set_blocking(uv_stream_t* handle, int blocking)

    启用或禁用流的阻塞模式.

    启用阻塞模式后，所有写入同步完成。否则界面保持不变，例如操作的完成或失败仍将通过异步进行的回调报告.

    .. warning::
        不建议过度依赖此 API。未来可能会发生重大变化.

        目前仅适用于 Windows 的 :c:type:`uv_pipe_t` 句柄.
        在 UNIX 平台上，支持所有 :c:type:`uv_stream_t` 句柄.

        此外，当写入请求已提交后更改阻塞模式时，libuv 目前不提供排序保证。因此建议在打开或创建流后立即设置阻塞模式.

    .. versionchanged:: 1.4.0 UNIX implementation added.

.. c:function:: size_t uv_stream_get_write_queue_size(const uv_stream_t* stream)

    Returns `stream->write_queue_size`.

    .. versionadded:: 1.19.0

.. seealso:: The :c:type:`uv_handle_t` API functions also apply.
