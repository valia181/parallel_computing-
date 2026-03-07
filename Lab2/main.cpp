#include <iostream>
#include <chrono>
#include <string>
#include <fstream>
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
    int numThreads = 4;

    vector<int> sizes = {10, 100, 1000, 10000, 100000, 1000000, 10000000};

    ofstream csvFile("result.csv");

    csvFile << "Array_Size,Num_Threads,Seq_Time,Mutex_Time,CAS_Time\n";


    for (int size : sizes) {
        cout << "Size: " << size << endl;

        Array arr(size);

        // Sequence algorithm
        auto start_seq = chrono::high_resolution_clock::now();

        Result res_seq = arr.findSeq();

        auto end_seq = chrono::high_resolution_clock::now();

        chrono::duration<double, milli> duration_seq = end_seq - start_seq;

        // Mutex
        auto start_mutex = chrono::high_resolution_clock::now();

        Result res_mutex = arr.findMutex(numThreads);

        auto end_mutex = chrono::high_resolution_clock::now();

        chrono::duration<double, milli> duration_mutex = end_mutex - start_mutex;

        // CAS
        auto start_CAS = chrono::high_resolution_clock::now();

        Result res_CAS = arr.findCAS(numThreads);

        auto end_CAS = chrono::high_resolution_clock::now();

        chrono::duration<double, milli> duration_CAS = end_CAS - start_CAS;

        printResults("Sequence", res_seq, duration_seq);
        printResults("Mutex", res_mutex, duration_mutex);
        printResults("CAS", res_CAS, duration_CAS);

        csvFile << size << ","
                << numThreads << ","
                << duration_seq.count() << ","
                << duration_mutex.count() << ","
                << duration_CAS.count() << "\n";
    }

    csvFile.close();

    return 0;
}