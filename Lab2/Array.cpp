#include "Array.h"
#include <cstdlib>
#include <iostream>
#include <mutex>

Array::Array(int n) : size(n)
{
    data.resize(size);
    srand(time(0));
    for (int i = 0; i < size; i++)
    {
        data[i] = rand() % 1000000;
    }
}

void Array::printData()
{
    cout << "Data: ";

    for (int i = 0; i < data.size(); i++)
    {
        cout << data[i] << " ";
    }

    cout << endl;
}

Result Array::findSeq()
{
    int min = data[0];
    int max = data[0];

    for (int i = 1; i < size; ++i)
    {
        if (data[i] < min) min = data[i];
        if (data[i] > max) max = data[i];
    }

    return {min, max, min + max};
}

Result Array::findMutex(int numThreads)
{
    int min = data[0];
    int max = data[0];
    mutex mtx;

    vector<thread> threads;

    auto localWithThreads = [&](int thread_id)
    {
        int local_min = data[thread_id];
        int local_max = data[thread_id];
        for (int i = thread_id; i < size; i += numThreads)
        {
            if (data[i] < local_min) local_min = data[i];
            if (data[i] > local_max) local_max = data[i];
        }

        lock_guard<mutex> lock(mtx);
        if (local_min < min) min = local_min;
        if (local_max > max) max = local_max;
    };

    for (int i = 0; i < numThreads; i++)
    {
        threads.emplace_back(localWithThreads, i);
    }

    for (int i = 0; i < numThreads; i++)
    {
        threads[i].join();
    }

    return {min, max, min + max};
}

Result Array::findCAS(int numThreads)
{
    return {0, 0, 0};
}
