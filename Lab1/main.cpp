#include <iostream>
#include <random>
#include <vector>

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
    int size = 5;
    vector<vector<int>> matrix(size, vector<int>(size));
    init_matrix(matrix);
    print_matrix(matrix);
    change_matrix(matrix);
    cout << endl;
    print_matrix(matrix);
    return 0;
}