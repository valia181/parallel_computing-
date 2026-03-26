#ifndef THREAD_POOL_H
#define THREAD_POOL_H

#include <vector>
#include <thread>
#include <condition_variable>
#include <functional>
#include "TaskQueue.h"

class ThreadPool {
public:
    ThreadPool(int threads, int queues);
    ~ThreadPool();

    void initialize();
    void terminate();

    void add_task(Task task);

    bool working() const;
    bool working_unsafe() const;

private:
    void routine(int queue_index);

    int m_thread_count;
    int m_queue_count;

    TaskQueue* m_queues;
    thread* m_workers;

    mutable condition_variable_any m_task_waiter;
    mutable read_write_lock m_rw_lock;

    bool m_initialized = false;
    bool m_terminated = false;
};

#endif