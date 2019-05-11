//  Copyright 2019 Aglikov Ilya

#define _SCL_SECURE_NO_WARNINGS
#include <omp.h>
#include <iostream>
#include <algorithm>
#include <utility>
#include <ctime>


void shellsort(int* arr, int size);
void shellsortPar(int* arr, int size, int procNum);
void printArr(int* arr, int size);
bool checkEquality(int* arr0, int* arr1, int size);
void merge(int* arr, int n, int m);

int main(int argc, char* argv[]) {
    int size = ((argc >= 2) && (atoi(argv[1]) > 0)) ? atoi(argv[1]) : 10;
    int procNum = ((argc >= 3) && (atoi(argv[2]) > 0)) ?
        atoi(argv[2]) : omp_get_max_threads();
    omp_set_num_threads(procNum);

    int* arrSingle = new int[size];
    int* arrPar = new int[size];
    std::srand(static_cast<unsigned int>(time(0)));
    for (int i = 0; i < size; i++)
        arrSingle[i] = arrPar[i] = std::rand() % 100;
    if (size <= 30) {
        std::cout << "Original array: ";
        printArr(arrSingle, size);
    }

    double shellTime = omp_get_wtime();
    shellsort(arrSingle, size);
    shellTime = omp_get_wtime() - shellTime;

    double parTime = omp_get_wtime();
#pragma omp parallel shared(arrPar)
    {
#pragma omp single nowait
        shellsortPar(arrPar, size, procNum);
    }
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
#pragma omp task firstprivate(procNum) shared(arr)
        shellsortPar(arr, size / 2, procNum / 2);
#pragma omp task firstprivate(procNum) shared(arr)
        shellsortPar(arr + size / 2, size - size / 2, procNum - procNum / 2);
#pragma omp taskwait
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

bool checkEquality(int* arr0, int* arr1, int size) {
    bool flag = true;
    for (int i = 0; i < size; i++)
        if (arr0[i] != arr1[i]) {
            flag = false;
            break;
        }
    return flag;
}
