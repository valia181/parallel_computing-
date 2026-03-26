#include "TaskQueue.h"

void TaskQueue::push(Task task)
{
    write_lock lock(m_rw_lock);
    m_tasks.push(std::move(task));
}

bool TaskQueue::pop(Task& task)
{
    write_lock lock(m_rw_lock);
    if (m_tasks.empty()) return false;
    task = std::move(m_tasks.front());
    m_tasks.pop();
    return true;
}

void TaskQueue::clear()
{
    write_lock lock(m_rw_lock);
    while (!m_tasks.empty()) m_tasks.pop();
}

bool TaskQueue::empty() const
{
    read_lock lock(m_rw_lock);
    return m_tasks.empty();
}