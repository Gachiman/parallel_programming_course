// Copyright 2019 Tolstikov Maksim
// RadixSort with simple merge (double)
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <omp.h>
#include <iostream>
#include <ctime>
#include <algorithm>
#include <cstring>
#include <climits>
#include <vector>

void CountingSort(double* inp, double* out, int byteNum, int size) {
    unsigned char* mas = (unsigned char*)inp;
    int counter[256];
    int tem;
    memset(counter, 0, sizeof(int) * 256);
    for (int i = 0; i < size; i++)
        counter[mas[8 * i + byteNum]]++;
    int j = 0;
    for (; j < 256; j++) {
        if (counter[j] != 0)
            break;
    }
    tem = counter[j];
    counter[j] = 0;
    j++;
    for (; j < 256; j++) {
        int b = counter[j];
        counter[j] = tem;
        tem += b;
    }
    for (int i = 0; i < size; i++) {
        out[counter[mas[8 * i + byteNum]]] = inp[i];
        counter[mas[8 * i + byteNum]]++;
    }
}

void LastCountingSort(double* inp, double* out, int byteNum, int size) {
    unsigned char* mas = (unsigned char*)inp;
    int counter[256];
    int tem;
    memset(counter, 0, sizeof(int) * 256);
    for (int i = 0; i < size; i++)
        counter[mas[8 * i + byteNum] + 128]++;
    int j = 0;
    for (; j < 256; j++) {
        if (counter[j] != 0)
            break;
    }
    tem = counter[j];
    counter[j] = 0;
    j++;
    for (; j < 256; j++) {
        int b = counter[j];
        counter[j] = tem;
        tem += b;
    }
    for (int i = 0; i < size; i++) {
        out[counter[mas[8 * i + byteNum] + 128]] = inp[i];
        counter[mas[8 * i + byteNum] + 128]++;
    }
}

void merge(double* mas, int sizel, int sizer) {
    int size = sizel + sizer;
    double* tempMas = new double[size];
    int i = 0, j = sizel, k = 0;

    while (i != sizel && j != size) {
        if (mas[i] <= mas[j]) {
            tempMas[k] = mas[i];
            ++i;
            ++k;
        } else {
            tempMas[k] = mas[j];
            ++j;
            ++k;
        }
    }

    if (i < sizel) {
        for (; i < sizel; ++i) {
            tempMas[k] = mas[i];
            ++k;
        }
    }

    if (j < size) {
        for (; j < size; ++j) {
            tempMas[k] = mas[j];
            ++k;
        }
    }

    for (i = 0; i < size; ++i) {
        mas[i] = tempMas[i];
    }

    delete[] tempMas;
}

std::vector<int> merge_size(std::vector<int> counts, int num_th) {
    std::vector<int> tmp;
    if (num_th % 2 == 1) {
        for (int i = 0; i < num_th / 2; ++i) {
            tmp.push_back(counts[2 * i] + counts[2 * i + 1]);
        }
        tmp.push_back(counts[counts.size() - 1]);
    } else {
        for (int i = 0; i < num_th / 2; ++i) {
            tmp.push_back(counts[2 * i] + counts[2 * i + 1]);
        }
    }
    return tmp;
}

int  displacement_M(std::vector<int> counts, int num_th) {
    int sum = 0;
    for (int i = 0; i < num_th; ++i) {
        sum += counts[2 * i] + counts[2 * i + 1];
    }
    return sum;
}

int  displacement_S(std::vector<int> counts, int num_th) {
    int sum = 0;
    for (int i = 0; i < num_th; ++i) {
        sum += counts[i];
    }
    return sum;
}

void PrintArray(double* array, int size) {
    if (size < 20) {
        for (int i = 0; i < size; i++) {
            std::cout << array[i] << " ";
        }
        std::cout << std::endl;
    }
}

void LSDSortDouble(double* inp, int size) {
    double* out = new double[size];

    for (int i = 0; i < 6; i += 2) {
        CountingSort(inp, out, i, size);
        CountingSort(out, inp, i + 1, size);
    }
    CountingSort(inp, out, 6, size);
    LastCountingSort(out, inp, 7, size);
    delete[] out;
}

void CopyArray(double* mas, double* tmp, int size) {
    for (int i = 0; i < size; i++)
        tmp[i] = mas[i];
}

void CheckingSort(double* mas, double* tmp, int size) {
    for (int i = 0; i < size; i++) {
        if (mas[i] != tmp[i]) {
            std::cout << "Sort is incorrectly" << std::endl;
            break;
        }
    }
    std::cout << "Sort is correctly" << std::endl;
}

void GenerateArray(double* mas, int size) {
    int b = 100;
    int a = 0;
    for (int i = 0; i < size; i++) {
        mas[i] = static_cast<double>(std::rand()) * (b - a + 1) / RAND_MAX + a;
    }
}

int main(int argc, char* argv[]) {
    double time_lsd = 0;
    double ptime_lsd = 0;
    std::vector<int> counts;
    int size = 10;
    int n = 2;
    std::srand((unsigned)time(NULL));
    double* mas, * tmp, * lmas;
    if (argc == 4) {
        n = atoi(argv[1]);
        if (strcmp(argv[2], "-size") == 0)
            size = atoi(argv[3]);
    }
    mas = new double[size];
    tmp = new double[size];
    lmas = new double[size];
    int tail = size % n;
    for (int i = 0; i < n; ++i) {
        if (i == 0) {
            counts.push_back(size / n + tail);
        } else {
            counts.push_back(size / n);
        }
    }
    if (size < 20) {
        for (int i = 0; i < n; ++i) {
            std::cout << counts[i] << "  ";
        }
        std::cout << std::endl;
    }
    if (size < 20)
        std::cout << "Array: ";
    GenerateArray(mas, size);
    CopyArray(mas, tmp, size);
    std::sort(tmp, tmp + size);
    CopyArray(mas, lmas, size);
    if (mas == NULL) {
        std::cout << "Error! Incorrect input data for array";
        return -1;
    }
    PrintArray(mas, size);
    ptime_lsd = omp_get_wtime();
    std::cout << "Num threads: " << n << std::endl;
    omp_set_num_threads(n);
#pragma omp parallel
    {
        LSDSortDouble(mas + displacement_S(counts, omp_get_thread_num()), counts[omp_get_thread_num()]);
#pragma omp barrier
    }
    int f = n;
    int k = n / 2 + n % 2;
    while (k > 0) {
        omp_set_num_threads(k);
#pragma omp parallel
        {
            if (f % 2 == 1) {
                if (omp_get_thread_num() != k - 1) {
                    merge(mas + displacement_M(counts, omp_get_thread_num()),
                        counts[2 * omp_get_thread_num()],
                        counts[2 * omp_get_thread_num() + 1]);
                }
            } else {
                merge(mas + displacement_M(counts, omp_get_thread_num()),
                    counts[2 * omp_get_thread_num()],
                    counts[2 * omp_get_thread_num() + 1]);
            }
#pragma omp barrier
            if (omp_get_thread_num() == 0) {
                counts = merge_size(counts, f);
                if (k == 1) {
                    k = 0;
                } else {
                    k = k / 2 + k % 2;
                }
                f = f / 2 + f % 2;
            }
        }
    }

    ptime_lsd = omp_get_wtime() - ptime_lsd;
    time_lsd = omp_get_wtime();
    LSDSortDouble(lmas, size);
    time_lsd = omp_get_wtime() - time_lsd;
    if (size < 20)
        std::cout << "Array after LSD sort: ";
    PrintArray(lmas, size);
    std::cout << "LSD ";
    CheckingSort(lmas, tmp, size);
    if (size < 20)
        std::cout << "Array after LSD sort with simle merge: ";
    PrintArray(mas, size);
    std::cout << "LSD with simple merge ";
    CheckingSort(mas, tmp, size);
    std::cout << "Execution time LSD sort: " << time_lsd << std::endl;
    std::cout << "Execution time LSD sort with simple merge: " << ptime_lsd << std::endl;
    std::cout << "Boost is : " << time_lsd / ptime_lsd << std::endl;
    delete[] mas;
    delete[] tmp;
    delete[] lmas;
    return 0;
}
