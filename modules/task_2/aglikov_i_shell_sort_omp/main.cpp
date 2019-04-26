//  Copyright 2019 Aglikov Ilya

#include <omp.h>
#include <iostream>
#include <algorithm>
#include <utility>

void shellsort(int* arr, int size);
void printArr(int* arr, int size);
bool checkEquality(int* arr0, int* arr1, int size);

int main(int argc, char* argv[]) {
	int* arr0 = nullptr;
	int* arr1 = nullptr;
	int* arr2 = nullptr;
	int size = 10;

	if (argc > 1)   size = atoi(argv[1]);
	arr0 = new int[size];
	arr1 = new int[size];
	arr2 = new int[size];
	for (int i = 0; i < size; i++)
		arr0[i] = arr1[i] = arr2[i] = size - i;
	if (size <= 30) {
		std::cout << "Original array: ";
		printArr(arr0, size);
	}

	double singleTime = omp_get_wtime();
	std::sort(arr0, arr0 + size);    // Single sort
	singleTime = omp_get_wtime() - singleTime;

	double parTime = omp_get_wtime();
	shellsort(arr1, size);
	parTime = omp_get_wtime() - parTime;

	double parTime1 = omp_get_wtime();
#pragma omp parallel
	{
#pragma omp master
		shellsort(arr2, size);
		std::cout << "LOL\n";
	}
	parTime1 = omp_get_wtime() - parTime1;

	if (size <= 30) {
		std::cout << "Std sort: ";
		printArr(arr0, size);
		std::cout << "Shell sort single: ";
		printArr(arr2, size);
		std::cout << "Shell sort 1: ";
		printArr(arr1, size);
		std::cout << std::endl;
	}

	std::cout << std::fixed << "Single time = " << singleTime
		<< "\nParallel time 1 = " << parTime
		<< "\nParallel time 2 = " << parTime1
		<< "\nAcceleration = " << (singleTime / parTime) << std::endl;

	if (checkEquality(arr0, arr2, size))
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
				else break;
		}
		step /= 2;
	}
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
