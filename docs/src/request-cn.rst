
.. _request:

:c:type:`uv_req_t` --- Base request
===================================

`uv_req_t` 是所有 libuv 请求类型的基本类型.

结构对齐，以便任何 libuv 请求都可以转换为 `uv_req_t`.
此处定义的所有 API 函数都适用于任何请求类型.


Data types
----------

.. c:type:: uv_req_t

    The base libuv request structure.

.. c:type:: uv_any_req

    Union of all request types.


Public members
^^^^^^^^^^^^^^

.. c:member:: void* uv_req_t.data

    用户定义的任意数据的空间。 libuv 不使用该字段.

.. c:member:: uv_req_type uv_req_t.type

    表示请求的类型。只读.

    ::

        typedef enum {
            UV_UNKNOWN_REQ = 0,
            UV_REQ,
            UV_CONNECT,
            UV_WRITE,
            UV_SHUTDOWN,
            UV_UDP_SEND,
            UV_FS,
            UV_WORK,
            UV_GETADDRINFO,
            UV_GETNAMEINFO,
            UV_REQ_TYPE_MAX,
        } uv_req_type;


API
---

.. c:macro:: UV_REQ_TYPE_MAP(iter_macro)

    扩展为每个请求类型的一系列 "iter_macro" 调用的宏。 `iter_macro` 使用两个参数调用：没有 `UV_` 前缀的 `uv_req_type` 元素的名称，以及没有 `uv_` 前缀和 `_t` 后缀的相应结构类型的名称.

.. c:function:: int uv_cancel(uv_req_t* req)

    取消挂起的请求。如果请求正在执行或已完成执行，则失败.

    成功返回 0, 失败返回错误代码 < 0.

    当前仅取消 :c:type:`uv_fs_t`、:c:type:`uv_getaddrinfo_t`、:c:type:`uv_getnameinfo_t`、:c:type:`uv_random_t` 和 :c:type:`uv_work_t` 请求支持的.

    取消的请求会在未来某个时间调用它们的回调.
    在调用回调之前释放与请求关联的内存是 **不** 安全的.

    这是向回调报告取消的方式:

    * A :c:type:`uv_fs_t` 请求的 req->result 字段设置为 `UV_ECANCELED`.

    * A :c:type:`uv_work_t`, :c:type:`uv_getaddrinfo_t`,
      :c:type:`uv_getnameinfo_t` or :c:type:`uv_random_t` 请求使用 status == `UV_ECANCELED` 调用其回调.

.. c:function:: size_t uv_req_size(uv_req_type type)

    返回给定请求类型的大小。对于不想知道结构布局的 FFI 绑定编写者很有用.

.. c:function:: void* uv_req_get_data(const uv_req_t* req)

    Returns `req->data`.

    .. versionadded:: 1.19.0

.. c:function:: void* uv_req_set_data(uv_req_t* req, void* data)

    Sets `req->data` to `data`.

    .. versionadded:: 1.19.0

.. c:function:: uv_req_type uv_req_get_type(const uv_req_t* req)

    Returns `req->type`.

    .. versionadded:: 1.19.0

.. c:function:: const char* uv_req_type_name(uv_req_type type)

    返回给定请求类型的等效结构的名称，例如 `UV_CONNECT` 的 `"connect"`（如 :c:type:`uv_connect_t`）.

    如果不存在这样的请求类型，则返回 "NULL"。

    .. versionadded:: 1.19.0
