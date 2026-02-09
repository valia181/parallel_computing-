#include <iostream>
#include <chrono>
#include "matrix.cpp"
#include <thread>
#include <fstream>

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
    int size = 5000;
    int n_th = 8;
    Matrix matrix1(size);

//    matrix1.change_matrix(1, 0);
//    matrix1.change_matrix(1, 0);

    //matrix1.print_matrix();

//    auto start = chrono::high_resolution_clock::now();

//    matrix1.change_matrix(1, 0);

//    auto end = chrono::high_resolution_clock::now();
//
//    chrono::duration<double, milli> duration = end - start;

    //matrix1.print_matrix();
//
//    volatile int rand_el = matrix1.return_rand();
//    cout << rand_el << endl;
//
//    cout << "Time: " << duration.count() << " ms" << endl;

    vector<thread> threads;
    vector<int> sizes = {100, 1000, 5000, 10000, 20000};
    vector<int> threads_count = {2, 4, 8, 16, 32, 64};

    ofstream results_file("results.csv");

    for (int size: sizes)
    {
        Matrix matrix2(size);

        for (int thread_num: threads_count)
        {
            threads.clear();

            start_th(thread_num, threads, matrix2);
            threads.clear();
            start_th(thread_num, threads, matrix2);
            threads.clear();

            auto start = chrono::high_resolution_clock::now();

            start_th(thread_num, threads, matrix2);

            auto end = chrono::high_resolution_clock::now();

            threads.clear();

            chrono::duration<double, milli> duration = end - start;

            results_file << size << "," << thread_num << "," << duration.count() << endl;

            volatile int rand_el = matrix2.return_rand();

            cout << "Thread_num: " << thread_num << ", " << duration.count() << " ms, " << "rand_el: " << rand_el << endl;
        }
    }

    results_file.close();

    return 0;
}