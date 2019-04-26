// Copyright 2019 Pozdeeva Varvara
#include <omp.h>
#include <iostream>
#include <ctime>
#include <random>

double* CreateRandMatrix(int size) {
    double* tmp = new double[size*size];
    for (int i = 0; i < size*size; i++)
        tmp[i] = (std::rand() % 10000) / 1000.0f;
    return tmp;
}
void CheckEqual(double* A, double*B, int n) {
    for (int i = 0; i < n*n; i++) {
        if (std::fabs(A[i] - B[i]) > 0.000001) {
            std::cout << "NOT equal" << std::endl;
            return;
        }
    }
    std::cout << "equal" << std::endl;
}
int PrintMatrix(double* A, int N) {
    for (int i = 0; i < N*N; i += N) {
        for (int j = 0; j< N; j++)
            std::cout << A[i + j] << " ";
        std::cout << std::endl;
    }
    std::cout << std::endl;
    return 1;
}
void MultiMatrix(double* A, double* B, double* C, int n, int bSize) {
    for (int i = 0; i < bSize; ++i)
        for (int j = 0; j < bSize; ++j)
            for (int k = 0; k < bSize; ++k) {
                C[i * n + j] += A[i * n + k] * B[k * n + j];
            }
}
void Foxs(double* A, double* B, double* C, int n, int q) {
    int bSize = n / q;
    for (int i = 0; i < q; i++)
        for (int j = 0; j < q; j++)
            for (int k = 0; k < q; k++) {
                MultiMatrix(&A[(i*n+j)*bSize], &B[(j*n + k)*bSize], &C[(i*n + k)*bSize], n, bSize);
            }
}
void FoxsOmp(double* A, double* B, double* C, int n, int q) {
    int bSize = n / q;
#pragma omp parallel for
        for (int i = 0; i < q; i++)
            for (int j = 0; j < q; j++)
                for (int k = 0; k < q; k++) {
                     MultiMatrix(&A[(i*n + j)*bSize], &B[(j*n + k)*bSize], &C[(i*n + k)*bSize], n, bSize);
                }
}
int main(int argc, char** argv) {
    double *A, *B, *C, *C1;
    int N = 8, q = 2;
    if (argc == 3) {
        N = atoi(argv[1]);
        q = atoi(argv[2]);
    }
    srand((unsigned int)time(0));
    A = CreateRandMatrix(N);
    B = CreateRandMatrix(N);
    // PrintMatrix(A, N);
    // PrintMatrix(B, N);
    C = new double[N*N];
    C1 = new double[N*N];
    for (int i = 0; i < N*N; i++) {
        C[i] = 0.0;
        C1[i] = 0.0;
    }
    double startTime = omp_get_wtime();
    MultiMatrix(A, B, C, N, N);
    double finishTime = omp_get_wtime();
    std::cout << "Time usual is " << finishTime - startTime << std::endl;
    // PrintMatrix(C, N);
    startTime = omp_get_wtime();
    Foxs(A, B, C1, N, q);
    finishTime = omp_get_wtime();
    std::cout <<"Time consistent with Fox algorithm is "<< finishTime - startTime << std::endl;
    std::cout << "Matrices multiplied by usual and consistent Fox algorithm - ";
    // PrintMatrix(C1, N);
    CheckEqual(C, C1, N);
    for (int i = 0; i < N*N; i++)
        C1[i] = 0.0;
    startTime = omp_get_wtime();
    FoxsOmp(A, B, C1, N, q);
    finishTime = omp_get_wtime();
    std::cout << "Time parallel is " << finishTime - startTime << std::endl;
    std::cout << "Matrices multiplied by usual and parallel Fox algorithm - ";
    // PrintMatrix(C1, N);
    CheckEqual(C, C1, N);
    return 0;
}
