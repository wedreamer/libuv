
.. _errors:

错误处理
==============

在 libuv 中，错误是负数常量。 根据经验，只要有状态参数，或者 API 函数返回整数，负数就意味着错误。.

当接受回调的函数返回错误时，将永远不会调用回调.

.. note::
    实现细节：在 Unix 上，错误代码是否定的 `errno`（或 `-errno`），而在 Windows 上，它们由 libuv 定义为任意负数.


Error constants
---------------

.. c:macro:: UV_E2BIG

    argument list too long

.. c:macro:: UV_EACCES

    permission denied

.. c:macro:: UV_EADDRINUSE

    address already in use

.. c:macro:: UV_EADDRNOTAVAIL

    address not available

.. c:macro:: UV_EAFNOSUPPORT

    address family not supported

.. c:macro:: UV_EAGAIN

    resource temporarily unavailable

.. c:macro:: UV_EAI_ADDRFAMILY

    address family not supported

.. c:macro:: UV_EAI_AGAIN

    temporary failure

.. c:macro:: UV_EAI_BADFLAGS

    bad ai_flags value

.. c:macro:: UV_EAI_BADHINTS

    invalid value for hints

.. c:macro:: UV_EAI_CANCELED

    request canceled

.. c:macro:: UV_EAI_FAIL

    permanent failure

.. c:macro:: UV_EAI_FAMILY

    ai_family not supported

.. c:macro:: UV_EAI_MEMORY

    out of memory

.. c:macro:: UV_EAI_NODATA

    no address

.. c:macro:: UV_EAI_NONAME

    unknown node or service

.. c:macro:: UV_EAI_OVERFLOW

    argument buffer overflow

.. c:macro:: UV_EAI_PROTOCOL

    resolved protocol is unknown

.. c:macro:: UV_EAI_SERVICE

    service not available for socket type

.. c:macro:: UV_EAI_SOCKTYPE

    socket type not supported

.. c:macro:: UV_EALREADY

    connection already in progress

.. c:macro:: UV_EBADF

    bad file descriptor

.. c:macro:: UV_EBUSY

    resource busy or locked

.. c:macro:: UV_ECANCELED

    operation canceled

.. c:macro:: UV_ECHARSET

    invalid Unicode character

.. c:macro:: UV_ECONNABORTED

    software caused connection abort

.. c:macro:: UV_ECONNREFUSED

    connection refused

.. c:macro:: UV_ECONNRESET

    connection reset by peer

.. c:macro:: UV_EDESTADDRREQ

    destination address required

.. c:macro:: UV_EEXIST

    file already exists

.. c:macro:: UV_EFAULT

    bad address in system call argument

.. c:macro:: UV_EFBIG

    file too large

.. c:macro:: UV_EHOSTUNREACH

    host is unreachable

.. c:macro:: UV_EINTR

    interrupted system call

.. c:macro:: UV_EINVAL

    invalid argument

.. c:macro:: UV_EIO

    i/o error

.. c:macro:: UV_EISCONN

    socket is already connected

.. c:macro:: UV_EISDIR

    illegal operation on a directory

.. c:macro:: UV_ELOOP

    too many symbolic links encountered

.. c:macro:: UV_EMFILE

    too many open files

.. c:macro:: UV_EMSGSIZE

    message too long

.. c:macro:: UV_ENAMETOOLONG

    name too long

.. c:macro:: UV_ENETDOWN

    network is down

.. c:macro:: UV_ENETUNREACH

    network is unreachable

.. c:macro:: UV_ENFILE

    file table overflow

.. c:macro:: UV_ENOBUFS

    no buffer space available

.. c:macro:: UV_ENODEV

    no such device

.. c:macro:: UV_ENOENT

    no such file or directory

.. c:macro:: UV_ENOMEM

    not enough memory

.. c:macro:: UV_ENONET

    machine is not on the network

.. c:macro:: UV_ENOPROTOOPT

    protocol not available

.. c:macro:: UV_ENOSPC

    no space left on device

.. c:macro:: UV_ENOSYS

    function not implemented

.. c:macro:: UV_ENOTCONN

    socket is not connected

.. c:macro:: UV_ENOTDIR

    not a directory

.. c:macro:: UV_ENOTEMPTY

    directory not empty

.. c:macro:: UV_ENOTSOCK

    socket operation on non-socket

.. c:macro:: UV_ENOTSUP

    operation not supported on socket

.. c:macro:: UV_EOVERFLOW

    value too large for defined data type

.. c:macro:: UV_EPERM

    operation not permitted

.. c:macro:: UV_EPIPE

    broken pipe

.. c:macro:: UV_EPROTO

    protocol error

.. c:macro:: UV_EPROTONOSUPPORT

    protocol not supported

.. c:macro:: UV_EPROTOTYPE

    protocol wrong type for socket

.. c:macro:: UV_ERANGE

    result too large

.. c:macro:: UV_EROFS

    read-only file system

.. c:macro:: UV_ESHUTDOWN

    cannot send after transport endpoint shutdown

.. c:macro:: UV_ESPIPE

    invalid seek

.. c:macro:: UV_ESRCH

    no such process

.. c:macro:: UV_ETIMEDOUT

    connection timed out

.. c:macro:: UV_ETXTBSY

    text file is busy

.. c:macro:: UV_EXDEV

    cross-device link not permitted

.. c:macro:: UV_UNKNOWN

    unknown error

.. c:macro:: UV_EOF

    end of file

.. c:macro:: UV_ENXIO

    no such device or address

.. c:macro:: UV_EMLINK

    too many links

.. c:macro:: UV_ENOTTY

    inappropriate ioctl for device

.. c:macro:: UV_EFTYPE

    inappropriate file type or format

.. c:macro:: UV_EILSEQ

    illegal byte sequence

.. c:macro:: UV_ESOCKTNOSUPPORT

    socket type not supported


API
---

.. c:macro:: UV_ERRNO_MAP(iter_macro)

    扩展为上述每个错误常量的一系列“iter_macro”调用的宏。 `iter_macro` 使用两个参数调用：不带 `UV_` 前缀的错误常量的名称，以及错误消息字符串文字.

.. c:function:: const char* uv_strerror(int err)

    返回给定错误代码的错误消息。 当您使用未知错误代码调用它时会泄漏几个字节的内存.

.. c:function:: char* uv_strerror_r(int err, char* buf, size_t buflen)

    返回给定错误代码的错误消息。 以零结尾的消息存储在用户提供的缓冲区 "buf" 中，最多为 "buflen" 字节.

    .. versionadded:: 1.22.0

.. c:function:: const char* uv_err_name(int err)

    返回给定错误代码的错误名称。 当您使用未知错误代码调用它时会泄漏几个字节的内存.

.. c:function:: char* uv_err_name_r(int err, char* buf, size_t buflen)

    返回给定错误代码的错误名称。 以零结尾的名称存储在用户提供的缓冲区 "buf" 中，最多为 "buflen" 字节.

    .. versionadded:: 1.22.0

.. c:function:: int uv_translate_sys_error(int sys_errno)

   返回与给定平台相关错误代码等效的 libuv 错误代码：Unix 上的 POSIX 错误代码（存储在 `errno` 中的错误代码）和 Windows 上的 Win32 错误代码(由 `GetLastError()` 或 `WSAGetLastError()` 返回的错误代码 ).

   如果 `sys_errno` 已经是 libuv 错误，则直接返回.

   .. versionchanged:: 1.10.0 function declared public.
