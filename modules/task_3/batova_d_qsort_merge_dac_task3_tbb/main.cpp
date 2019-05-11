// Copyright 2019 Batova Darya
#define _SCL_SECURE_NO_WARNINGS
#include <omp.h>
#include <tbb/tbb.h>
#include <iostream>
#include <random>
#include <ctime>
#include <algorithm>

template<typename T>
void FillingArray(T* array, int n, int min, int max) {
    std::random_device random_device;
    std::mt19937 generator(random_device());
    std::uniform_int_distribution<> distribution(min, max);
    for (int i = 0; i < n; i++) {
        array[i] = distribution(generator);
    }
}

void quickSort(int arr[], int left, int right) {
    int i = left, j = right;
    int tmp;
    int pivot = arr[(left + right) / 2];

    /* partition */
    while (i <= j) {
        while (arr[i] < pivot)
            i++;
        while (arr[j] > pivot)
            j--;
        if (i <= j) {
            tmp = arr[i];
            arr[i] = arr[j];
            arr[j] = tmp;
            i++;
            j--;
        }
    }

    /* recursion */
    if (left < j)
        quickSort(arr, left, j);
    if (i < right)
        quickSort(arr, i, right);
}

void ShowArray(int* arr, int n) {
    if (n < 20) {
        for (int i = 0; i < n; i++) {
            std::cout << arr[i] << " ";
        }
        std::cout << std::endl;
    }
}


void Merge_Two_Arrays_Into_Tmp(int* arr1, int* arr2, int* tmp, int size1, int size2) {
    int index1 = 0;
    int index2 = 0;
    int i = 0;

    while ((index1 != size1) && (index2 != size2)) {
        if (arr1[index1] <= arr2[index2]) {
            tmp[i] = arr1[index1];
            index1++;
        } else {
            tmp[i] = arr2[index2];
            index2++;
        }
        i++;
    }

    if (index1 == size1) {
        int j = index2;
        for (; j < size2; j++, i++) {
            tmp[i] = arr2[j];
        }
    } else {
        int j = index1;
        for (; j < size1; j++, i++) {
            tmp[i] = arr1[j];
        }
    }
}

int BinSearch(int* arr, int left, int right, int x) {
    if (left == right) {
        return left;
    }
    if (left + 1 == right) {
        if (x < arr[1]) {
            return 1;
        } else {
            return right;
        }
    }
    int middle = (left + right) / 2;
    if (x < arr[middle]) {
        right = middle;
    } else {
        if (x > arr[middle]) {
            left = middle;
        } else {
            return middle;
        }
    }
    return BinSearch(arr, left, right, x);
}

void Merge_Two_Arrays_Into_Res(int* arr1, int* arr2, int size1, int size2, int* res) {
    int x = arr1[size1 / 2];
    int x1 = BinSearch(arr2, 0, size2, x);
    int nSize1 = size1 / 2 + x1;
    int nSize2 = size1 + size2 - nSize1;
    int* tmp1 = new int[nSize1];
    int* tmp2 = new int[nSize2];
    Merge_Two_Arrays_Into_Tmp(arr1, arr2, tmp1, size1 / 2, x1);
    Merge_Two_Arrays_Into_Tmp(arr1 + size1 / 2, arr2 + x1, tmp2, size1 - size1 / 2, size2 - x1);
    std::copy(tmp1, tmp1 + nSize1, res);
    std::copy(tmp2, tmp2 + nSize2, res + nSize1);
    delete[] tmp1;
    delete[] tmp2;
}


class QuickSortMergeTask : public tbb::task {
 private:
    int* array;
    int* res;
    int size;
    int pNum;

 public:
    QuickSortMergeTask(int* _array, int* _res, int _size, int _pNum) :
        array(_array), res(_res), size(_size), pNum(_pNum) { }
    tbb::task* execute() {
        if (pNum == 1) {
            quickSort(array, 0, size - 1);
        } else if (pNum > 1) {
            QuickSortMergeTask& a = *new(allocate_child())
                QuickSortMergeTask(array, res, size / 2, pNum / 2);
            QuickSortMergeTask& b = *new(allocate_child())
                QuickSortMergeTask(array + size / 2, res + size / 2, size - size / 2, pNum - pNum / 2);
            set_ref_count(3);
            spawn(b);
            spawn_and_wait_for_all(a);
            Merge_Two_Arrays_Into_Res(array, array + size / 2, size / 2, size - size / 2, res);
            std::copy(res, res + size, array);
        }
        return nullptr;
    }
};

void ParallelQuickSort(int* array, int* res, int size, int pNum) {
    QuickSortMergeTask& a = *new(tbb::task::allocate_root())
        QuickSortMergeTask(array, res, size, pNum);
    tbb::task::spawn_root_and_wait(a);
}



int main(int argc, char** argv) {
    int* array;
    int* copy;
    int size;
    int pNum;
    tbb::tick_count tLin1, tLin2, tPar1, tPar2;
    double speed;
    if (argc == 3) {
        size = atoi(argv[1]);
        pNum = atoi(argv[2]);
    } else {
        size = 10000000;
        pNum = 4;
    }
    tbb::task_scheduler_init init(pNum);
    // omp_set_num_threads(pNum);
    array = new int[size];
    copy = new int[size];
    FillingArray(array, size, 0, 300000000);
    // std::cout << "Initially the array looks like this: " << std::endl;
    ShowArray(array, size);
    std::copy(array, array + size, copy);
    // quickSort(copy, 0, size-1);
    tLin1 = tbb::tick_count::now();
    // std::sort(copy, copy + size);
    quickSort(copy, 0, size - 1);
    tLin2 = tbb::tick_count::now();
    // std::cout << "Sorted array using quick sort: " << std::endl;
    // ShowArray(copy, size);
    int* res = new int[size];
    tPar1 = tbb::tick_count::now();
    ParallelQuickSort(array, res, size, pNum);
    tPar2 = tbb::tick_count::now();
    // std::cout << "Sorted array using divide and conquer: " << std::endl;
    // ShowArray(res, size);
    speed = (tLin2 - tLin1).seconds() / (tPar2 - tPar1).seconds();
    // std::cout << "Sorted array using QuickSort: " << std::endl;
    ShowArray(copy, size);
    std::cout << "Time of work sort (linear version): " << (tLin2 - tLin1).seconds() << std::endl;
    std::cout << "Time of work QuickSortMerge (parallel version): " << (tPar2 - tPar1).seconds() << std::endl;
    std::cout << "Speed: " << speed << std::endl;
    if (std::equal(copy, copy + size, res)) {
        std::cout << "Excellent! Arrays are matching" << std::endl;
    } else {
        std::cout << "Error! Arrays are not matching" << std::endl;
    }
    delete[] array;
    delete[] copy;
    delete[] res;
    return 0;
}
