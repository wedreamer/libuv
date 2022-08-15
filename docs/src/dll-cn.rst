
.. _dll:

共享库处理
=======================

libuv 提供跨平台实用程序，用于加载共享库并从中检索符号，使用以下 API.


Data types
----------

.. c:type:: uv_lib_t

    Shared library data type.


Public members
^^^^^^^^^^^^^^

N/A


API
---

.. c:function:: int uv_dlopen(const char* filename, uv_lib_t* lib)

    打开共享库。 文件名是 utf-8。 成功返回 0，错误返回 -1。 调用 :c:func:`uv_dlerror` 获取错误信息.

.. c:function:: void uv_dlclose(uv_lib_t* lib)

    关闭共享库.

.. c:function:: int uv_dlsym(uv_lib_t* lib, const char* name, void** ptr)

    从动态库中检索数据指针。 符号映射到 NULL 是合法的。 成功返回 0，如果未找到符号则返回 -1.

.. c:function:: const char* uv_dlerror(const uv_lib_t* lib)

    返回最后一个 uv_dlopen() 或 uv_dlsym() 错误消息.
