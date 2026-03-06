#include <iostream>
#include <chrono>
#include "Array.cpp"

using namespace std;

int main() {
    int size = 10;

    Array arr(size);

    auto start_seq = chrono::high_resolution_clock::now();

    Result res_seq = arr.findSeq();

    arr.printData();

    auto end_seq = chrono::high_resolution_clock::now();
    chrono::duration<double, milli> duration_seq = end_seq - start_seq;

    cout << "Sequence algorithm: " << endl;
    cout << "Min: " << res_seq.min << endl;
    cout << "Max: " << res_seq.max << endl;
    cout << "Sum: " << res_seq.sum << endl;
    cout << "Time: " << duration_seq.count() << " ms\n" << endl;

    return 0;
}