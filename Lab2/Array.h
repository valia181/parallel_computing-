#ifndef LAB2_ARRAY_H
#define LAB2_ARRAY_H
#include <vector>

using namespace std;

struct Result {
    int min;
    int max;
    int sum;
};

class Array {
private:
    vector<int> data;
    int size;

public:
    Array(int n);

    void printData();

    Result findSeq();

    Result findMutex(int numThreads);

    Result findCAS(int numThreads);
};


#endif //LAB2_ARRAY_H
