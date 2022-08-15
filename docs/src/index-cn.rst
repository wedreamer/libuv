
Welcome to the libuv documentation
==================================

Overview
--------

libuv 是一个专注于异步 I/O 的多平台支持库。 它最初是为 `Node.js`_ 使用而开发的，但它也被 `Luvit`_、 `Julia`_、 `uvloop`_ 和 `others`_ 使用.

.. note::
    如果您在本文档中发现错误，您可以通过发送
    `pull requests <https://github.com/libuv/libuv>`_!

.. _Node.js: https://nodejs.org
.. _Luvit: https://luvit.io
.. _Julia: https://julialang.org
.. _uvloop: https://github.com/MagicStack/uvloop
.. _others: https://github.com/libuv/libuv/blob/v1.x/LINKS.md


Features
--------

* Full-featured event loop backed by epoll, kqueue, IOCP, event ports.
* Asynchronous TCP and UDP sockets
* Asynchronous DNS resolution
* Asynchronous file and file system operations
* File system events
* ANSI escape code controlled TTY
* IPC with socket sharing, using Unix domain sockets or named pipes (Windows)
* Child processes
* Thread pool
* Signal handling
* High resolution clock
* Threading and synchronization primitives


Documentation
-------------

.. toctree::
   :maxdepth: 1

   design
   api
   guide
   upgrading


Downloads
---------

libuv can be downloaded from `here <https://dist.libuv.org/dist/>`_.


Installation
------------

Installation instructions can be found in `the README <https://github.com/libuv/libuv/blob/master/README.md>`_.

