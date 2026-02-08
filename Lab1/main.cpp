#include <iostream>
#include <chrono>
#include "matrix.cpp"
#include <thread>

using namespace std;

void start_th (int n_th, vector<thread> &threads, Matrix &matrix)
{
    for (int i = 0; i < n_th; i++)
    {
        threads.emplace_back(&Matrix::change_matrix, &matrix, n_th, i);
    }

    for (int i = 0; i < n_th; i++)
    {
        threads[i].join();
    }
}



int main() {
    int size = 50000;
    int n_th = 8;
    Matrix matrix1(size);

    matrix1.change_matrix(1, 0);
    matrix1.change_matrix(1, 0);

    //matrix1.print_matrix();

    auto start = chrono::high_resolution_clock::now();
    matrix1.change_matrix(1, 0);
    auto end = chrono::high_resolution_clock::now();

    chrono::duration<double, milli> duration = end - start;

    //matrix1.print_matrix();

    matrix1.change_matrix(1, 0);

    volatile int rand_el = matrix1.return_rand();
    cout << rand_el << endl;

    cout << "Time: " << duration.count() << " ms" << endl;

    vector<thread> threads;

    //matrix1.print_matrix();

    auto start1 = chrono::high_resolution_clock::now();

    start_th(n_th, threads, matrix1);
    threads.clear();
    start_th(n_th, threads, matrix1);

    auto end1 = chrono::high_resolution_clock::now();

    chrono::duration<double, milli> duration1 = end1 - start1;

    //matrix1.print_matrix();

    volatile int rand_el1 = matrix1.return_rand();
    cout << rand_el1 << endl;

    cout << "Time with threads: " << duration1.count() << " ms" << endl;

    return 0;
}