#ifndef LAB3_TASK_H
#define LAB3_TASK_H

#include <functional>
#include <chrono>

using namespace std;

struct Task {
    function<void()> func;
    int id;

    void execute()
    {
        if (func) func();
    }
};

#endif //LAB3_TASK_H
