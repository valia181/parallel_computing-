#ifndef TASK_QUEUE_H
#define TASK_QUEUE_H

#include <queue>
#include <mutex>
#include <shared_mutex>
#include "Task.h"

using read_write_lock = std::shared_mutex;
using read_lock = std::shared_lock<read_write_lock>;
using write_lock = std::unique_lock<read_write_lock>;

class TaskQueue {
public:
    TaskQueue() = default;
    ~TaskQueue()
    {
        clear();
    }

    TaskQueue(const TaskQueue&) = delete;
    TaskQueue(TaskQueue&&) = delete;
    TaskQueue& operator=(const TaskQueue&) = delete;
    TaskQueue& operator=(TaskQueue&&) = delete;

    void push(Task task);
    bool pop(Task& task);
    void clear();

    bool empty() const;
    size_t size() const;

private:
    std::queue<Task> m_tasks;
    mutable read_write_lock m_rw_lock;
};

#endif