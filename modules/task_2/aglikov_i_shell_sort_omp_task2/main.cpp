//  Copyright 2019 Aglikov Ilya

#define _SCL_SECURE_NO_WARNINGS
#include <omp.h>
#include <iostream>
#include <algorithm>
#include <utility>

void shellsort(int* arr, int size);
void shellsortPar(int* arr, int size);
void printArr(int* arr, int size);
bool checkEquality(int* arr0, int* arr1, int size);
void mergeArray(int* arr, int size, int procNum);
void merge(int* arr, int n, int m);

int main(int argc, char* argv[]) {
    int size = ((argc >= 2) && (atoi(argv[1]) > 0)) ? atoi(argv[1]) : 10;
    if ((argc == 3) && (atoi(argv[2])))
        omp_set_num_threads(atoi(argv[2]));

    int* arrSingle = new int[size];
    int* arrPar = new int[size];
    for (int i = 0; i < size; i++)
        arrSingle[i] = arrPar[i] = size - i;
    if (size <= 30) {
        std::cout << "Original array: ";
        printArr(arrSingle, size);
    }

    double shellTime = omp_get_wtime();
    shellsort(arrSingle, size);
    shellTime = omp_get_wtime() - shellTime;

    double parTime = omp_get_wtime();
    shellsortPar(arrPar, size);
    parTime = omp_get_wtime() - parTime;

    if (size <= 30) {
        std::cout << "Shell sort array: ";
        printArr(arrPar, size);
        std::cout << std::endl;
    }

    std::cout << std::fixed << "Single time = " << shellTime
        << "\nParallel time = " << parTime
        << "\nAcceleration = " << (shellTime / parTime) << std::endl;

    if (checkEquality(arrSingle, arrPar, size))
        std::cout << "Sort working right.\n";
    else
        std::cout << "Sort working wrong.\n";
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

void shellsortPar(int* arr, int size) {
    int procNum = omp_get_max_threads();
    if ((procNum == 1) || (size <= procNum * 2)) {
        shellsort(arr, size);
    }  else {
#pragma omp parallel
        {
            int threadNum = omp_get_thread_num();
            if (threadNum != procNum - 1)
                shellsort(arr + (size / procNum) * threadNum, size / procNum);
            else
                shellsort(arr + (size / procNum) * threadNum,
                    size - (size / procNum) * threadNum);
        }
        mergeArray(arr, size, procNum);
    }
}

void mergeArray(int* arr, int size, int procNum) {
    int count = 1, div = procNum;
    while (procNum > 1) {
        if (procNum == 2)
            merge(arr, size / div * count, size - size / div * count);
        else
            merge(arr, size / div * count, size / div);
        count++;
        procNum--;
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
}

void printArr(int* arr, int size) {
    for (int i = 0; i < size; i++)
        std::cout << arr[i] << " ";
    std::cout << std::endl;
}

bool checkEquality(int* arr0, int* arr1, int size) {
    bool flag = true;
    for (int i = 0; i < size; i++)
        if (arr0[i] != arr1[i]) {
            flag = false;
            break;
        }
    return flag;
}
