
.. _threading:

Threading and synchronization utilities
=======================================

libuv 为多线程和同步原语提供跨平台实现。该 API 很大程度上遵循 pthreads API.


Data types
----------

.. c:type:: uv_thread_t

    Thread data type.

.. c:type:: void (*uv_thread_cb)(void* arg)

    调用以初始化线程执行的回调。 `arg` 与传递给 :c:func:`uv_thread_create` 的值相同.

.. c:type:: uv_key_t

    Thread-local key data type.

.. c:type:: uv_once_t

    Once-only initializer data type.

.. c:type:: uv_mutex_t

    Mutex data type.

.. c:type:: uv_rwlock_t

    Read-write lock data type.

.. c:type:: uv_sem_t

    Semaphore data type.

.. c:type:: uv_cond_t

    Condition data type.

.. c:type:: uv_barrier_t

    Barrier data type.


API
---

Threads
^^^^^^^

.. c:type:: uv_thread_options_t

    生成新线程的选项（传递给 :c:func:`uv_thread_create_ex`）.

    ::

        typedef struct uv_thread_options_s {
          enum {
            UV_THREAD_NO_FLAGS = 0x00,
            UV_THREAD_HAS_STACK_SIZE = 0x01
          } flags;
          size_t stack_size;
        } uv_thread_options_t;

    更多字段可能随时添加到此结构中，因此不应依赖其确切的布局和大小.

    .. versionadded:: 1.26.0

.. c:function:: int uv_thread_create(uv_thread_t* tid, uv_thread_cb entry, void* arg)

    .. versionchanged:: 1.4.1 returns a UV_E* error code on failure

.. c:function:: int uv_thread_create_ex(uv_thread_t* tid, const uv_thread_options_t* params, uv_thread_cb entry, void* arg)

    类似于 :c:func:`uv_thread_create`，但另外指定了创建新线程的选项。

    如果设置了 `UV_THREAD_HAS_STACK_SIZE`, `stack_size` 指定新线程的堆栈大小.
    `0` 表示应该使用默认值，即表现得好像没有设置标志.
    其他值将四舍五入到最近的页面边界.

    .. versionadded:: 1.26.0

.. c:function:: uv_thread_t uv_thread_self(void)
.. c:function:: int uv_thread_join(uv_thread_t *tid)
.. c:function:: int uv_thread_equal(const uv_thread_t* t1, const uv_thread_t* t2)

Thread-local storage
^^^^^^^^^^^^^^^^^^^^

.. note::
    总线程本地存储大小可能受到限制。也就是说，可能无法创建多个 TLS 密钥.

.. c:function:: int uv_key_create(uv_key_t* key)
.. c:function:: void uv_key_delete(uv_key_t* key)
.. c:function:: void* uv_key_get(uv_key_t* key)
.. c:function:: void uv_key_set(uv_key_t* key, void* value)

Once-only initialization
^^^^^^^^^^^^^^^^^^^^^^^^

运行一次且仅一次的函数。使用相同的守卫并发调用:c:func:`uv_once` 将阻塞除一个之外的所有调用者（未指定哪一个）.
防护应该使用 UV_ONCE_INIT 宏静态初始化.

.. c:function:: void uv_once(uv_once_t* guard, void (*callback)(void))

Mutex locks
^^^^^^^^^^^

函数在成功或错误代码 < 0 时返回 0(当然，除非返回类型为 void).

.. c:function:: int uv_mutex_init(uv_mutex_t* handle)
.. c:function:: int uv_mutex_init_recursive(uv_mutex_t* handle)
.. c:function:: void uv_mutex_destroy(uv_mutex_t* handle)
.. c:function:: void uv_mutex_lock(uv_mutex_t* handle)
.. c:function:: int uv_mutex_trylock(uv_mutex_t* handle)
.. c:function:: void uv_mutex_unlock(uv_mutex_t* handle)

Read-write locks
^^^^^^^^^^^^^^^^

函数在成功或错误代码 < 0 时返回 0(当然，除非返回类型为 void).

.. c:function:: int uv_rwlock_init(uv_rwlock_t* rwlock)
.. c:function:: void uv_rwlock_destroy(uv_rwlock_t* rwlock)
.. c:function:: void uv_rwlock_rdlock(uv_rwlock_t* rwlock)
.. c:function:: int uv_rwlock_tryrdlock(uv_rwlock_t* rwlock)
.. c:function:: void uv_rwlock_rdunlock(uv_rwlock_t* rwlock)
.. c:function:: void uv_rwlock_wrlock(uv_rwlock_t* rwlock)
.. c:function:: int uv_rwlock_trywrlock(uv_rwlock_t* rwlock)
.. c:function:: void uv_rwlock_wrunlock(uv_rwlock_t* rwlock)

Semaphores
^^^^^^^^^^

函数在成功或错误代码 < 0 时返回 0(当然, 除非返回类型为 void).

.. c:function:: int uv_sem_init(uv_sem_t* sem, unsigned int value)
.. c:function:: void uv_sem_destroy(uv_sem_t* sem)
.. c:function:: void uv_sem_post(uv_sem_t* sem)
.. c:function:: void uv_sem_wait(uv_sem_t* sem)
.. c:function:: int uv_sem_trywait(uv_sem_t* sem)

Conditions
^^^^^^^^^^

函数在成功或错误代码 < 0 时返回 0(当然, 除非返回类型为 void).

.. note::
    1. 调用者应该准备好处理 :c:func:`uv_cond_wait` 和 :c:func:`uv_cond_timedwait` 上的虚假唤醒.
    2. :c:func:`uv_cond_timedwait` 的超时参数与调用函数的时间相关.
    3. 在 z/OS 上，:c:func:`uv_cond_timedwait` 的超时参数被转换为等待到期的绝对系统时间。如果当前系统时钟时间超过了在发出条件信号之前计算的绝对时间，则会产生 ETIMEDOUT 错误。等待开始后，等待时间不受系统时钟更改的影响。

.. c:function:: int uv_cond_init(uv_cond_t* cond)
.. c:function:: void uv_cond_destroy(uv_cond_t* cond)
.. c:function:: void uv_cond_signal(uv_cond_t* cond)
.. c:function:: void uv_cond_broadcast(uv_cond_t* cond)
.. c:function:: void uv_cond_wait(uv_cond_t* cond, uv_mutex_t* mutex)
.. c:function:: int uv_cond_timedwait(uv_cond_t* cond, uv_mutex_t* mutex, uint64_t timeout)

Barriers
^^^^^^^^

函数在成功或错误代码 < 0 时返回 0(当然, 除非返回类型为 void).

.. note::
    :c:func:`uv_barrier_wait` 向任意选择的“序列化器”线程返回一个 > 0 的值以方便清理，即.

    ::

        if (uv_barrier_wait(&barrier) > 0)
            uv_barrier_destroy(&barrier);

.. c:function:: int uv_barrier_init(uv_barrier_t* barrier, unsigned int count)
.. c:function:: void uv_barrier_destroy(uv_barrier_t* barrier)
.. c:function:: int uv_barrier_wait(uv_barrier_t* barrier)
