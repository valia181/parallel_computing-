#include <iostream>
#include <chrono>
#include "matrix.cpp"

using namespace std;

int main() {
    int size = 5;
    Matrix matrix1(size);

    matrix1.print_matrix();

    auto start = chrono::high_resolution_clock::now();

    auto end = chrono::high_resolution_clock::now();

    chrono::duration<double, milli> duration = end - start;

    matrix1.print_matrix();

    volatile int rand_el = matrix1.return_rand();
    cout << rand_el << endl;

    cout << "Time: " << duration.count() << " ms" << endl;

    return 0;
}