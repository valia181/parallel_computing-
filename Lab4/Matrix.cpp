#ifndef MATRIX_H
#define MATRIX_H

#include <iostream>
#include <vector>
#include <random>
#include <ctime>
#include <winsock2.h>

using namespace std;

class Matrix {
private:
    vector<int32_t> data;
    int size;

public:
    Matrix() : size(0) {}

    Matrix(int s) : size(s)
    {
        data.resize(size * size);
        init_matrix();
    }

    void resize(int s)
    {
        size = s;
        data.resize(size * size);
    }

    void init_matrix()
    {
        srand(time(0));
        for (int i = 0; i < size * size; i++)
        {
            data[i] = rand() % 100;
        }
    }

    void print_matrix()
    {
        for (int i = 0; i < size; i++)
        {
            for (int j = 0; j < size; j++)
            {
                cout << data[i * size + j] << "\t";
            }
            cout << endl;
        }
        cout << endl;
    }

    void change_matrix(int n_th, int thread_id)
    {
        for (int i = thread_id; i < size; i += n_th)
        {
            for (int j = i + 1; j < size; j++)
            {
                swap(data[i * size + j], data[j * size + i]);
            }
        }
    }

    char* get_raw_data()
    {
        return (char*)data.data();
    }

    int get_byte_size() const
    {
        return size * size * sizeof(int32_t);
    }

    void to_network()
    {
        for (int i = 0; i < size * size; i++)
        {
            data[i] = htonl(data[i]);
        }
    }

    void to_host()
    {
        for (int i = 0; i < size * size; i++)
        {
            data[i] = ntohl(data[i]);
        }
    }
};

#endif // MATRIX_H