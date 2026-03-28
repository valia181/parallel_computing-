#ifndef LAB3_TASK_H
#define LAB3_TASK_H

#include <functional>
#include <chrono>

struct Task {
    std::function<void()> action;
    int id;

    Task() = default;

    Task(std::function<void()> f, int task_id): action(std::move(f)), id(task_id) {}
};

#endif //LAB3_TASK_H
