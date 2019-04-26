// Copyright 2019 Kaganov Dmitryi
// Radix sort with simple merge (int)
#include <omp.h>
#include <ctime>
#include <cmath>
#include <utility>
#include <iostream>

unsigned int *sortMerge(unsigned int *firstArray, int firstSize,
                        unsigned int *secondArray, int secondSize) {
    unsigned int *array = new unsigned int[firstSize + secondSize];
    int i = 0, j = 0, k = 0;
    unsigned int tmp1 = 0;
    unsigned int tmp2 = 0;
    while (i < firstSize && j < secondSize) {
        tmp1 = firstArray[i];
        tmp2 = secondArray[j];
        if (tmp1 <= tmp2) {
            array[k] = tmp1;
            i++;
        } else {
            array[k] = tmp2;
            j++;
        }
        k++;
    }
    while (i < firstSize)
        array[k++] = firstArray[i++];
    while (j < secondSize)
        array[k++] = secondArray[j++];
    return array;
}

void lsdSort(unsigned int* A, unsigned int count) {
    unsigned int* B = new unsigned int[count];
    unsigned int index[4][256] = { {0} };
    unsigned int x, y, z;
    for (unsigned int i = 0; i < count; i++) {
        x = A[i];
        for (int j = 0; j < 4; j++) {
            index[j][static_cast<int>(x & 0xff)]++;
            x >>= 8;
        }
    }
    for (int i = 0; i < 4; i++) {
        y = 0;
        for (int j = 0; j < 256; j++) {
            z = index[i][j];
            index[i][j] = y;
            y += z;
        }
    }
    for (int i = 0; i < 4; i++) {
        for (unsigned int j = 0; j < count; j++) {
            x = A[j];
            y = static_cast<int>(x >> (i << 3)) & 0xff;
            B[index[i][y]++] = x;
        }
        std::swap(A, B);
    }
    delete[] B;
}

unsigned int* radixSortLinear(unsigned int* A, unsigned int arrSize, int size, int mergeNum) {
    unsigned int* R = new unsigned int[arrSize];
    unsigned int* buff = new unsigned int[size];                    // auxiliary array for merge
    unsigned int* lastBuff = new unsigned int[arrSize % size];      // remainder array
    for (unsigned int i = 0; i < arrSize; i += (size)) {
        // merge for remainder
        if ((i == arrSize - arrSize % size) && (arrSize % mergeNum != 0)) {
            for (unsigned int j = i, k = 0; j < i + (arrSize % size); j++, k++)
                lastBuff[k] = A[j];

            lsdSort(lastBuff, arrSize % size);
            // printArray(lastBuff, arrSize % size);
            R = sortMerge(R, i, lastBuff, arrSize % size);

            // merge for without remainder
        } else {
            for (unsigned int j = i, k = 0; j < i + size; j++, k++)
                buff[k] = A[j];

            lsdSort(buff, size);
            // printArray(buff, size);

            // if the  buff is first, then it is placed in the resulting (R)
            if (i == 0) {
                for (int j = 0; j < size; j++) {
                    R[j] = buff[j];
                }
            } else {                                        // for further merging with him
                R = sortMerge(R, i, buff, size);
            }
        }
    }
    return R;

    delete[] R;
    delete[] buff;
    delete[] lastBuff;
}

unsigned int* radixSortParallel(unsigned int* A, unsigned int arrSize, int size, int mergeNum, int numThreads) {
    // pointless copy array A
    unsigned int* R = new unsigned int[arrSize];
    for (unsigned int i = 0; i < arrSize; i++)
        R[i] = A[i];
    // auxiliary array containing size of "size"
    int* sizeArr = new int[mergeNum];
    int remainder = arrSize % size;
    for (int i = 0; i < mergeNum; i++) {
        sizeArr[i] = size;
    }
    if (arrSize % mergeNum != 0) {
        sizeArr[mergeNum - 1] = remainder;
    }

    omp_set_num_threads(numThreads);
    #pragma omp parallel for schedule(dynamic, 1)
    for (int i = 0; i < mergeNum; i++)
        lsdSort(R + i * size, sizeArr[i]);

    int counter = static_cast<int>(std::log(mergeNum) / std::log(2));

    int block = 0;
    for (int c = 0; c < counter; c++) {
        if (c != 0) {
            size = size * 2;
        }
        block = static_cast<int>(mergeNum / pow(2, c) / 2);
        int* r1 = new int[block];
        int* l1 = new int[block];
        int* r2 = new int[block];
        int* l2 = new int[block];
        for (int j = 0; j < block; j++) {
            l1[j] = j * size * 2;
            r1[j] = j * size * 2 + size - 1;
            l2[j] = j * size * 2 + size;
            r2[j] = j * size * 2 + size * 2 - 1;;
        }
        r2[block - 1] = arrSize - 1;
        #pragma omp parallel for schedule(dynamic, 1)
        for (int i = 0; i < block; i++) {
            unsigned int* tmp = new unsigned int[r1[i] - l1[i] + 1 + r2[i] - l2[i] + 1];
            tmp = sortMerge(R + l1[i], r1[i] - l1[i] + 1, R + l2[i], r2[i] - l2[i] + 1);
            int j = l1[i], g = 0;
            while (j <= r2[i]) {
                R[j] = tmp[g++];
                j++;
            }
            delete[] tmp;
        }
    }
    return R;
    delete[] R;
    delete[] sizeArr;
}

void printArray(unsigned int *array, int size) {
    if (size < 11) {
        for (int i = 0; i < size; i++) {
            std::cout << array[i] << " ";
        }
        std::cout << std::endl;
    }
}

void check(unsigned int* R, unsigned int size) {
    for (unsigned int i = 0; i < size - 1; i++) {
        if (R[i] > R[i + 1]) {
            std::cout << "Error! Array not sorted!\n";
            return;
        }
    }
    std::cout << "Array is sorted\n";
}

void checkResult(unsigned int* linearResult, unsigned int* parallelResult, unsigned int size) {
    for (unsigned int i = 0; i < size - 1; i++) {
        if (linearResult[i] != parallelResult[i]) {
            std::cout << "\nError! Arrays do not coincide! Perhaps one of them is incorrectly sorted\n";
            return;
        }
    }
    std::cout << "\nBoth arrays are sorted correctly and coincide. Congratulations!\n";
}

int main(int argc, char** argv) {
    std::srand(static_cast<int>(time(NULL)));
    const int numThreads = 8;
    int rank = 1000;
    int mergeNum = 0;                   // number of mergers
    unsigned int arrSize = 0;           // array size
    unsigned int* inputArray = NULL;
    unsigned int* linearResult = NULL;
    unsigned int* parallelResult = NULL;
    double t1, t2, pt1, pt2;

    if (argc > 2) {
        arrSize = atoi(argv[1]);
        mergeNum = atoi(argv[2]);
    } else {
        arrSize = 10;
        mergeNum = 2;
    }

    // rounding for proper array division
    int size = (static_cast<int>(arrSize) + static_cast<int>(mergeNum) - 1) / static_cast<int>(mergeNum);

    std::cout << "\nArray size : " << arrSize << std::endl;
    std::cout << "Number of divisions : " << mergeNum << std::endl;
    std::cout << "Size of divisions : " << size << std::endl;
    inputArray = new unsigned int[arrSize];
    linearResult = new unsigned int[arrSize];
    parallelResult = new unsigned int[arrSize];

    for (unsigned int i = 0; i < arrSize; i++)
        inputArray[i] = std::rand() % rank + rank;

    std::cout << "\nInput array: \n";
    printArray(inputArray, arrSize);

// LINEAR BLOCK
    t1 = omp_get_wtime();
    linearResult = radixSortLinear(inputArray, arrSize, size, mergeNum);
    t2 = omp_get_wtime();

    std::cout << "\nSorted array:\n";
    printArray(linearResult, arrSize);
    check(linearResult, arrSize);
    std::cout << "Time: " << t2 - t1 << std::endl;
// END LINEAR BLOCK

// PARALLEL BLOCK
    pt1 = omp_get_wtime();
    parallelResult = radixSortParallel(inputArray, arrSize, size, mergeNum, numThreads);
    pt2 = omp_get_wtime();

    std::cout << "\nSorted array:\n";
    printArray(parallelResult, arrSize);
    check(parallelResult, arrSize);
    std::cout << "Time: " << pt2 - pt1 << std::endl;
// END PARALLEL BLOCK

    checkResult(linearResult, parallelResult, arrSize);
    std::cout << "Average acceleration: " << (t2 - t1) / (pt2 - pt1) << std::endl;

    delete[] inputArray;
    delete[] linearResult;
    delete[] parallelResult;

    return 0;
}
