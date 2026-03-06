#include "Array.h"
#include <cstdlib>
#include <iostream>

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

    int sum = min + max;

    return {min, max, sum};
}

Result Array::findMutex(int numThreads)
{
    return {0, 0, 0};
}

Result Array::findCAS(int numThreads)
{
    return {0, 0, 0};
}
