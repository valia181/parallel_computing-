#include <iostream>
#include <random>
#include <vector>

using namespace std;

class Matrix {
private:
    vector<vector<int>> matrix;
    int size;

public:
    void init_matrix ()
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

    Matrix(int size): size (size){
        matrix.resize(size, vector<int>(size));
        init_matrix();
    }

    void print_matrix ()
    {
        for (int i = 0; i < matrix.size(); i++)
        {
            for (int j = 0; j < matrix.size(); j++)
            {
                cout << matrix[i][j] << " ";
            }
            cout << endl;
        }
        cout << endl;
    }

    void change_matrix ()
    {
        for (int i = 0; i < matrix.size(); i++)
        {
            for (int j = i + 1; j < matrix.size(); j ++)
            {
                swap(matrix[i][j], matrix[j][i]);
            }
        }
    }

    int volatile return_rand ()
    {
        int rand_el = matrix[rand() % size][rand() % size];
        return rand_el;
    }
};