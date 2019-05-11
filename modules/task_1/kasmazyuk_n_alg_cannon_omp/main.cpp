// Copyright 2019 Kasmazyuk Nikita
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <string>

double* CreateMatrix(int N) {
    double *matrix = new double[N*N];
    return matrix;
}

void PrintMatrix(double* matrix, int N) {
for (int i = 0; i < N*N; i += N) {
        for (int j = 0; j< N; j++)
            std::cout << matrix[i + j] << " ";
        std::cout << std::endl;
    }
    std::cout << std::endl;
}

void RandMatrix(double* matrix1, double* matrix2, int N) {
    for (int i = 0; i < N*N; ++i) {
            matrix1[i] = (std::rand() % 10000) / 1000.0f;
            matrix2[i] = (std::rand() % 10000) / 1000.0f;
        }
}

void ClearMatrix(double *C, int N) {
    for (int i = 0; i < N*N; ++i) {
            C[i] = 0;
    }
}

void MultMatrix(double* A, double* B, double* C, int blockSize, int N) {
    for (int i = 0; i < blockSize; ++i)
        for (int j = 0; j < blockSize; ++j)
            for (int k = 0; k < blockSize; ++k) {
                C[i * N + j] += A[i * N + k] * B[k * N + j];
            }
}

void Canon(double *A, double *B, double* C, int n, int q) {
    int blockSize = n / q;
    for (int i = 0; i < q; ++i) {
        for (int j = 0; j < q; ++j) {
            for (int k = 0; k < q; ++k) {
                MultMatrix(&A[(i*n + (j+i+k)%q)*blockSize], &B[(((i+j+k)%q)*n + j)*blockSize],
                    &C[(i*n + j)*blockSize], blockSize, n);
            }
        }
    }
}


int main(int argc, char** argv) {
    int size = 625;
    int q = 25;
    int prov = 0;
    double *A, *B, *C, *S;

    if (argc > 2) {
    size = atoi(argv[1]);
    q = atoi(argv[2]);
    }

    A = CreateMatrix(size);
    B = CreateMatrix(size);
    C = CreateMatrix(size);
    S = CreateMatrix(size);
    ClearMatrix(C, size);
    ClearMatrix(S, size);

    RandMatrix(A, B, size);
    if (size < 5) {
    PrintMatrix(A, size);
    PrintMatrix(B, size);
    }

    Canon(A, B, C, size, q);
    MultMatrix(A, B, S, size, size);

    if (size < 5) {
    PrintMatrix(C, size);
    PrintMatrix(S, size);
    }

    for (int i = 0; i < size*size; i += size) {
        for (int j = 0; j< size; j++)
            if ((C[i + j] - S[i + j]) > 0.001)
                prov++;
            else
                prov = 0;
    }

    if (prov == 0)
        std::cout << "All very well!" << std::endl;
    else
        std::cout << "Oooops :( Error" << std::endl;

    return 0;
}
