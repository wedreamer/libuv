
.. _version:

Version-checking macros and functions
=====================================

从 1.0.0 版开始，libuv 遵循 `semantic versioning`_ 方案。这意味着可以在主要版本的整个生命周期中引入新的 API。在本节中，您将找到所有允许您有条件地编写或编译代码的宏和函数，以便使用多个 libuv 版本.

.. _semantic versioning: https://semver.org


Macros
------

.. c:macro:: UV_VERSION_MAJOR

    libuv 版本的主编号.

.. c:macro:: UV_VERSION_MINOR

    libuv 版本的次要编号.

.. c:macro:: UV_VERSION_PATCH

    libuv 版本的补丁号.

.. c:macro:: UV_VERSION_IS_RELEASE

    设置为 1 表示 libuv 的发布版本，0 表示开发快照.

.. c:macro:: UV_VERSION_SUFFIX

    libuv 版本后缀。某些开发版本（例如 Release Candidates）可能具有诸如“rc”之类的后缀.

.. c:macro:: UV_VERSION_HEX

    返回打包成单个整数的 libuv 版本。每个组件使用 8 位，补丁编号存储在 8 个最低有效位中。例如。对于 libuv 1.2.3，这将是 0x010203.

    .. versionadded:: 1.7.0


Functions
---------

.. c:function:: unsigned int uv_version(void)

    Returns :c:macro:`UV_VERSION_HEX`.

.. c:function:: const char* uv_version_string(void)

    以字符串形式返回 libuv 版本号。对于非发布版本，包含版本后缀.
