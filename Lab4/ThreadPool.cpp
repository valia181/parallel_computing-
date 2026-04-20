#include "ThreadPool.h"
#include <ctime>

void ThreadPool::initialize(size_t num_queues, size_t threads_per_queue)
{
    write_lock lock(m_rw_lock);
    if (m_initialized) return;

    m_num_queues = num_queues;

    for (size_t i = 0; i < m_num_queues; i++)
    {
        m_task_queues.push_back(new TaskQueue());
        m_waiters.push_back(new std::condition_variable_any());
    }

    for (size_t q_idx = 0; q_idx < m_num_queues; q_idx++)
    {
        for (size_t t = 0; t < threads_per_queue; t++)
        {
            m_workers.emplace_back(&ThreadPool::routine, this, q_idx);
        }
    }
    m_initialized = true;
}

void ThreadPool::add_task(Task task)
{
    srand(static_cast<unsigned int>(time(0)));
    size_t target_queue;
    {
        read_lock lock(m_rw_lock);
        if (!working_unsafe()) return;

        target_queue = rand() % m_num_queues;
    }

    m_task_queues[target_queue]->push(std::move(task));
    m_waiters[target_queue]->notify_one();
}

void ThreadPool::routine(size_t queue_index)
{
    while (true)
    {
        Task task;
        bool has_task = false;

        {
            write_lock lock(m_rw_lock);

            m_waiters[queue_index]->wait(lock, [this, queue_index, &task, &has_task]
            {
                has_task = m_task_queues[queue_index]->pop(task);

                return m_terminated || (!m_paused && has_task);
            });
        }

        if (m_terminated && !has_task) return;

        if (has_task)
        {
            task.action();
        }
    }
}

bool ThreadPool::working_unsafe() const
{
    return m_initialized && !m_terminated;
}

void ThreadPool::pause()
{
    write_lock lock(m_rw_lock);
    m_paused = true;
}

void ThreadPool::resume()
{
    {
        write_lock lock(m_rw_lock);
        m_paused = false;
    }
    for (auto& cv : m_waiters) cv->notify_all();
}

ThreadPool::~ThreadPool()
{
    terminate();
}

void ThreadPool::terminate()
{
    {
        write_lock lock(m_rw_lock);
        if (!m_initialized || m_terminated) return;
        m_terminated = true;
    }

    for (auto cv : m_waiters) cv->notify_all();

    for (std::thread& worker : m_workers)
    {
        if (worker.joinable()) worker.join();
    }
    m_workers.clear();

    for (auto q : m_task_queues) delete q;
    for (auto v : m_waiters) delete v;

    m_task_queues.clear();
    m_waiters.clear();
    m_initialized = false;
}