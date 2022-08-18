# 概览

> 当前文章目的在于分析 libuv 相关源码, 对其主要功能以及实现进行相关分析. 其中主要参考 libuv 的[官方文档](https://docs.libuv.org/en/v1.x/)以及 libuv 的[实现代码](https://github.com/libuv/libuv). 假定环境都是 linux, 跳过 windows 平台相关代码.

主要内容如下:

## 事件循环

## 错误处理

## 文件系统

## 网络

## 进程

## 线程

## 工具


libuv 是跨平台支持库，最初是为 `Node.js`_ 编写的。 它是围绕事件驱动的异步 I/O 模型设计的。

libuv 的事件驱动我们由一个简单的 pipe-echo-server 示例代码来看

```c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <uv.h>

#ifdef _WIN32
#define PIPENAME "\\\\?\\pipe\\echo.sock"
#else
#define PIPENAME "/tmp/echo.sock"
#endif

uv_loop_t *loop;

typedef struct {
    uv_write_t req;
    uv_buf_t buf;
} write_req_t;

void free_write_req(uv_write_t *req) {
    write_req_t *wr = (write_req_t*) req;
    free(wr->buf.base);
    free(wr);
}

void alloc_buffer(uv_handle_t *handle, size_t suggested_size, uv_buf_t *buf) {
  buf->base = malloc(suggested_size);
  buf->len = suggested_size;
}

void echo_write(uv_write_t *req, int status) {
    if (status < 0) {
        fprintf(stderr, "Write error %s\n", uv_err_name(status));
    }
    free_write_req(req);
}

void echo_read(uv_stream_t *client, ssize_t nread, const uv_buf_t *buf) {
    if (nread > 0) {
        write_req_t *req = (write_req_t*) malloc(sizeof(write_req_t));
        req->buf = uv_buf_init(buf->base, nread);
        uv_write((uv_write_t*) req, client, &req->buf, 1, echo_write);
        return;
    }

    if (nread < 0) {
        if (nread != UV_EOF)
            fprintf(stderr, "Read error %s\n", uv_err_name(nread));
        uv_close((uv_handle_t*) client, NULL);
    }

    free(buf->base);
}

void on_new_connection(uv_stream_t *server, int status) {
    if (status == -1) {
        // error!
        return;
    }

    // 处理请求的客户端管道
    uv_pipe_t *client = (uv_pipe_t*) malloc(sizeof(uv_pipe_t));
    uv_pipe_init(loop, client, 0);
    // 接受相关请求
    if (uv_accept(server, (uv_stream_t*) client) == 0) {
        // 注册读操作事件
        uv_read_start((uv_stream_t*) client, alloc_buffer, echo_read);
    }
    else {
        uv_close((uv_handle_t*) client, NULL);
    }
}

void remove_sock(int sig) {
    uv_fs_t req;
    uv_fs_unlink(loop, &req, PIPENAME, NULL);
    exit(0);
}

int main() {
    loop = uv_default_loop();

    uv_pipe_t server;
    // 初始化 tcp server 的管道
    uv_pipe_init(loop, &server, 0);

    // 注册 socket 退出信号处理回调
    signal(SIGINT, remove_sock);

    int r;
    // 绑定操作
    if ((r = uv_pipe_bind(&server, PIPENAME))) {
        fprintf(stderr, "Bind error %s\n", uv_err_name(r));
        return 1;
    }
    // 开始监听, 并且注册回调函数
    if ((r = uv_listen((uv_stream_t*) &server, 128, on_new_connection))) {
        fprintf(stderr, "Listen error %s\n", uv_err_name(r));
        return 2;
    }
    return uv_run(loop, UV_RUN_DEFAULT);
}
```

我们可以看到对于 libuv 的 tcp-echo-server 的实现, 对于大量请求相关操作都在初始化的时候需要进行相关回调注册操作, 这对于 libuv 的使用者来说, 只需要关心对应的操作, 以及重点投入到相关事件函数的编写中去. 这几乎是每一个网络库库的核心功能, 也是为了通用功能的重复使用, 减少相关工作量. 这些回调函数在对应事件发生时, 会经过事件循环来调用. 并非是立即调用的, 事件循环如果立即调用的话就需要保证事件不能被阻塞, 因为事件循环还有其他功能, 并不是简单监听事件, 之后触发事件, 以及执行触发事件的回调. 

事件循环的行为主要包含:

- 启动并运行一个循环
- 在这个循环中持续执行相关操作
- 检查相关事件
- 合理执行相关回调

对于异步而言, libuv 实现异步主要基于 epoll 的异步通知.

### 事件驱动



### 异步


POSIX defines these two terms as follows:
· A synchronous I/O operation causes the requesting process to be blocked until that I/O operation completes.
· An asynchronous I/O operation does not cause the requesting process to be blocked.
Using these definitions, 
the first four I/O models—blocking, nonblocking, I/O multiplexing, and signal-driven I/O—are all synchronous because
 the actual I/O operation (recvfrom) blocks the process. Only the asynchronous I/O model matches the asynchronous 
I/O definition.
<UNIX网络编程>> 第三版 6.2节

这段是关于IO同步异步的描述，书中说到：
一个同步的 IO 操作使得请求进程一直被阻塞，直到 IO 操作完成；
一个异步的 IO 操作不会导致请求进程被阻塞.
 
IO层面
消息处理层面
从IO层面来看, epoll 绝对是同步的；
从消息处理层面来看, epoll 是异步的.
 
在处理 IO 的时候，阻塞和非阻塞都是同步 IO。
只有使用了特殊的 API 才是异步 IO。

![sync-async-aio](http://rgmhwumdz.hb-bkt.clouddn.com/2022-08/2022-08-18/pic_1660772893803-5.png)  


作者：陈硕
链接：https://www.zhihu.com/question/19732473/answer/26091478
来源：知乎
著作权归作者所有。商业转载请联系作者获得授权，非商业转载请注明出处。


我们可以下个结论, 对于 libuv 而言, libuv 使用的是 epoll 来注册事件, epoll 事件的收集来自于事件循环的 poll IO 阶段, 对于相关注册事件的文件而言, 文件相关事件的相关事件会在这个阶段被 libuv 来收集. 其他关于用户态, 内核态的相关操作都是同步的. 而对于事件消息的收集, 对于 libuv 来说是异步的. 所以, 对于应用程序而言 libuv 绝对是异步的.
