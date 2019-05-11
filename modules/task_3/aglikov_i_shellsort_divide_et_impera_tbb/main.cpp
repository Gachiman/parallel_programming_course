//  Copyright 2019 Aglikov Ilya

#define _SCL_SECURE_NO_WARNINGS
#include <tbb/tbb.h>
#include <iostream>
#include <algorithm>
#include <utility>

void shellsort(int* arr, int size);
void shellsortPar(int* arr, int size, int procNum);
void printArr(int* arr, int size);
void merge(int* arr, int n, int m);

int main(int argc, char* argv[]) {
    int size = ((argc >= 2) && (atoi(argv[1]) > 0)) ? atoi(argv[1]) : 10;
    int procNum = ((argc >= 3) && (atoi(argv[2]) > 0)) ?
        atoi(argv[2]) : tbb::task_scheduler_init::default_num_threads();
    tbb::task_scheduler_init init(procNum);

    int* arrSingle = new int[size];
    int* arrPar = new int[size];
    for (int i = 0; i < size; i++)
        arrSingle[i] = arrPar[i] = size - i;
    if (size <= 30) {
        std::cout << "Original array: ";
        printArr(arrSingle, size);
    }

    tbb::tick_count shellStart = tbb::tick_count::now();
    shellsort(arrSingle, size);
    tbb::tick_count shellEnd = tbb::tick_count::now();

    tbb::tick_count parStart = tbb::tick_count::now();
    shellsortPar(arrPar, size, procNum);
    tbb::tick_count parEnd = tbb::tick_count::now();

    if (size <= 30) {
        std::cout << "Shell sort array: ";
        printArr(arrPar, size);
        std::cout << std::endl;
    }

    std::cout << std::fixed << "Single time = " << (shellEnd - shellStart).seconds()
        << "\nParallel time = " << (parEnd - parStart).seconds()
        << "\nAcceleration = "
        << ((shellEnd - shellStart).seconds() / (parEnd - parStart).seconds())
        << std::endl;

    if (std::equal(arrSingle, arrSingle + size, arrPar))
        std::cout << "Sort working right.\n";
    else
        std::cout << "Sort working wrong.\n";

    delete[] arrSingle;
    delete[] arrPar;
    return 0;
}

void shellsort(int* arr, int size) {
    int step = size / 2;
    while (step != 0) {
        for (int i = step; i < size; i++) {
            for (int j = i; j >= step; j -= step)
                if (arr[j] < arr[j - step])
                    std::swap(arr[j], arr[j - step]);
                else
                    break;
        }
        step /= 2;
    }
}

void shellsortPar(int* arr, int size, int procNum) {
    if ((procNum == 1) || (size < procNum * 2)) {
        shellsort(arr, size);
    } else {
        tbb::task_group tg;
        tg.run([&arr, size, procNum] {
            shellsortPar(arr, size / 2, procNum / 2);
        });
        tg.run([&arr, size, procNum] {
            shellsortPar(arr + size / 2, size - size / 2, procNum - procNum / 2);
        });
        tg.wait();
        merge(arr, size / 2, size - size / 2);
    }
}

void merge(int* arr, int n, int m) {
    int i = 0, j = 0, k = 0;
    int* leftArr = new int[n];
    int* rightArr = new int[m];
    std::copy(arr, arr + n, leftArr);
    std::copy(arr + n, arr + n + m, rightArr);

    while (i < n && j < m)
        if (leftArr[i] < rightArr[j])
            arr[k++] = leftArr[i++];
        else
            arr[k++] = rightArr[j++];
    while (i < n)
        arr[k++] = leftArr[i++];
    while (j < m)
        arr[k++] = rightArr[j++];

    delete[] leftArr;
    delete[] rightArr;
}

void printArr(int* arr, int size) {
    for (int i = 0; i < size; i++)
        std::cout << arr[i] << " ";
    std::cout << std::endl;
}
