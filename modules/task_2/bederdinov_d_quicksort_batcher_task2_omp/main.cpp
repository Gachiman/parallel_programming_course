// Copyright 2019 Bederdinov Daniil

#define kLength 50000000
#include <omp.h>
#include <cmath>
#include <ctime>
#include <iomanip>
#include <iostream>
#include <vector>

void shuffle(int *array, int size) {
    srand((unsigned int)time(NULL));
    int i = size, j, temp;
    while (--i > 0) {
        j = std::rand() % size;
        temp = array[j];
        array[j] = array[i];
        array[i] = temp;
    }
}

void fillArray(int *array, int len) {
    for (int i = 0; i < len; i++) {
        array[i] = i;
    }
}

enum elemType {
    EVEN,
    ODD
};

void quickSort(int *array, int size) {
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

void mergeAndSort(const std::vector<int> vec1, const std::vector<int> vec2, int *write_to) {
    int i = 0, j = 0;
    int size1 = vec1.size(), size2 = vec2.size();

    while (i < size1 && j < size2) {
        write_to[i + j] = vec1[i];
        write_to[i + j + 1] = vec2[j];
        i++;
        j++;
    }

    while (i < size1) {
        write_to[size2 + i] = vec1[i];
        i++;
    }
    while (j < size2) {
        write_to[size1 + j] = vec2[j];
        j++;
    }

    i = 1;  // Первый (нулевой) элемент проверять не нужно
    while (i < size1 + size2 - 1) {
        if (write_to[i] > write_to[i + 1]) {
            j = write_to[i];
            write_to[i] = write_to[i + 1];
            write_to[i + 1] = j;
        }
        i++;
    }
}

void selectElements(elemType type, const int *arr1, int size1, const int *arr2, int size2, std::vector<int> *result) {
    int i, j;
    if (type == EVEN)
        i = 0, j = 0;
    else
        i = 1, j = 1;
    result->reserve(size1 + size2);
    while (i < size1 && j < size2) {
        if (arr1[i] <= arr2[j]) {
            result->push_back(arr1[i]);
            i += 2;
        } else {
            result->push_back(arr2[j]);
            j += 2;
        }
    }
    if (i >= size1) {
        while (j < size2) {
            result->push_back(arr2[j]);
            j += 2;
        }
    } else {
        while (i < size1) {
            result->push_back(arr1[i]);
            i += 2;
        }
    }
}

void printArray(int *array, const int size) {
    for (int i = 0; i < size; i++) {
        std::cout << array[i] << " ";
    }
    std::cout << std::endl;
}

bool check(int *A, int *B, int size) {
    for (int i = 0; i < size; ++i)
        if (A[i] != B[i]) {
            return false;
        }
    return true;
}

int compare(const int *i, const int *j) {
    return *i - *j;
}

int main(int argc, char *argv[]) {
    int threads = 4;
    int size = kLength;
    if (argc == 2) {
        threads = atoi(argv[1]);
    } else if (argc == 3) {
        threads = atoi(argv[1]);
        size = atoi(argv[2]);
    }

    // int size = size;
    int *array = new int[size];
    int *copy1 = new int[size];
    int *copy2 = new int[size];
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

    double time = omp_get_wtime();
    double timeSerial = 0;
    int step;
    std::vector<int> *tempArray = new std::vector<int>[threads];
    int *shift = new int[threads];  // shift - массив сдвигов
    int *chunk = new int[threads];  // chunk - массив, содержащий размеры частей массива
#pragma omp parallel shared(array, step, shift, chunk, tempArray) num_threads(threads)
    {
        int thread_id, thread_index;  // thread_id - переменная для хранения ID текущего потока,
                                // thread_index - определяет необходимый сдвиг для получения
                                // парного потока (на шаге 1 = 1, на шаге 2 = 2 и т.д.)
        thread_id = omp_get_thread_num();

        /* Распределение частей исходного массива по потокам и сортировка данных частей */

        shift[thread_id] = thread_id * (size / threads);
        chunk[thread_id] = (thread_id == threads - 1) ? (size / threads) +
        (size % threads) : size / threads;
        quickSort(array + shift[thread_id], chunk[thread_id]);
#pragma omp barrier  // Ожидаем, пока все потоки отсортируют свою часть массива

        step = 1;
        while (step < threads) {
            /* На каждом шаге выбираем четные и нечетные элементы из парных потоков, записываем в tempArray */
            thread_index = static_cast<int>(pow(2, step - 1));

            if (thread_id % (thread_index * 2) == 0) {
                selectElements(EVEN, array + shift[thread_id], chunk[thread_id], array + shift[thread_id +
                thread_index], chunk[thread_id + thread_index], &tempArray[thread_id]);
            } else if (thread_id % thread_index == 0) {
                selectElements(ODD, array + shift[thread_id], chunk[thread_id], array +
                shift[thread_id - thread_index], chunk[thread_id - thread_index], &tempArray[thread_id]);
            }
#pragma omp barrier  // Ожидаем выполнения данной части всеми потоками
                     /* Производим слияние и сортировку tempArray в парных потоках */
            if (thread_id % (thread_index * 2) == 0) {
                mergeAndSort(tempArray[thread_id], tempArray[thread_id + thread_index], array + shift[thread_id]);
                chunk[thread_id] += chunk[thread_id + thread_index];
                tempArray[thread_id].clear();
                tempArray[thread_id].shrink_to_fit();
                tempArray[thread_id + thread_index].clear();
                tempArray[thread_id + thread_index].shrink_to_fit();
            }
#pragma omp single
            {
                step *= 2;  // Переходим на следующий шаг первым освободившимся потоком
            }
#pragma omp barrier  // Ждем пока слияние и сортировку выполнят все потоки
        }
    }
    delete[] tempArray;
    delete[] chunk;
    delete[] shift;

    time = omp_get_wtime() - time;

    if (size <= 100) {
        printArray(array, size);
    }

    qsort(copy1, size, sizeof(int), (int (*)(const void *, const void *))compare);

    std::cout << "OpenMP time: " << time << std::endl;
    timeSerial = omp_get_wtime();
    quickSort(copy2, size);
    timeSerial = omp_get_wtime() - timeSerial;

    std::cout << "Serial time time: " << std::fixed << std::setprecision(8) << timeSerial << std::endl;

    std::cout << "Boost: " << timeSerial / time << std::endl;

    if (check(copy1, array, size))
        std::cout << "Arrays are equal" << std::endl;
    else
        std::cout << "Arrays are NOT equal" << std::endl;


    delete[] copy1;
    delete[] copy2;
    delete[] array;
    return 0;
}
