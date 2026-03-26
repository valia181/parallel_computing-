#ifndef TASK_QUEUE_H
#define TASK_QUEUE_H

#include <queue>
#include <mutex>
#include <shared_mutex>
#include "Task.h"

using read_write_lock = shared_mutex;
using read_lock = shared_lock<read_write_lock>;
using write_lock = unique_lock<read_write_lock>;

class TaskQueue {
public:
    TaskQueue() = default;
    ~TaskQueue() { clear(); }

    void push(Task task);
    bool pop(Task& task);
    bool empty() const;
    void clear();

private:
    queue<Task> m_tasks;
    mutable read_write_lock m_rw_lock;
};

#endif