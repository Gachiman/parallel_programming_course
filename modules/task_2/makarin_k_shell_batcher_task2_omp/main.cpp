#define _SCL_SECURE_NO_WARNINGS
// Copyright 2019 Kirill Makarin
#include <omp.h>
#include <algorithm>
#include <ctime>
#include <iomanip>
#include <iostream>
#include <utility>
#include <random>

void ShellSort(int* array, int size) {
    for (int k = size / 2; k > 0; k /= 2) {
        for (int i = k; i < size; i++) {
            int j;
            int t = array[i];

            for (j = i; j >= k; j -= k) {
                if (t < array[j - k]) {
                    array[j] = array[j - k];
                } else {
                    break;
                }
            }
            array[j] = t;
        }
    }
}

void EvenSplitter(int* array, int* tmp, int size1, int size2) {
    for (int i = 0; i < size1; i += 2)
        tmp[i] = array[i];

    int* array2 = array + size1;

    int a = 0;
    int b = 0;
    int i = 0;

    while ((a < size1) && (b < size2)) {
        if (tmp[a] <= array2[b]) {
            array[i] = tmp[a];
            a += 2;
        } else {
            array[i] = array2[b];
            b += 2;
        }

        i += 2;
    }

    if (a == size1) {
        for (int j = b; j < size2; j += 2, i += 2)
            array[i] = array2[j];
    } else {
        for (int j = a; j < size1; j += 2, i += 2)
            array[i] = tmp[j];
    }
}

void OddSplitter(int* array, int* tmp, int size1, int size2) {
    for (int i = 1; i < size1; i += 2)
        tmp[i] = array[i];

    int* array2 = array + size1;

    int a = 1;
    int b = 1;
    int i = 1;

    while ((a < size1) && (b < size2)) {
        if (tmp[a] <= array2[b]) {
            array[i] = tmp[a];
            a += 2;
        } else {
            array[i] = array2[b];
            b += 2;
        }

        i += 2;
    }

    if (a == size1) {
        for (int j = b; j < size2; j += 2, i += 2)
            array[i] = array2[j];
    } else {
        for (int j = a; j < size1; j += 2, i += 2)
            array[i] = tmp[j];
    }
}

void SimpleComparator(int* array, int size) {
    for (int i = 1; i < (size + 1) / 2; i++)
        if (array[2 * i] < array[2 * i - 1]) {
            int tmp = array[2 * i - 1];
            array[2 * i - 1] = array[2 * i];
            array[2 * i] = tmp;
        }
}

void ParallelSort(int* data, int size, int pnum) {
    if (pnum == 1) {
        ShellSort(data, size);
    } else {
        int* tmp = new int[size];
        int part_size = size / pnum;
#pragma omp parallel shared(data, size, part_size, pnum, tmp) num_threads(pnum)
        {
            int tid = omp_get_thread_num();
            int start = tid * part_size;
            ShellSort(data + start, part_size);
#pragma omp barrier
            int level = pnum;
            int s = part_size;
            while (level != 1) {
                if (tid % 2 == 0 && tid < level) {
                    EvenSplitter(data + tid * s, tmp + tid * s, s, s);
                }
                if (tid % 2 == 1 && tid < level) {
                    int t = tid - 1;
                    OddSplitter(data + t * s, tmp + tid * s, s, s);
                }
#pragma omp barrier
                if (tid % 2 == 0 && tid < level) {
                    SimpleComparator(data + tid * s, s * 2);
                }
#pragma omp barrier
                level /= 2;
                s *= 2;
            }
        }
        delete[] tmp;
    }
}

bool IsCorrectlySorted(int* linear_sorted_array, int* parallel_sorted_array,
                       const int size) {
    for (int i = 0; i < size; i++) {
        if (linear_sorted_array[i] != parallel_sorted_array[i]) {
            return false;
        }
    }

    return true;
}

void PrintArray(int* array, const int size) {
    for (int i = 0; i < size; i++) {
        std::cout << array[i] << " ";
    }
    std::cout << std::endl;
}

int main(int argc, char** argv) {
    int SIZE = 100000;
    int THREADS = omp_get_max_threads();

    if (argc == 2) {
        SIZE = atoi(argv[1]);
    } else if (argc == 3) {
        SIZE = atoi(argv[1]);
        THREADS = atoi(argv[2]);
    }

    int* array = new int[SIZE];
    int* linear_sorted_array = new int[SIZE];
    int* stl_sorted_array = new int[SIZE];

    srand(static_cast<unsigned int>(time(NULL)));
    std::generate(array, array + SIZE, []() { return std::rand() % 100 + 1; });
    std::copy(array, array + SIZE, linear_sorted_array);
    std::copy(array, array + SIZE, stl_sorted_array);
    if (SIZE < 50) {
        std::cout << "Initial array: ";
        PrintArray(array, SIZE);
    }

    double start_time = omp_get_wtime();
    ParallelSort(array, SIZE, THREADS);
    double finish_time = omp_get_wtime();
    double parallel_time = finish_time - start_time;

    start_time = omp_get_wtime();
    ShellSort(linear_sorted_array, SIZE);
    finish_time = omp_get_wtime();
    double linear_time = finish_time - start_time;

    // Sort array using stl sort to check custom algorithms
    std::sort(stl_sorted_array, stl_sorted_array + SIZE);

    if (IsCorrectlySorted(stl_sorted_array, array, SIZE) &&
        IsCorrectlySorted(stl_sorted_array, linear_sorted_array, SIZE)) {
        if (SIZE < 50) {
            std::cout << "Sorted array:  ";
            PrintArray(array, SIZE);
        }
    } else {
        std::cout << "Failed." << std::endl;
    }
    std::cout << std::fixed << std::setprecision(3);
    std::cout << "Parallel time: " << parallel_time << std::endl;
    std::cout << "Linear time:   " << linear_time << std::endl;
    std::cout << "Boost: " << linear_time / parallel_time << std::endl;

    delete[] array;
    delete[] linear_sorted_array;
    delete[] stl_sorted_array;

    return 0;
}
