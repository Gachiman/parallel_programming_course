// Copyright 2019 Novikova Alisa
#include <tbb/tbb.h>
#include <iostream>
#include <ctime>
#include <random>

int ShowMatrix(double* A, int N) {
    std::cout << std::endl;
    for (int i = 0; i < N*N; i += N) {
        for (int j = 0; j< N; j++)
            std::cout << A[i + j] << " ";
        std::cout << std::endl;
    }
    std::cout << std::endl;
    return 1;
}
double* GetRandMatrix(int N) {
    double *result = new double[N*N];
    for (int i = 0; i< N*N; i++)
        result[i] = (std::rand() % 10000) / 1000.0f;
    return result;
}
void MultiplayMatrix(double* A, double* B, double* C, int blockSize, int n) {
    for (int i = 0; i < blockSize; ++i)
        for (int j = 0; j < blockSize; ++j)
            for (int k = 0; k < blockSize; ++k) {
                C[i * n + j] += A[i *n + k] * B[k *n + j];
            }
}
class MultiMatrixTBB{
    double *A, *B, *C;
    int n, q;
 public:
    MultiMatrixTBB(double *a, double *b, double *c,
        int n2, int q2) : A(a), B(b), C(c), n(n2), q(q2) {}
    void operator()(const tbb::blocked_range2d<int>& r) const {
        int blockSize = n / q;
        for (int i = r.rows().begin(); i < r.rows().end(); ++i) {
            for (int j = r.cols().begin(); j < r.cols().end(); ++j) {
                for (int k = 0; k < q; ++k) {
                    MultiplayMatrix(&A[(i*n + (j + i + k) % q)*blockSize],
                        &B[(((i + j + k) % q)*n + j)*blockSize],
                        &C[(i*n + j)*blockSize], blockSize, n);
                }
            }
        }
    }
};
void Cannon(double *A, double *B, double* C, int n, int q) {
    int blockSize = n / q;
    for (int i = 0; i < q; ++i) {
        for (int j = 0; j < q; ++j) {
            for (int k = 0; k < q; ++k) {
                MultiplayMatrix(&A[(i*n + (j + i + k) % q)*blockSize],
                    &B[(((i + j + k) % q)*n + j)*blockSize],
                    &C[(i*n + j)*blockSize], blockSize, n);
            }
        }
    }
}
void CannonTBB(double *A, double *B, double* C, int n, int q, int gSize) {
    tbb::task_scheduler_init init;
    tbb::parallel_for(tbb::blocked_range2d<int>(0, q, gSize, 0, q, gSize),
        MultiMatrixTBB(A, B, C, n, q));
}
int CheckMatrixForEqual(double *A, double *C, int N) {
    for (int i = 0; i < N*N; i++)
        if (std::abs(A[i] - C[i]) > 0.000001)
            return 0;
    return 1;
}
int main(int argc, char** argv) {
    srand((unsigned int)time(0));
    double *A, *B, *C1, *C2;
    int N = 1000, q = 2;
    int gSize = 1;
    if (argc == 3) {
        N = atoi(argv[1]);
        q = atoi(argv[2]);
    }
    A = GetRandMatrix(N);
    B = GetRandMatrix(N);
    // ShowMatrix(A, N);
    // ShowMatrix(B, N);
    C1 = new double[N*N];
    C2 = new double[N*N];
    for (int i = 0; i < N*N; i++) {
        C1[i] = 0.0;
        C2[i] = 0.0;
    }
    tbb::tick_count sTime1 = tbb::tick_count::now();
    Cannon(A, B, C1, N, q);
    tbb::tick_count fTime1 = tbb::tick_count::now();
    std::cout << "Time for Cannon " << (fTime1 - sTime1).seconds() << std::endl;
    // ShowMatrix(C1, N);
    tbb::tick_count sTime2 = tbb::tick_count::now();
    CannonTBB(A, B, C2, N, q, gSize);
    tbb::tick_count fTime2 = tbb::tick_count::now();
    // ShowMatrix(C2, N);
    std::cout << "Time for CannonTBB " << (fTime2 - sTime2).seconds() << std::endl;
    if (CheckMatrixForEqual(C1, C2, N) == 1)
        std::cout << "Matrices are equal" << std::endl;
    else
        std::cout << "Matrices are not equal" << std::endl;
    delete[] A;
    delete[] B;
    delete[] C1;
    delete[] C2;
    return 0;
}
