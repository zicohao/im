## �̳߳ؽ���

�̳߳���ʲô���ҵļ��������һ��Ԥ���������̣߳�Ȼ����һ������Ա������͵�����Щ�̣߳���ֻ�費�ϰ���Ҫ��ɵ����񽻸��������ͻ�����̵߳���Դ��������ɡ�

��ô����Ա����ô�����أ�һ�ּ򵥵ķ�ʽ���ǣ�����Ա����һ������Ķ��У�����յ��µ����񣬾Ͱ�����ӵ�����β��ÿ���̶߳��Ŷ��У�������зǿգ���ȥ����ͷ��һ������������ÿ������ֻ�ܱ�һ���߳��õ������������˾ͼ���ȥ����ȡ�������û�������ˣ��߳̾����ߣ�ֱ��������в�Ϊ�ա�����������Ա������һ�㣬�����ܻ���û������������ٵ�ʱ������̵߳�������������������ʱ�������̵߳�������������ʵ������Դ�Ķ�̬����

��ô������ʲô�أ��Ժ�̨������Ϊ����ÿһ���û����������һ�������̲߳��ϵ������������ȡ��������ɺ����������һ������

��ͼʾΪ��
![threadpool](https://github.com/Apriluestc/img.org/blob/master/20151224173731480.png)

�̳߳���һ���ô����Ǽ����̴߳��������ٵ�ʱ�䣬��������ʱ��Ƚ϶̵�ʱ������ô��ǳ����������������������Ч�ʡ�

## �̳߳�ʵ��
������ܵ����̳߳ص�һ����ʵ�֣��ڴ�����ʱ��Ԥ������ָ���������̣߳�Ȼ��ȥ�������ȡ��ӽ�����������д���ͺá�

����˵֮�����Ӹ������ԣ�������Ϊѧϰ֮���������汾Ϊ׼�ͺ��ˡ�

��Ŀ��ҳ��[threadpool](https://github.com/mbrossard/threadpool)

### ���ݽṹ
��Ҫ�������Զ�������ݽṹ

#### `threadpool_task_t`
���ڱ���һ���ȴ�ִ�е�����һ��������Ҫָ����Ҫ���еĶ�Ӧ�����������Ĳ�������������� struct ���к���ָ��� void ָ�롣

```c
typedef struct {
    void (*function)(void *);
    void *argument;
} threadpool_task_t;
```

#### `thread_pool_t`
һ���̳߳صĽṹ����Ϊ�� C ���ԣ�����������������������飬��ά������ͷ�Ͷ���β��ʵ�֡�

```c
struct threadpool_t {
  pthread_mutex_t lock;     /* ������ */
  pthread_cond_t notify;    /* �������� */
  pthread_t *threads;       /* �߳��������ʼָ�� */
  threadpool_task_t *queue; /* ��������������ʼָ�� */
  int thread_count;         /* �߳����� */
  int queue_size;           /* ������г��� */
  int head;                 /* ��ǰ�������ͷ */
  int tail;                 /* ��ǰ�������β */
  int count;                /* ��ǰ�����е������� */
  int shutdown;             /* �̳߳ص�ǰ״̬�Ƿ�ر� */
  int started;              /* �������е��߳��� */
};
```

### ����
#### ����ӿ�
* `threadpool_t *threadpool_create(int thread_count, int queue_size, int flags);` �����̳߳أ��� thread_count ָ�������߳�����queue_size ָ��������г��ȣ�flags Ϊ����������δʹ�á�
* `int threadpool_add(threadpool_t *pool, void (*routine)(void *),void *arg, int flags);` �����Ҫִ�е����񡣵ڶ�������Ϊ��Ӧ����ָ�룬������Ϊ��Ӧ����������flags δʹ�á�
* `int threadpool_destroy(threadpool_t *pool, int flags);` ���ٴ��ڵ��̳߳ء�flags ����ָ�������̽�������ƽ�ͽ��������̽���ָ������������Ƿ�Ϊ�գ����̽�����ƽ�ͽ���ָ�ȴ�������е�����ȫ��ִ������ٽ���������������в���������µ�����

#### �ڲ���������
* `static void *threadpool_thread(void *threadpool);` �̳߳�ÿ���߳���ִ�еĺ�����
* `int threadpool_free(threadpool_t *pool);` �ͷ��̳߳���������ڴ���Դ��

## �̳߳�ʹ��
### ����
�ο���Ŀ��Ŀ¼�µ� Makefile, ֱ���� `make` ���롣

### ��������
��Ŀ�ṩ������������������ `threadpool/test/`�������ǿ����Դ���ѧϰ�̳߳ص��÷��������Ƿ����������������ṩ����һ����

```c
#define THREAD 32
#define QUEUE  256

#include <stdio.h
#include <pthread.h
#include <unistd.h
#include <assert.h

#include "threadpool.h"

int tasks = 0, done = 0;
pthread_mutex_t lock;

void dummy_task(void *arg) {
    usleep(10000);
    pthread_mutex_lock(&lock);
    /* ��¼�ɹ���ɵ������� */
    done++;
    pthread_mutex_unlock(&lock);
}

int main(int argc, char **argv)
{
    threadpool_t *pool;

    /* ��ʼ�������� */
    pthread_mutex_init(&lock, NULL);

    /* �����̳߳ش����ɹ� */
    assert((pool = threadpool_create(THREAD, QUEUE, 0)) != NULL);
    fprintf(stderr, "Pool started with %d threads and "
            "queue size of %d\n", THREAD, QUEUE);

    /* ֻҪ������л�û������һֱ��� */
    while(threadpool_add(pool, &dummy_task, NULL, 0) == 0) {
        pthread_mutex_lock(&lock);
        tasks++;
        pthread_mutex_unlock(&lock);
    }

    fprintf(stderr, "Added %d tasks\n", tasks);

    /* ���ϼ���������Ƿ����һ�����ϣ�û����������� */
    while((tasks / 2)  done) {
        usleep(10000);
    }
    /* ��ʱ�������̳߳�,0 ���� immediate_shutdown */
    assert(threadpool_destroy(pool, 0) == 0);
    fprintf(stderr, "Did %d tasks\n", done);

    return 0;
}
```

## Դ��ע��
Դ��ע��һ������ github, [���ҡ�](https://github.com/AngryHacker/code-with-comments#threadpool)

### threadpool.h
```c
/*
 * Copyright (c) 2013, Mathias Brossard <mathias@brossard.org.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *  1. Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *
 *  2. Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef _THREADPOOL_H_
#define _THREADPOOL_H_

#ifdef __cplusplus
/* ���� C++ ��������ָ���� C ���﷨���� */
extern "C" {
#endif

/**
 * @file threadpool.h
 * @brief Threadpool Header File
 */

 /**
 * Increase this constants at your own risk
 * Large values might slow down your system
 */
#define MAX_THREADS 64
#define MAX_QUEUE 65536

/* �򻯱������� */
typedef struct threadpool_t threadpool_t;

/* ��������� */
typedef enum {
    threadpool_invalid        = -1,
    threadpool_lock_failure   = -2,
    threadpool_queue_full     = -3,
    threadpool_shutdown       = -4,
    threadpool_thread_failure = -5
} threadpool_error_t;

typedef enum {
    threadpool_graceful       = 1
} threadpool_destroy_flags_t;

/* �������̳߳��������� API */

/**
 * @function threadpool_create
 * @brief Creates a threadpool_t object.
 * @param thread_count Number of worker threads.
 * @param queue_size   Size of the queue.
 * @param flags        Unused parameter.
 * @return a newly created thread pool or NULL
 */
/**
 * �����̳߳أ��� thread_count ���̣߳����� queue_size ����������У�flags ����û��ʹ��
 */
threadpool_t *threadpool_create(int thread_count, int queue_size, int flags);

/**
 * @function threadpool_add
 * @brief add a new task in the queue of a thread pool
 * @param pool     Thread pool to which add the task.
 * @param function Pointer to the function that will perform the task.
 * @param argument Argument to be passed to the function.
 * @param flags    Unused parameter.
 * @return 0 if all goes well, negative values in case of error (@see
 * threadpool_error_t for codes).
 */
/**
 *  ��������̳߳�, pool Ϊ�̳߳�ָ�룬routine Ϊ����ָ�룬 arg Ϊ���������� flags δʹ��
 */
int threadpool_add(threadpool_t *pool, void (*routine)(void *),
                   void *arg, int flags);

/**
 * @function threadpool_destroy
 * @brief Stops and destroys a thread pool.
 * @param pool  Thread pool to destroy.
 * @param flags Flags for shutdown
 *
 * Known values for flags are 0 (default) and threadpool_graceful in
 * which case the thread pool doesn't accept any new tasks but
 * processes all pending tasks before shutdown.
 */
/**
 * �����̳߳أ�flags ��������ָ���رյķ�ʽ
 */
int threadpool_destroy(threadpool_t *pool, int flags);

#ifdef __cplusplus
}
#endif

#endif /* _THREADPOOL_H_ */
```

### threadpool.c
```c
/*
 * Copyright (c) 2013, Mathias Brossard <mathias@brossard.org.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *  1. Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *
 *  2. Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/**
 * @file threadpool.c
 * @brief Threadpool implementation file
 */

#include <stdlib.h
#include <pthread.h
#include <unistd.h

#include "threadpool.h"

/**
 * �̳߳عرյķ�ʽ
 */
typedef enum {
    immediate_shutdown = 1,
    graceful_shutdown  = 2
} threadpool_shutdown_t;

/**
 *  @struct threadpool_task
 *  @brief the work struct
 *
 *  @var function Pointer to the function that will perform the task.
 *  @var argument Argument to be passed to the function.
 */
/**
 * �̳߳�һ������Ķ���
 */

typedef struct {
    void (*function)(void *);
    void *argument;
} threadpool_task_t;

/**
 *  @struct threadpool
 *  @brief The threadpool struct
 *
 *  @var notify       Condition variable to notify worker threads.
 *  @var threads      Array containing worker threads ID.
 *  @var thread_count Number of threads
 *  @var queue        Array containing the task queue.
 *  @var queue_size   Size of the task queue.
 *  @var head         Index of the first element.
 *  @var tail         Index of the next element.
 *  @var count        Number of pending tasks
 *  @var shutdown     Flag indicating if the pool is shutting down
 *  @var started      Number of started threads
 */
/**
 * �̳߳صĽṹ����
 *  @var lock         �����ڲ������Ļ�����
 *  @var notify       �̼߳�֪ͨ����������
 *  @var threads      �߳����飬������ָ������ʾ�������� = ��Ԫ��ָ��
 *  @var thread_count �߳�����
 *  @var queue        �洢��������飬���������
 *  @var queue_size   ������д�С
 *  @var head         ����������׸�����λ�ã�ע���������������������δ��ʼ���еģ�
 *  @var tail         ������������һ���������һ��λ�ã�ע������������洢��head �� tail ָʾ����λ�ã�
 *  @var count        �����������������������ȴ����е�������
 *  @var shutdown     ��ʾ�̳߳��Ƿ�ر�
 *  @var started      ��ʼ���߳���
 */
struct threadpool_t {
  pthread_mutex_t lock;
  pthread_cond_t notify;
  pthread_t *threads;
  threadpool_task_t *queue;
  int thread_count;
  int queue_size;
  int head;
  int tail;
  int count;
  int shutdown;
  int started;
};

/**
 * @function void *threadpool_thread(void *threadpool)
 * @brief the worker thread
 * @param threadpool the pool which own the thread
 */
/**
 * �̳߳���ÿ���߳����ܵĺ���
 * ���� static Ӧ��ֻΪ��ʹ����ֻ�ڱ��ļ�����Ч
 */
static void *threadpool_thread(void *threadpool);

int threadpool_free(threadpool_t *pool);

threadpool_t *threadpool_create(int thread_count, int queue_size, int flags)
{
    if(thread_count <= 0 || thread_count  MAX_THREADS || queue_size <= 0 || queue_size  MAX_QUEUE) {
        return NULL;
    }

    threadpool_t *pool;
    int i;

    /* �����ڴ洴���ڴ�ض��� */
    if((pool = (threadpool_t *)malloc(sizeof(threadpool_t))) == NULL) {
        goto err;
    }

    /* Initialize */
    pool-thread_count = 0;
    pool-queue_size = queue_size;
    pool-head = pool-tail = pool-count = 0;
    pool-shutdown = pool-started = 0;

    /* Allocate thread and task queue */
    /* �����߳�������������������ڴ� */
    pool-threads = (pthread_t *)malloc(sizeof(pthread_t) * thread_count);
    pool-queue = (threadpool_task_t *)malloc
        (sizeof(threadpool_task_t) * queue_size);

    /* Initialize mutex and conditional variable first */
    /* ��ʼ������������������ */
    if((pthread_mutex_init(&(pool-lock), NULL) != 0) ||
       (pthread_cond_init(&(pool-notify), NULL) != 0) ||
       (pool-threads == NULL) ||
       (pool-queue == NULL)) {
        goto err;
    }

    /* Start worker threads */
    /* ����ָ���������߳̿�ʼ���� */
    for(i = 0; i < thread_count; i++) {
        if(pthread_create(&(pool-threads[i]), NULL,
                          threadpool_thread, (void*)pool) != 0) {
            threadpool_destroy(pool, 0);
            return NULL;
        }
        pool-thread_count++;
        pool-started++;
    }

    return pool;

 err:
    if(pool) {
        threadpool_free(pool);
    }
    return NULL;
}

int threadpool_add(threadpool_t *pool, void (*function)(void *),
                   void *argument, int flags)
{
    int err = 0;
    int next;

    if(pool == NULL || function == NULL) {
        return threadpool_invalid;
    }

    /* ������ȡ�û���������Ȩ */
    if(pthread_mutex_lock(&(pool-lock)) != 0) {
        return threadpool_lock_failure;
    }

    /* ������һ�����Դ洢 task ��λ�� */
    next = pool-tail + 1;
    next = (next == pool-queue_size) ? 0 : next;

    do {
        /* Are we full ? */
        /* ����Ƿ���������� */
        if(pool-count == pool-queue_size) {
            err = threadpool_queue_full;
            break;
        }

        /* Are we shutting down ? */
        /* ��鵱ǰ�̳߳�״̬�Ƿ�ر� */
        if(pool-shutdown) {
            err = threadpool_shutdown;
            break;
        }

        /* Add task to queue */
        /* �� tail ��λ�÷��ú���ָ��Ͳ�������ӵ�������� */
        pool-queue[pool-tail].function = function;
        pool-queue[pool-tail].argument = argument;
        /* ���� tail �� count */
        pool-tail = next;
        pool-count += 1;

        /* pthread_cond_broadcast */
        /*
         * ���� signal,��ʾ�� task ����ӽ�����
         * �������Ϊ������п��������̣߳���ʱ����һ��������
         * ���û����ʲô������
         */
        if(pthread_cond_signal(&(pool-notify)) != 0) {
            err = threadpool_lock_failure;
            break;
        }
        /*
         * �����õ��� do { ... } while(0) �ṹ
         * ��֤������౻ִ��һ�Σ������м䷽����Ϊ�쳣������ִ�п�
         */
    } while(0);

    /* �ͷŻ�������Դ */
    if(pthread_mutex_unlock(&pool-lock) != 0) {
        err = threadpool_lock_failure;
    }

    return err;
}

int threadpool_destroy(threadpool_t *pool, int flags)
{
    int i, err = 0;

    if(pool == NULL) {
        return threadpool_invalid;
    }

    /* ȡ�û�������Դ */
    if(pthread_mutex_lock(&(pool-lock)) != 0) {
        return threadpool_lock_failure;
    }

    do {
        /* Already shutting down */
        /* �ж��Ƿ����������ط��ر� */
        if(pool-shutdown) {
            err = threadpool_shutdown;
            break;
        }

        /* ��ȡָ���Ĺرշ�ʽ */
        pool-shutdown = (flags & threadpool_graceful) ?
            graceful_shutdown : immediate_shutdown;

        /* Wake up all worker threads */
        /* �������������������������̣߳����ͷŻ����� */
        if((pthread_cond_broadcast(&(pool-notify)) != 0) ||
           (pthread_mutex_unlock(&(pool-lock)) != 0)) {
            err = threadpool_lock_failure;
            break;
        }

        /* Join all worker thread */
        /* �ȴ������߳̽��� */
        for(i = 0; i < pool-thread_count; i++) {
            if(pthread_join(pool-threads[i], NULL) != 0) {
                err = threadpool_thread_failure;
            }
        }
        /* ͬ���� do{...} while(0) �ṹ*/
    } while(0);

    /* Only if everything went well do we deallocate the pool */
    if(!err) {
        /* �ͷ��ڴ���Դ */
        threadpool_free(pool);
    }
    return err;
}

int threadpool_free(threadpool_t *pool)
{
    if(pool == NULL || pool-started  0) {
        return -1;
    }

    /* Did we manage to allocate ? */
    /* �ͷ��߳� ������� ������ �������� �̳߳���ռ�ڴ���Դ */
    if(pool-threads) {
        free(pool-threads);
        free(pool-queue);

        /* Because we allocate pool-threads after initializing the
           mutex and condition variable, we're sure they're
           initialized. Let's lock the mutex just in case. */
        pthread_mutex_lock(&(pool-lock));
        pthread_mutex_destroy(&(pool-lock));
        pthread_cond_destroy(&(pool-notify));
    }
    free(pool);
    return 0;
}


static void *threadpool_thread(void *threadpool)
{
    threadpool_t *pool = (threadpool_t *)threadpool;
    threadpool_task_t task;

    for(;;) {
        /* Lock must be taken to wait on conditional variable */
        /* ȡ�û�������Դ */
        pthread_mutex_lock(&(pool-lock));

        /* Wait on condition variable, check for spurious wakeups.
           When returning from pthread_cond_wait(), we own the lock. */
        /* �� while ��Ϊ���ڻ���ʱ���¼������ */
        while((pool-count == 0) && (!pool-shutdown)) {
            /* �������Ϊ�գ����̳߳�û�йر�ʱ���������� */
            pthread_cond_wait(&(pool-notify), &(pool-lock));
        }

        /* �رյĴ��� */
        if((pool-shutdown == immediate_shutdown) ||
           ((pool-shutdown == graceful_shutdown) &&
            (pool-count == 0))) {
            break;
        }

        /* Grab our task */
        /* ȡ��������еĵ�һ������ */
        task.function = pool-queue[pool-head].function;
        task.argument = pool-queue[pool-head].argument;
        /* ���� head �� count */
        pool-head += 1;
        pool-head = (pool-head == pool-queue_size) ? 0 : pool-head;
        pool-count -= 1;

        /* Unlock */
        /* �ͷŻ����� */
        pthread_mutex_unlock(&(pool-lock));

        /* Get to work */
        /* ��ʼ�������� */
        (*(task.function))(task.argument);
        /* ����һ���������н��� */
    }

    /* �߳̽����������������߳��� */
    pool-started--;

    pthread_mutex_unlock(&(pool-lock));
    pthread_exit(NULL);
    return(NULL);
}
```
