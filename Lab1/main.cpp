#include <iostream>
#include <random>
#include <vector>
#include <chrono>

using namespace std;

void print_matrix (vector<vector<int>>& matrix)
{
    for (int i = 0; i < matrix.size(); i++)
    {
        for (int j = 0; j < matrix.size(); j++)
        {
            cout << matrix[i][j] << " ";
        }
        cout << endl;
    }
}

void init_matrix (vector<vector<int>>& matrix)
{
    srand(time(0));
    for (int i = 0; i < matrix.size(); i++)
    {
        for (int j = 0; j < matrix.size(); j++)
        {
            matrix[i][j] = rand() % 100;
        }
    }
}

void change_matrix (vector<vector<int>>& matrix)
{
    for (int i = 0; i < matrix.size(); i++)
    {
        for (int j = i + 1; j < matrix.size(); j ++)
        {
            swap(matrix[i][j], matrix[j][i]);
        }
    }
}

int main() {
    int size = 10;
    vector<vector<int>> matrix(size, vector<int>(size));

    init_matrix(matrix);

    change_matrix(matrix);
    change_matrix(matrix);

    //print_matrix(matrix);
    cout << endl;

    auto start = chrono::high_resolution_clock::now();
    change_matrix(matrix);
    auto end = chrono::high_resolution_clock::now();

    chrono::duration<double, milli> duration = end - start;

    //print_matrix(matrix);

    volatile int rand_el;
    rand_el = matrix[rand() % size][rand() % size];
    cout << rand_el << endl;

    cout << "Time: " << duration.count() << " ms" << endl;

    return 0;
}