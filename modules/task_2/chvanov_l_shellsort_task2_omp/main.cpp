//  Copyright 2019 Chvanov Leonid

#include <omp.h>
#include <ctime>
#include <iostream>
#include <cstdlib>
#include <algorithm>
#include <utility>
#include <functional>

void insertSort(int arr[], int size, int step) {
    for (int j = step; j < size; j += step) {
        int key = arr[j];
        int i = j - step;
        while (i >= 0 && arr[i] > key) {
            arr[i + step] = arr[i];
            i -= step;
        }
        arr[i + step] = key;
    }
}

void shellsort(int arr[], int size) {
    int i, step;

    for (step = size / 2; step > 0; step /= 2) {
#pragma omp parallel for shared(arr, step, size) private(i) default(none)
        for (i = 0; i < step; i++)
            insertSort(arr + i, size - i, step);
    }
}

void printArray(int* arr, int size) {
    for (int i = 0; i < size; i++)
        std::cout << arr[i] << " ";
    std::cout << std::endl;
}

bool compareArrays(int* arr0, int* arr1, int size) {
    for (int i = 0; i < size; i++)
        if (arr0[i] != arr1[i])
            return false;
    return true;
}

void copyArray(int* src, int* dest, int size) {
    for (int i = 0; i < size; i++)
        dest[i] = src[i];
}

int main(int argc, char **argv) {
    const int size = 1000;
    const int threadNum = 12;
    int *arrSTL, *arrLin, *arrPar;
    double startTime, stlTime, linearTime, parallelTime;

    arrSTL = new int[size];
    arrLin = new int[size];
    arrPar = new int[size];

    for (int i = 0; i < size; i++)
        arrSTL[i] = i;
    std::random_shuffle(arrSTL, arrSTL + size);

    copyArray(arrSTL, arrLin, size);
    copyArray(arrSTL, arrPar, size);

    if (size < 100) {
        std::cout << "Initial array: ";
        printArray(arrSTL, size);
    }

    startTime = omp_get_wtime();
    std::sort(arrSTL, arrSTL + size);
    stlTime = omp_get_wtime() - startTime;

    omp_set_num_threads(1);
    startTime = omp_get_wtime();
    shellsort(arrLin, size);
    linearTime = omp_get_wtime() - startTime;

    omp_set_num_threads(threadNum);
    startTime = omp_get_wtime();
    shellsort(arrPar, size);
    parallelTime = omp_get_wtime() - startTime;


    if (size < 100) {
        std::cout << "Sorted array: ";
        printArray(arrPar, size);
    }

    if (compareArrays(arrSTL, arrPar, size))
        std::cout << "Parallel shell sort is working" << std::endl;
    else
        std::cout << "Parallel shell sort isn't working!" << std::endl;
    std::cout << "STL time:      " << stlTime << std::endl
        << "Linear time:   " << linearTime << std::endl
        << "Parallel time: " << parallelTime << std::endl
        << "Parallel acceleration = "
        <<  linearTime / parallelTime << std::endl;
    return 0;
}
