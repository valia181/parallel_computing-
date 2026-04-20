#ifndef LAB3_TASK_H
#define LAB3_TASK_H
#include <functional>
#include <chrono>


struct Task {
    std::function<void()> action;
    SOCKET socket;

    Task() = default;

    Task(std::function<void()> f, SOCKET client_socket): action(std::move(f)), socket(client_socket) {}
};

#endif //LAB3_TASK_H
