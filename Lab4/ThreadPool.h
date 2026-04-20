#ifndef THREAD_POOL_H
#define THREAD_POOL_H

#include <vector>
#include <thread>
#include <condition_variable>
#include <functional>
#include "TaskQueue.cpp"

class ThreadPool {
public:
    ThreadPool() = default;
    ~ThreadPool();

    ThreadPool(const ThreadPool& other) = delete;
    ThreadPool(ThreadPool&& other) = delete;
    ThreadPool& operator=(const ThreadPool& rhs) = delete;
    ThreadPool& operator=(ThreadPool&& rhs) = delete;

    void initialize(size_t num_queues, size_t threads_per_queue);
    void terminate();

    void pause();
    void resume();

    void add_task(Task task);

private:
    bool working_unsafe() const;
    void routine(size_t queue_index);

    size_t m_num_queues = 0;

    mutable read_write_lock m_rw_lock;

    std::vector<std::condition_variable_any*> m_waiters;
    std::vector<std::thread> m_workers;
    std::vector<TaskQueue*> m_task_queues;

    bool m_initialized = false;
    bool m_terminated = false;
    bool m_paused = false;
};

#endif