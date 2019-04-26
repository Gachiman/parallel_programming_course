// Copyright 2019 Kasmazyuk Nikita
#include <omp.h>
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

void Canon_Omp(double* pAMatrix, double* pBMatrix, double *pCMatrix, int q, int Size) {
    omp_set_num_threads(q);
    int GridSize = static_cast <int>(sqrt(q));
    int BlockSize = Size / GridSize;
    #pragma omp parallel
    {
        int ThreadID = omp_get_thread_num();
        int RowIndex = ThreadID / GridSize;
        int ColIndex = ThreadID%GridSize;
        for (int iter = 0; iter < GridSize; iter++) {
            for (int i = RowIndex*BlockSize; i < (RowIndex + 1)*BlockSize; i++)
                for (int j = ColIndex*BlockSize; j < (ColIndex + 1)*BlockSize; j++)
                    for (int k = iter*BlockSize; k < (iter + 1)*BlockSize; k++) {
                        pCMatrix[i*Size + j] += pAMatrix[i*Size + k] * pBMatrix[k*Size + j];
            }
        }
    }
}

int main(int argc, char** argv) {
    std::cout << "Chislo potokov (q) - polniy kvadrat!" << std::endl;
    int size = 4;
    int q = 2;
    int proverka = 0;
    double *A, *B, *C, *SS, *S, *C1;
    double time_par = 0;
    double time_pos = 0;
    double time_izi = 0;
    std::cout << "omp_get_max_threads() = " << omp_get_max_threads() << std::endl;
    if (argc > 2) {
    size = atoi(argv[1]);
    q = atoi(argv[2]);
    }
    A = CreateMatrix(size);
    SS = CreateMatrix(size);
    B = CreateMatrix(size);
    C = CreateMatrix(size);
    S = CreateMatrix(size);
    C1 = CreateMatrix(size);

    ClearMatrix(C, size);
    ClearMatrix(SS, size);
    ClearMatrix(S, size);
    ClearMatrix(C1, size);

    RandMatrix(A, B, size);

    if (size < 5) {
    PrintMatrix(A, size);
    PrintMatrix(B, size);
    }

    time_izi = omp_get_wtime();
    for (int i = 0; i < size; ++i)
        for (int j = 0; j < size; ++j)
            for (int k = 0; k < size; ++k) {
               SS[i * size + j] += A[i * size + k] * B[k * size + j];
            }
    time_izi = omp_get_wtime() - time_izi;

    time_pos = omp_get_wtime();
    Canon(A, B, C, size, q);
    time_pos = omp_get_wtime() - time_pos;

    time_par = omp_get_wtime();
    Canon_Omp(A, B, S, q, size);
    time_par = omp_get_wtime() - time_par;


    if (size < 5) {
    PrintMatrix(SS, size);
    PrintMatrix(C, size);
    PrintMatrix(S, size);
    }

    for (int i = 0; i < size; i++)
        for (int j = 0; j < size; j++) {
            if (fabs(S[i * size + j] - C[i * size + j]) < 0.1)
                proverka++;
            else
                proverka = 0;
        }

    std::cout << "Time izi version is " << time_izi << std::endl;
    std::cout << "Time posl version is " << time_pos << std::endl;
    std::cout << "Time parallel version is " << time_par << std::endl;
    std::cout << "Boost is " << time_izi / time_par << std::endl;
    if (proverka == 0)
        std::cout << "DANGER: Result not right" << std::endl;
    else
        std::cout << "Bingo! Result right!" << std::endl;

    return 0;
}
