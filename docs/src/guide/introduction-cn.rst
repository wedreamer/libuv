介绍
============

这本 ``书`` 是关于将 libuv_ 用作一个高性能事件 I/O 库，在 Windows 和 Unix 上提供相同的 API。

它旨在涵盖 libuv 的主要领域，但不是全面的参考讨论每个函数和数据结构。 `官方libuv文档`_ 可以查阅完整的细节。

.. _official libuv documentation: https://docs.libuv.org/en/v1.x/

这本书仍在进行中，因此部分可能不完整，但我希望你会喜欢它的成长。

这本书是给谁看的
--------------------

如果你正在读这本书，你要么:

1) 系统程序员，创建低级程序，如守护进程或网络服务和客户。 您发现事件循环方法很好适合您的应用程序并决定使用 libuv.

2) 想要封装平台 API 的 node.js 模块编写者用 C 或 C++ 编写，带有一组 (a) 同步 API, 这些 API 暴露于 JavaScript。 您将纯粹在 node.js 的上下文中使用 libuv。 为了这你将需要一些其他资源，因为这本书不包括部分特定于 v8/node.js.

本书假定您熟悉 C 编程语言。

背景
----------

node.js_ 项目始于 2009 年，作为一个解耦的 JavaScript 环境从浏览器。 使用 Google 的 V8_ 和 Marc Lehmann 的 libev_, node.js将 I/O 模型（事件）与非常适合的语言相结合编程风格； 由于它被浏览器塑造的方式。 作为 node.js 越来越受欢迎，让它在 Windows 上运行很重要，但是 libev 仅在 Unix 上运行。 内核事件通知的 Windows 等效项像 kqueue 或 (e)poll 这样的机制就是 IOCP. libuv 是围绕 libev 的抽象或 IOCP 取决于平台，为用户提供基于 libev 的 API。在 libuv 的 node-v0.9.0 版本中，`libev 已被移除`_。

此后 libuv 不断成熟，成为一个高质量的独立用于系统编程的库。 node.js 之外的用户包括 Mozilla 的 Rust_ 编程语言和多种语言绑定。

本书及代码基于 libuv 版本`v1.42.0`_。

代码
----

所有示例代码和本书的源代码都包含在 GitHub 上的 libuv_ 项目。
克隆或下载 libuv_ ，然后构建它::

    sh autogen.sh
    ./configure
    make

不需要 "make install"。 要构建示例，请在 ``docs/code/`` 目录。

.. _v1.42.0: https://github.com/libuv/libuv/releases/tag/v1.42.0
.. _V8: https://v8.dev
.. _libev: http://software.schmorp.de/pkg/libev.html
.. _libuv: https://github.com/libuv/libuv
.. _node.js: https://www.nodejs.org
.. _libev was removed: https://github.com/joyent/libuv/issues/485
.. _Rust: https://www.rust-lang.org
.. _variety: https://github.com/libuv/libuv/blob/v1.x/LINKS.md
