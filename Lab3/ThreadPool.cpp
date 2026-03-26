#include "ThreadPool.h"

ThreadPool::ThreadPool(int threads, int queues) : m_thread_count(threads), m_queue_count(queues), m_queues(nullptr), m_workers(nullptr)
{
    m_queues = new TaskQueue[m_queue_count];
}

ThreadPool::~ThreadPool()
{
    terminate();
    delete[] m_queues;
    delete[] m_workers;
}

void ThreadPool::initialize()
{
    write_lock lock(m_rw_lock);
    if (m_initialized || m_terminated) return;

    m_workers = new std::thread[m_thread_count];

    for (int i = 0; i < m_thread_count; ++i)
    {
        int q_idx = i % m_queue_count;
        m_workers[i] = std::thread(&ThreadPool::routine, this, q_idx);
    }
    m_initialized = true;
}

void ThreadPool::terminate()
{
    {
        write_lock lock(m_rw_lock);
        if (!working_unsafe()) return;
        m_terminated = true;
    }

    m_task_waiter.notify_all();

    if (m_workers)
    {
        for (int i = 0; i < m_thread_count; ++i)
        {
            if (m_workers[i].joinable()) m_workers[i].join();
        }
    }

    write_lock lock(m_rw_lock);
    m_initialized = false;
    m_terminated = false;
}

void ThreadPool::add_task(Task task)
{
    {
        read_lock lock(m_rw_lock);
        if (!working_unsafe()) return;
    }

    int q_idx = rand() % m_queue_count;
    m_queues[q_idx].push(std::move(task));
    m_task_waiter.notify_all();
}

void ThreadPool::routine(int q_idx)
{
    while (true)
    {
        Task task;
        bool task_acquired = false;

        {
            write_lock lock(m_rw_lock);
            m_task_waiter.wait(lock, [this, q_idx, &task, &task_acquired]
            {
                task_acquired = m_queues[q_idx].pop(task);
                return m_terminated || task_acquired;
            });
        }

        if (m_terminated && !task_acquired) return;
        if (task_acquired) task.execute();
    }
}