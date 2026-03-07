#include <iostream>
#include <chrono>
#include <thread>
#include <string>
#include "Array.cpp"

using namespace std;

void printResults(string nameFunction, Result res, chrono::duration<double, milli> duration)
{
    cout << nameFunction << ": " << endl;
    cout << "Min: " << res.min << endl;
    cout << "Max: " << res.max << endl;
    cout << "Sum: " << res.sum << endl;
    cout << "Time: " << duration.count() << " ms\n" << endl;
    cout << endl;
}

int main() {
    int size = 10;

    Array arr(size);

    arr.printData();
    cout << endl;

    // Sequence algorithm

    auto start_seq = chrono::high_resolution_clock::now();

    Result res_seq = arr.findSeq();

    auto end_seq = chrono::high_resolution_clock::now();
    chrono::duration<double, milli> duration_seq = end_seq - start_seq;

    printResults("Sequence algorithm", res_seq, duration_seq);

    // Mutex
    int numThreads = 2;
    auto start_mutex = chrono::high_resolution_clock::now();

    Result res_mutex = arr.findMutex(numThreads);

    auto end_mutex = chrono::high_resolution_clock::now();
    chrono::duration<double, milli> duration_mutex = end_mutex - start_mutex;

    printResults("With mutex", res_mutex, duration_mutex);

    return 0;
}