
.. _fs:

文件系统操作
======================

libuv 提供了广泛的跨平台同步和异步文件系统操作。 本文档中定义的所有函数都接受一个回调，允许为 NULL。 如果回调为NULL，则请求同步完成，否则将异步执行.

所有文件操作都在线程池上运行。 有关线程池大小的信息，请参阅 :ref:`threadpool`.

.. note::
     在 Windows 上，`uv_fs_*` 函数使用 utf-8 编码.

Data types
----------

.. c:type:: uv_fs_t

    File system request type.

.. c:type:: uv_timespec_t

    Portable equivalent of ``struct timespec``.

    ::

        typedef struct {
            long tv_sec;
            long tv_nsec;
        } uv_timespec_t;

.. c:type:: uv_stat_t

    Portable equivalent of ``struct stat``.

    ::

        typedef struct {
            uint64_t st_dev;
            uint64_t st_mode;
            uint64_t st_nlink;
            uint64_t st_uid;
            uint64_t st_gid;
            uint64_t st_rdev;
            uint64_t st_ino;
            uint64_t st_size;
            uint64_t st_blksize;
            uint64_t st_blocks;
            uint64_t st_flags;
            uint64_t st_gen;
            uv_timespec_t st_atim;
            uv_timespec_t st_mtim;
            uv_timespec_t st_ctim;
            uv_timespec_t st_birthtim;
        } uv_stat_t;

.. c:enum:: uv_fs_type

    文件系统请求类型.

    ::

        typedef enum {
            UV_FS_UNKNOWN = -1,
            UV_FS_CUSTOM,
            UV_FS_OPEN,
            UV_FS_CLOSE,
            UV_FS_READ,
            UV_FS_WRITE,
            UV_FS_SENDFILE,
            UV_FS_STAT,
            UV_FS_LSTAT,
            UV_FS_FSTAT,
            UV_FS_FTRUNCATE,
            UV_FS_UTIME,
            UV_FS_FUTIME,
            UV_FS_ACCESS,
            UV_FS_CHMOD,
            UV_FS_FCHMOD,
            UV_FS_FSYNC,
            UV_FS_FDATASYNC,
            UV_FS_UNLINK,
            UV_FS_RMDIR,
            UV_FS_MKDIR,
            UV_FS_MKDTEMP,
            UV_FS_RENAME,
            UV_FS_SCANDIR,
            UV_FS_LINK,
            UV_FS_SYMLINK,
            UV_FS_READLINK,
            UV_FS_CHOWN,
            UV_FS_FCHOWN,
            UV_FS_REALPATH,
            UV_FS_COPYFILE,
            UV_FS_LCHOWN,
            UV_FS_OPENDIR,
            UV_FS_READDIR,
            UV_FS_CLOSEDIR,
            UV_FS_MKSTEMP,
            UV_FS_LUTIME
        } uv_fs_type;

.. c:type:: uv_statfs_t

    简化 ``struct statfs`` 跨平台等价.
    使用 :c:func:`uv_fs_statfs`.

    ::

        typedef struct uv_statfs_s {
            uint64_t f_type;
            uint64_t f_bsize;
            uint64_t f_blocks;
            uint64_t f_bfree;
            uint64_t f_bavail;
            uint64_t f_files;
            uint64_t f_ffree;
            uint64_t f_spare[4];
        } uv_statfs_t;

.. c:enum:: uv_dirent_t

    Cross platform (reduced) equivalent of ``struct dirent``.
    Used in :c:func:`uv_fs_scandir_next`.

    ::

        typedef enum {
            UV_DIRENT_UNKNOWN,
            UV_DIRENT_FILE,
            UV_DIRENT_DIR,
            UV_DIRENT_LINK,
            UV_DIRENT_FIFO,
            UV_DIRENT_SOCKET,
            UV_DIRENT_CHAR,
            UV_DIRENT_BLOCK
        } uv_dirent_type_t;

        typedef struct uv_dirent_s {
            const char* name;
            uv_dirent_type_t type;
        } uv_dirent_t;

.. c:type:: uv_dir_t

    用于流目录迭代的数据类型。
    由 :c:func:`uv_fs_opendir()`、:c:func:`uv_fs_readdir()` 和 :c:func:`uv_fs_closedir()` 使用。 `dirents` 表示用户提供的用于保存结果的 `uv_dirent_t` 数组。 `nentries` 是用户提供的 `dirents` 的最大数组大小.

    ::

        typedef struct uv_dir_s {
            uv_dirent_t* dirents;
            size_t nentries;
        } uv_dir_t;


Public members
^^^^^^^^^^^^^^

.. c:member:: uv_loop_t* uv_fs_t.loop

    启动此请求的循环以及将报告完成的位置。
    只读。

.. c:member:: uv_fs_type uv_fs_t.fs_type

    FS request type.

.. c:member:: const char* uv_fs_t.path

    影响请求的路径.

.. c:member:: ssize_t uv_fs_t.result

    请求的结果。 < 0 表示错误，否则成功。 在 :c:func:`uv_fs_read` 或 :c:func:`uv_fs_write` 等请求上，它分别指示读取或写入的数据量.

.. c:member:: uv_stat_t uv_fs_t.statbuf

    存储:c:func:`uv_fs_stat` 和其他统计请求的结果.

.. c:member:: void* uv_fs_t.ptr

    存储 :c:func:`uv_fs_readlink` 和 :c:func:`uv_fs_realpath` 的结果并作为 `statbuf` 的别名.

.. seealso:: The :c:type:`uv_req_t` members also apply.


API
---

.. c:function:: void uv_fs_req_cleanup(uv_fs_t* req)

    清理请求。 必须在请求完成后调用以释放 libuv 可能已分配的任何内存.

.. c:function:: int uv_fs_close(uv_loop_t* loop, uv_fs_t* req, uv_file file, uv_fs_cb cb)

    Equivalent to :man:`close(2)`.

.. c:function:: int uv_fs_open(uv_loop_t* loop, uv_fs_t* req, const char* path, int flags, int mode, uv_fs_cb cb)

    Equivalent to :man:`open(2)`.

    .. note::
        在 Windows 上，libuv 使用`CreateFileW`，因此文件总是以二进制模式打开。 因此，不支持 O_BINARY 和 O_TEXT 标志.

.. c:function:: int uv_fs_read(uv_loop_t* loop, uv_fs_t* req, uv_file file, const uv_buf_t bufs[], unsigned int nbufs, int64_t offset, uv_fs_cb cb)

    Equivalent to :man:`preadv(2)`.

    .. warning::
        在 Windows 上，在非 MSVC 环境下（例如，当使用 GCC 或 Clang 构建 libuv 时），如果内存映射读取操作失败，使用“UV_FS_O_FILEMAP”打开的文件可能会导致致命崩溃.

.. c:function:: int uv_fs_unlink(uv_loop_t* loop, uv_fs_t* req, const char* path, uv_fs_cb cb)

    Equivalent to :man:`unlink(2)`.

.. c:function:: int uv_fs_write(uv_loop_t* loop, uv_fs_t* req, uv_file file, const uv_buf_t bufs[], unsigned int nbufs, int64_t offset, uv_fs_cb cb)

    Equivalent to :man:`pwritev(2)`.

    .. warning::
        在 Windows 上，在非 MSVC 环境下（例如，当使用 GCC 或 Clang 构建 libuv 时），如果内存映射写入操作失败，使用“UV_FS_O_FILEMAP”打开的文件可能会导致致命崩溃.

.. c:function:: int uv_fs_mkdir(uv_loop_t* loop, uv_fs_t* req, const char* path, int mode, uv_fs_cb cb)

    Equivalent to :man:`mkdir(2)`.

    .. note::
        `mode` is currently not implemented on Windows.

.. c:function:: int uv_fs_mkdtemp(uv_loop_t* loop, uv_fs_t* req, const char* tpl, uv_fs_cb cb)

    Equivalent to :man:`mkdtemp(3)`. The result can be found as a null terminated string at `req->path`.

.. c:function:: int uv_fs_mkstemp(uv_loop_t* loop, uv_fs_t* req, const char* tpl, uv_fs_cb cb)

    Equivalent to :man:`mkstemp(3)`. The created file path can be found as a null terminated string at `req->path`.
    The file descriptor can be found as an integer at `req->result`.

    .. versionadded:: 1.34.0

.. c:function:: int uv_fs_rmdir(uv_loop_t* loop, uv_fs_t* req, const char* path, uv_fs_cb cb)

    Equivalent to :man:`rmdir(2)`.

.. c:function:: int uv_fs_opendir(uv_loop_t* loop, uv_fs_t* req, const char* path, uv_fs_cb cb)

    打开 `path` 作为目录流。 成功时，通过 `req->ptr` 分配并返回一个`uv_dir_t`。 `uv_fs_req_cleanup()` 不会释放此内存，尽管`req->ptr` 设置为`NULL`。 分配的内存必须通过调用`uv_fs_closedir()`来释放。 失败时，不分配内存.

    可以通过将生成的 `uv_dir_t` 传递给 `uv_fs_readdir()` 来迭代目录的内容.

    .. versionadded:: 1.28.0

.. c:function:: int uv_fs_closedir(uv_loop_t* loop, uv_fs_t* req, uv_dir_t* dir, uv_fs_cb cb)

    关闭 `dir` 表示的目录流并释放 `uv_fs_opendir()` 分配的内存.

    .. versionadded:: 1.28.0

.. c:function:: int uv_fs_readdir(uv_loop_t* loop, uv_fs_t* req, uv_dir_t* dir, uv_fs_cb cb)

    遍历由成功的 uv_fs_opendir() 调用返回的目录流 dir。 在调用 `uv_fs_readdir()` 之前，调用者必须设置 `dir->dirents` 和 `dir->nentries`，代表 :c:type:`uv_dirent_t` 元素的数组，用于保存读取的目录条目及其大小.

    成功时，结果是一个整数 >= 0，表示从流中读取的条目数.

    .. versionadded:: 1.28.0

    .. warning::
        `uv_fs_readdir()` is not thread safe.

    .. note::
        此函数不返回 `.`` 和 `..` 条目.

    .. note::
        成功时，此函数分配必须使用 uv_fs_req_cleanup() 释放的内存。 必须在使用 uv_fs_closedir() 关闭目录之前调用 uv_fs_req_cleanup().

.. c:function:: int uv_fs_scandir(uv_loop_t* loop, uv_fs_t* req, const char* path, int flags, uv_fs_cb cb)
.. c:function:: int uv_fs_scandir_next(uv_fs_t* req, uv_dirent_t* ent)

    等效于:man:`scandir(3)`，API 略有不同。 一旦调用了请求的回调，用户就可以使用 :c:func:`uv_fs_scandir_next` 来获取 `ent` 填充下一个目录条目数据。 当没有更多条目时，将返回“UV_EOF”.

    .. note::
        与 `scandir(3)` 不同，此函数不返回 "." 和“..”条目.

    .. note::
        在 Linux 上，仅某些文件系统（btrfs、ext2、ext3 和 ext4 在撰写本文时）支持获取条目的类型，请查看:man:`getdents(2)` 手册页.

.. c:function:: int uv_fs_stat(uv_loop_t* loop, uv_fs_t* req, const char* path, uv_fs_cb cb)
.. c:function:: int uv_fs_fstat(uv_loop_t* loop, uv_fs_t* req, uv_file file, uv_fs_cb cb)
.. c:function:: int uv_fs_lstat(uv_loop_t* loop, uv_fs_t* req, const char* path, uv_fs_cb cb)

    Equivalent to :man:`stat(2)`, :man:`fstat(2)` and :man:`lstat(2)` respectively.

.. c:function:: int uv_fs_statfs(uv_loop_t* loop, uv_fs_t* req, const char* path, uv_fs_cb cb)

    相当于:man:`statfs(2)`。 成功时，分配一个 `uv_statfs_t` 并通过 `req->ptr` 返回。 该内存由 uv_fs_req_cleanup() 释放.

    .. note::
        底层操作系统不支持的结果 `uv_statfs_t` 中的任何字段都设置为零.

    .. versionadded:: 1.31.0

.. c:function:: int uv_fs_rename(uv_loop_t* loop, uv_fs_t* req, const char* path, const char* new_path, uv_fs_cb cb)

    Equivalent to :man:`rename(2)`.

.. c:function:: int uv_fs_fsync(uv_loop_t* loop, uv_fs_t* req, uv_file file, uv_fs_cb cb)

    Equivalent to :man:`fsync(2)`.

    .. note::
        对于 AIX，`uv_fs_fsync` 在引用非常规文件的文件描述符上返回 `UV_EBADF`.

.. c:function:: int uv_fs_fdatasync(uv_loop_t* loop, uv_fs_t* req, uv_file file, uv_fs_cb cb)

    Equivalent to :man:`fdatasync(2)`.

.. c:function:: int uv_fs_ftruncate(uv_loop_t* loop, uv_fs_t* req, uv_file file, int64_t offset, uv_fs_cb cb)

    Equivalent to :man:`ftruncate(2)`.

.. c:function:: int uv_fs_copyfile(uv_loop_t* loop, uv_fs_t* req, const char* path, const char* new_path, int flags, uv_fs_cb cb)

    将文件从 `path` 复制到 `new_path`。 支持的 `标志` 如下所述.

    - `UV_FS_COPYFILE_EXCL`: 如果存在，如果目标路径已经存在，`uv_fs_copyfile()` 将失败并返回 `UV_EEXIST`。 默认行为是覆盖目标（如果存在）.
    - `UV_FS_COPYFILE_FICLONE`: 如果存在，`uv_fs_copyfile()` 将尝试创建一个写时复制引用链接。 如果底层平台不支持写时复制，或者尝试使用写时复制时发生错误，则使用基于:c:func:`uv_fs_sendfile()`的后备复制机制.
    - `UV_FS_COPYFILE_FICLONE_FORCE`: 如果存在，`uv_fs_copyfile()` 将尝试创建一个写时复制引用链接。 如果底层平台不支持写时复制，或者尝试使用写时复制时出错，则返回错误.

    .. warning::
        如果创建了目标路径，但在复制数据时发生错误，则删除目标路径。 在关闭和删除文件之间有一个短暂的时间窗口，另一个进程可以访问该文件.

    .. versionadded:: 1.14.0

    .. versionchanged:: 1.20.0 `UV_FS_COPYFILE_FICLONE` and
        `UV_FS_COPYFILE_FICLONE_FORCE` are supported.

    .. versionchanged:: 1.33.0 If an error occurs while using
        `UV_FS_COPYFILE_FICLONE_FORCE`, that error is returned. Previously,
        all errors were mapped to `UV_ENOTSUP`.

.. c:function:: int uv_fs_sendfile(uv_loop_t* loop, uv_fs_t* req, uv_file out_fd, uv_file in_fd, int64_t in_offset, size_t length, uv_fs_cb cb)

    Limited equivalent to :man:`sendfile(2)`.

.. c:function:: int uv_fs_access(uv_loop_t* loop, uv_fs_t* req, const char* path, int mode, uv_fs_cb cb)

    Equivalent to :man:`access(2)` on Unix. Windows uses ``GetFileAttributesW()``.

.. c:function:: int uv_fs_chmod(uv_loop_t* loop, uv_fs_t* req, const char* path, int mode, uv_fs_cb cb)
.. c:function:: int uv_fs_fchmod(uv_loop_t* loop, uv_fs_t* req, uv_file file, int mode, uv_fs_cb cb)

    Equivalent to :man:`chmod(2)` and :man:`fchmod(2)` respectively.

.. c:function:: int uv_fs_utime(uv_loop_t* loop, uv_fs_t* req, const char* path, double atime, double mtime, uv_fs_cb cb)
.. c:function:: int uv_fs_futime(uv_loop_t* loop, uv_fs_t* req, uv_file file, double atime, double mtime, uv_fs_cb cb)
.. c:function:: int uv_fs_lutime(uv_loop_t* loop, uv_fs_t* req, const char* path, double atime, double mtime, uv_fs_cb cb)

    Equivalent to :man:`utime(2)`, :man:`futimes(3)` and :man:`lutimes(3)` respectively.

    .. note::
      z/OS: `uv_fs_lutime()` is not implemented for z/OS. It can still be called but will return
      ``UV_ENOSYS``.

    .. note::
      AIX: `uv_fs_futime()` and `uv_fs_lutime()` functions only work for AIX 7.1 and newer.
      They can still be called on older versions but will return ``UV_ENOSYS``.

    .. versionchanged:: 1.10.0 sub-second precission is supported on Windows

.. c:function:: int uv_fs_link(uv_loop_t* loop, uv_fs_t* req, const char* path, const char* new_path, uv_fs_cb cb)

    Equivalent to :man:`link(2)`.

.. c:function:: int uv_fs_symlink(uv_loop_t* loop, uv_fs_t* req, const char* path, const char* new_path, int flags, uv_fs_cb cb)

    Equivalent to :man:`symlink(2)`.

    .. note::
        On Windows the `flags` parameter can be specified to control how the symlink will
        be created:

            * ``UV_FS_SYMLINK_DIR``: indicates that `path` points to a directory.

            * ``UV_FS_SYMLINK_JUNCTION``: request that the symlink is created
              using junction points.

.. c:function:: int uv_fs_readlink(uv_loop_t* loop, uv_fs_t* req, const char* path, uv_fs_cb cb)

    Equivalent to :man:`readlink(2)`.
    The resulting string is stored in `req->ptr`.

.. c:function:: int uv_fs_realpath(uv_loop_t* loop, uv_fs_t* req, const char* path, uv_fs_cb cb)

    Equivalent to :man:`realpath(3)` on Unix. Windows uses `GetFinalPathNameByHandle <https://docs.microsoft.com/en-us/windows/win32/api/fileapi/nf-fileapi-getfinalpathnamebyhandlea>`_.
    The resulting string is stored in `req->ptr`.

    .. warning::
        此函数具有某些特定于平台的警告，这些警告是在 Node 中使用时发现的.

        * macOS 和其他 BSD：如果在解析给定路径时发现超过 32 个符号链接，此函数将失败并显示 UV_ELOOP。 此限制是硬编码的，无法回避.
        * Windows：虽然此功能在常见情况下有效，但在一些特殊情况下它不起作用:

          - 无法解析由避开卷管理器的工具（例如 ImDisk）创建的 ramdisk 卷中的路径.
          - 使用驱动器号时大小写不一致.
          - 解决的路径绕过 subst'd 驱动器.

        虽然这个功能还是可以使用的，但是如果需要支持上述场景，不建议使用.

        可以查看有关这些问题的背景故事和更多详细信息
        `here <https://github.com/nodejs/node/issues/7726>`_.

    .. note::
      此功能在 Windows XP 和 Windows Server 2003 上未实现。在这些系统上，返回 UV_ENOSYS.

    .. versionadded:: 1.8.0

.. c:function:: int uv_fs_chown(uv_loop_t* loop, uv_fs_t* req, const char* path, uv_uid_t uid, uv_gid_t gid, uv_fs_cb cb)
.. c:function:: int uv_fs_fchown(uv_loop_t* loop, uv_fs_t* req, uv_file file, uv_uid_t uid, uv_gid_t gid, uv_fs_cb cb)
.. c:function:: int uv_fs_lchown(uv_loop_t* loop, uv_fs_t* req, const char* path, uv_uid_t uid, uv_gid_t gid, uv_fs_cb cb)

    Equivalent to :man:`chown(2)`, :man:`fchown(2)` and :man:`lchown(2)` respectively.

    .. note::
        These functions are not implemented on Windows.

    .. versionchanged:: 1.21.0 implemented uv_fs_lchown

.. c:function:: uv_fs_type uv_fs_get_type(const uv_fs_t* req)

    Returns `req->fs_type`.

    .. versionadded:: 1.19.0

.. c:function:: ssize_t uv_fs_get_result(const uv_fs_t* req)

    Returns `req->result`.

    .. versionadded:: 1.19.0

.. c:function:: int uv_fs_get_system_error(const uv_fs_t* req)

    Returns the platform specific error code - `GetLastError()` value on Windows
    and `-(req->result)` on other platforms.

    .. versionadded:: 1.38.0

.. c:function:: void* uv_fs_get_ptr(const uv_fs_t* req)

    Returns `req->ptr`.

    .. versionadded:: 1.19.0

.. c:function:: const char* uv_fs_get_path(const uv_fs_t* req)

    Returns `req->path`.

    .. versionadded:: 1.19.0

.. c:function:: uv_stat_t* uv_fs_get_statbuf(uv_fs_t* req)

    Returns `&req->statbuf`.

    .. versionadded:: 1.19.0

.. seealso:: The :c:type:`uv_req_t` API functions also apply.

Helper functions
----------------

.. c:function:: uv_os_fd_t uv_get_osfhandle(int fd)

   对于 C 运行时中的文件描述符，获取与操作系统相关的句柄。 在 UNIX 上，原封不动地返回 ``fd``。 在 Windows 上，这会调用 `_get_osfhandle <https://docs.microsoft.com/en-us/cpp/c-runtime-library/reference/get-osfhandle?view=vs-2019>`_。 请注意，返回值仍归 C 运行时所有，任何尝试关闭它或在关闭 fd 后使用它都可能导致故障.

    .. versionadded:: 1.12.0

.. c:function:: int uv_open_osfhandle(uv_os_fd_t os_fd)

   对于依赖于操作系统的句柄，在 C 运行时获取文件描述符。在 UNIX 上，返回 ``os_fd`` 原样。 在 Windows 上，这调用了 `_open_osfhandle <https://docs.microsoft.com/en-us/cpp/c-runtime-library/reference/open-osfhandle?view=vs-2019>`_。 请注意，这会消耗参数，任何尝试关闭它或在关闭返回值后使用它都可能导致故障

    .. versionadded:: 1.23.0

File open constants
-------------------

.. c:macro:: UV_FS_O_APPEND

    该文件以附加模式打开。 在每次写入之前，文件偏移量位于文件末尾。

.. c:macro:: UV_FS_O_CREAT

    如果文件不存在，则创建该文件。

.. c:macro:: UV_FS_O_DIRECT

    文件 I/O 直接进出用户空间缓冲区，必须对齐。 缓冲区大小和地址应该是块设备物理扇区大小的倍数。

    .. note::
        `UV_FS_O_DIRECT` is supported on Linux, and on Windows via
        `FILE_FLAG_NO_BUFFERING <https://docs.microsoft.com/en-us/windows/win32/fileio/file-buffering>`_.
        `UV_FS_O_DIRECT` is not supported on macOS.

.. c:macro:: UV_FS_O_DIRECTORY

    如果路径不是目录，则打开失败。

    .. note::
        `UV_FS_O_DIRECTORY` is not supported on Windows.

.. c:macro:: UV_FS_O_DSYNC

    为同步 I/O 打开文件。 将所有数据和最少的元数据刷新到磁盘后，写入操作将完成。

    .. note::
        `UV_FS_O_DSYNC` is supported on Windows via
        `FILE_FLAG_WRITE_THROUGH <https://docs.microsoft.com/en-us/windows/win32/fileio/file-buffering>`_.

.. c:macro:: UV_FS_O_EXCL

    如果设置了 `O_CREAT` 标志并且文件已经存在，则打开失败.

    .. note::
        一般来说，如果在没有 `O_CREAT` 的情况下使用 `O_EXCL`，它的行为是未定义的。 有一个例外：在 Linux 2.6 及更高版本上，如果路径名指的是块设备，则可以在不使用 `O_CREAT` 的情况下使用 `O_EXCL`。 如果系统正在使用块设备（例如，已挂载），则打开将失败并显示错误“EBUSY”.

.. c:macro:: UV_FS_O_EXLOCK

    原子地获取排他锁.

    .. note::
        `UV_FS_O_EXLOCK` is only supported on macOS and Windows.

    .. versionchanged:: 1.17.0 support is added for Windows.

.. c:macro:: UV_FS_O_FILEMAP

    使用内存文件映射来访问文件。 使用此标志时，文件不能同时打开多次.

    .. note::
        `UV_FS_O_FILEMAP` is only supported on Windows.

.. c:macro:: UV_FS_O_NOATIME

    读取文件时不更新文件访问时间.

    .. note::
        `UV_FS_O_NOATIME` is not supported on Windows.

.. c:macro:: UV_FS_O_NOCTTY

    如果路径标识了终端设备，则打开路径不会导致该终端成为进程的控制终端（如果该进程还没有控制终端）.

    .. note::
        `UV_FS_O_NOCTTY` is not supported on Windows.

.. c:macro:: UV_FS_O_NOFOLLOW

    如果路径是符号链接，则打开失败.

    .. note::
        `UV_FS_O_NOFOLLOW` is not supported on Windows.

.. c:macro:: UV_FS_O_NONBLOCK

    如果可能，以非阻塞模式打开文件.

    .. note::
        `UV_FS_O_NONBLOCK` is not supported on Windows.

.. c:macro:: UV_FS_O_RANDOM

    访问是随机的。 系统可以将此作为优化文件缓存的提示.

    .. note::
        `UV_FS_O_RANDOM` is only supported on Windows via
        `FILE_FLAG_RANDOM_ACCESS <https://docs.microsoft.com/en-us/windows/win32/api/fileapi/nf-fileapi-createfilea>`_.

.. c:macro:: UV_FS_O_RDONLY

    打开文件进行只读访问.

.. c:macro:: UV_FS_O_RDWR

    打开文件进行读写访问.

.. c:macro:: UV_FS_O_SEQUENTIAL

    访问旨在从头到尾按顺序进行。 系统可以将此作为优化文件缓存的提示.

    .. note::
        `UV_FS_O_SEQUENTIAL` is only supported on Windows via
        `FILE_FLAG_SEQUENTIAL_SCAN <https://docs.microsoft.com/en-us/windows/win32/api/fileapi/nf-fileapi-createfilea>`_.

.. c:macro:: UV_FS_O_SHORT_LIVED

    该文件是临时文件，如果可能，不应将其刷新到磁盘.

    .. note::
        `UV_FS_O_SHORT_LIVED` is only supported on Windows via
        `FILE_ATTRIBUTE_TEMPORARY <https://docs.microsoft.com/en-us/windows/win32/api/fileapi/nf-fileapi-createfilea>`_.

.. c:macro:: UV_FS_O_SYMLINK

    打开符号链接本身而不是它指向的资源.

.. c:macro:: UV_FS_O_SYNC

    为同步 I/O 打开文件。 一旦所有数据和所有元数据都刷新到磁盘，写入操作将完成.

    .. note::
        `UV_FS_O_SYNC` is supported on Windows via
        `FILE_FLAG_WRITE_THROUGH <https://docs.microsoft.com/en-us/windows/win32/fileio/file-buffering>`_.

.. c:macro:: UV_FS_O_TEMPORARY

    该文件是临时文件，如果可能，不应将其刷新到磁盘.

    .. note::
        `UV_FS_O_TEMPORARY` is only supported on Windows via
        `FILE_ATTRIBUTE_TEMPORARY <https://docs.microsoft.com/en-us/windows/win32/api/fileapi/nf-fileapi-createfilea>`_.

.. c:macro:: UV_FS_O_TRUNC

    如果文件存在并且是常规文件，并且文件被成功打开以进行写访问，则其长度应被截断为零.

.. c:macro:: UV_FS_O_WRONLY

    打开文件以进行只写访问.
