// Copyright 2019 Bederdinov Daniil
#define klength 50000000
#include <ctime>
#include <iomanip>
#include <iostream>
#include "tbb/blocked_range.h"
#include "tbb/parallel_for.h"
#include "tbb/task.h"
#include "tbb/task_scheduler_init.h"
#include "tbb/tbb.h"

void shuffle(int* array, int size) {
    srand((unsigned int)time(NULL));
    int i = size, j, temp;
    while (--i > 0) {
        j = std::rand() % size;
        temp = array[j];
        array[j] = array[i];
        array[i] = temp;
    }
}

void fillArray(int* array, int len) {
    for (int i = 0; i < len; i++) {
        array[i] = i;
    }
}

void quickSort(int* array, int size) {
    int i = 0, j = size - 1;
    int pivot = array[size / 2];

    do {
        while (array[i] < pivot)
            i++;
        while (array[j] > pivot)
            j--;

        if (i <= j) {
            int tmp = array[i];
            array[i] = array[j];
            array[j] = tmp;

            i++;
            j--;
        }
    } while (i <= j);

    if (j > 0)
        quickSort(array, j + 1);
    if (i < size)
        quickSort(&array[i], size - i);
}

void printArray(int* array, const int size) {
    for (int i = 0; i < size; i++) {
        std::cout << array[i] << " ";
    }
    std::cout << std::endl;
}

bool check(int* A, int* B, int size) {
    for (int i = 0; i < size; ++i)
        if (A[i] != B[i]) {
            return false;
        }
    return true;
}

int compare(const int* i, const int* j) {
    return *i - *j;
}

class EvenSelector : public tbb::task {
 private:
    int* array;
    int size1, size2;

 public:
    EvenSelector(int* _arr, int _size1, int _size2) : array(_arr), size1(_size1), size2(_size2) {
    }

    task* execute() {
        // int* array2 = array + size1;

        int numOfEven = (size1 + size2 + 1) / 2;  // Число четных элементов
        int* evenElements = new int[numOfEven];  // Массив для хранения четных элементов

        int firstArrayItterator = 0, secondArrayItterator = 0, i = 0;
        while (firstArrayItterator < size1 && secondArrayItterator < size2) {
            // if (array[firstArrayItterator] <= array2[secondArrayItterator]) {
            if (array[firstArrayItterator] <= array[secondArrayItterator + size1]) {
                evenElements[i] = array[firstArrayItterator];
                firstArrayItterator += 2;
            } else {
                // evenElements[i] = array2[secondArrayItterator];
                evenElements[i] = array[secondArrayItterator + size1];
                secondArrayItterator += 2;
            }
            i++;
        }

        if (firstArrayItterator >= size1) {
            for (int j = secondArrayItterator; j < size2; j += 2, i++)
                // evenElements[i] = array2[j];
                evenElements[i] = array[j + size1];
        } else {
            for (int j = firstArrayItterator; j < size1; j += 2, i++)
                evenElements[i] = array[j];
        }

        // Копирование из временного массива в основной
        for (int j = 0; j < numOfEven; j++)
            array[j * 2] = evenElements[j];
        return NULL;
    }
};

class OddSelector : public tbb::task {
 private:
    int* array;
    int size1, size2;

 public:
    OddSelector(int* _arr, int _size1, int _size2) : array(_arr), size1(_size1), size2(_size2) {
    }

    task* execute() {
        // int* array2 = array + size1;

        // int numOfOdd = (size1 + size2) - (size1 + size2 + 1) / 2;  // Число нечетных элементов
        int numOfOdd = (size1 + size2) / 2;  // Число нечетных элементов
        int* oddElements = new int[numOfOdd];  // Массив для хранения нечетных элементов

        int firstArrayItterator = 1, secondArrayItterator = 1, i = 0;
        while (firstArrayItterator < size1 && secondArrayItterator < size2) {
            // if (array[firstArrayItterator] <= array2[secondArrayItterator]) {
            if (array[firstArrayItterator] <= array[secondArrayItterator + size1]) {
                oddElements[i] = array[firstArrayItterator];
                firstArrayItterator += 2;
            } else {
                // oddElements[i] = array2[secondArrayItterator];
                oddElements[i] = array[secondArrayItterator + size1];
                secondArrayItterator += 2;
            }
            i++;
        }

        if (firstArrayItterator >= size1) {
            for (int j = secondArrayItterator; j < size2; j += 2, i++)
                // oddElements[i] = array2[j];
                oddElements[i] = array[j + size1];
        } else {
            for (int j = firstArrayItterator; j < size1; j += 2, i++)
                oddElements[i] = array[j];
        }

        // Копирование из временного массива в основной
        for (int j = 0; j < numOfOdd; j++)
            array[j * 2 + 1] = oddElements[j];
        return NULL;
    }
};

class Comparator {
 private:
    int* array;

 public:
    explicit Comparator(int* _arr) : array(_arr) {}

    void operator()(const tbb::blocked_range<int>& r) const {
        for (int i = r.begin(); i < r.end(); i++)
            if (array[i - 1] > array[i]) {
                int tmp = array[i - 1];
                array[i - 1] = array[i];
                array[i] = tmp;
            }
    }
};

class Sorter : public tbb::task {
 private:
    int* array;
    int size;
    int portion;

 public:
    Sorter(int* _arr, int _size, int _portion) : array(_arr), size(_size), portion(_portion) {}

    task* execute() {
        if (size <= portion) {
            quickSort(array, size);
        } else {
            int halfSize = size / 2 + (size / 2) % 2;
            Sorter& sorter1 = *new (allocate_child()) Sorter(array, halfSize, portion);
            Sorter& sorter2 = *new (allocate_child()) Sorter(array + halfSize, size - halfSize, portion);
            set_ref_count(3);
            spawn(sorter1);
            spawn_and_wait_for_all(sorter2);
            EvenSelector& evenSelector = *new (allocate_child()) EvenSelector(array, halfSize, size - halfSize);
            OddSelector& oddSelector = *new (allocate_child()) OddSelector(array, halfSize, size - halfSize);
            set_ref_count(3);
            spawn(evenSelector);
            spawn_and_wait_for_all(oddSelector);

            tbb::parallel_for(tbb::blocked_range<int>(1, size), Comparator(array));
        }
        return NULL;
    }
};

void TBB_QS(int* array, int size, int threads) {
    int portion = size / threads;
    Sorter& sorter = *new (tbb::task::allocate_root())Sorter(array, size, portion);
    tbb::task::spawn_root_and_wait(sorter);
}

int main(int argc, char* argv[]) {
    int threads = 4;
    int size = klength;
    if (argc == 2) {
        threads = atoi(argv[1]);
    } else if (argc == 3) {
        threads = atoi(argv[1]);
        size = atoi(argv[2]);
    }

    int* array = new int[size];
    int* copy1 = new int[size];
    int* copy2 = new int[size];
    fillArray(array, size);
    shuffle(array, size);

    for (int i = 0; i < size; i++) {
        copy1[i] = array[i];
        copy2[i] = array[i];
    }

    if (size <= 100) {
        std::cout << "Unsorted array:" << std::endl;
        printArray(array, size);
        std::cout << std::endl;
    }

    tbb::tick_count startTime = tbb::tick_count::now(), endTime;

    TBB_QS(array, size, threads);
    endTime = tbb::tick_count::now();
    auto parallelTime = (endTime - startTime).seconds();

    if (size <= 100) {
        printArray(array, size);
    }

    qsort(copy1, size, sizeof(int), (int (*)(const void*, const void*))compare);

    std::cout << "TBB time: " << parallelTime << std::endl;

    startTime = tbb::tick_count::now();
    quickSort(copy2, size);
    endTime = tbb::tick_count::now();
    auto serialTime = (endTime - startTime).seconds();

    std::cout << "Serial time time: " << std::fixed << std::setprecision(6) << serialTime << std::endl;

    std::cout << "Boost: " << serialTime / parallelTime << std::endl;

    if (check(copy1, array, size))
        std::cout << "Arrays are equal" << std::endl;
    else
        std::cout << "Arrays are NOT equal" << std::endl;

    delete[] copy1;
    delete[] copy2;
    delete[] array;

    return 0;
}
