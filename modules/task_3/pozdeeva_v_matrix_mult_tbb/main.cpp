// Copyright 2019 Pozdeeva Varvara

#include <tbb/tbb.h>

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
class Multiplicator {
    double *A, *B, *C;
    int n, q;
 public:
    Multiplicator(double *a, double *b, double *c,
        int N, int Q) : A(a), B(b), C(c), n(N), q(Q) {}
    void operator()(const tbb::blocked_range2d<int>& r) const {
        int bSize = n / q;
        for (int i = r.rows().begin(); i < r.rows().end(); ++i) {
            for (int j = r.cols().begin(); j < r.cols().end(); ++j) {
                for (int k = 0; k < q; ++k) {
                    MultiMatrix(&A[(i*n + k)*bSize], &B[(k*n + j)*bSize], &C[(i*n + j)*bSize], n, bSize);
                }
            }
        }
    }
};

void Foxs(double* A, double* B, double* C, int n, int q) {
    int bSize = n / q;
    for (int i = 0; i < q; i++)
        for (int j = 0; j < q; j++) {
            for (int k = 0; k < q; k++) {
                MultiMatrix(&A[(i*n + j)*bSize], &B[(j*n + k)*bSize], &C[(i*n + k)*bSize], n, bSize);
            }
        }
}
void FoxsTBB(double* A, double* B, double* C, int n, int q, int grainSize) {
    tbb::task_scheduler_init init;
    tbb::parallel_for(tbb::blocked_range2d<int>(0, q, grainSize, 0, q, grainSize),
        Multiplicator(A, B, C, n, q));
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
    //  PrintMatrix(A, N);
    C = new double[N*N];
    C1 = new double[N*N];
    for (int i = 0; i < N*N; i++) {
        C[i] = 0.0;
        C1[i] = 0.0;
    }
    int grainSize = 1;
    tbb::tick_count t1 = tbb::tick_count::now();
    Foxs(A, B, C, N, q);
    tbb::tick_count t2 = tbb::tick_count::now();
    std::cout << "Time consistent with Fox algorithm is " << (t2 - t1).seconds() << std::endl;
    //  PrintMatrix(C, N);
    tbb::tick_count t1tbb = tbb::tick_count::now();
    FoxsTBB(A, B, C1, N, q, grainSize);
    tbb::tick_count t2tbb = tbb::tick_count::now();
    std::cout << "Time parallel is " << (t2tbb - t1tbb).seconds() << std::endl;
    //  PrintMatrix(C1, N);
    CheckEqual(C, C1, N);

    std::cout << "Acceleration - " << ((t2 - t1).seconds()) / ((t2tbb - t1tbb).seconds()) << std::endl;

    delete[] A;
    delete[] B;
    delete[] C;
    delete[] C1;

    return 0;
}
