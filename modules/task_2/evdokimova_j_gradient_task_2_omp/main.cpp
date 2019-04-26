// Copyright 2019 Evdokimova Julia
#define _SCL_SECURE_NO_WARNINGS
#include <omp.h>
#include <iostream>
#include <random>
#include <iomanip>
#include <ctime>
#include <string>
#include <algorithm>
std::default_random_engine generator((unsigned)time(0));
std::uniform_int_distribution <int> dist(0, 10);

void CreateMatrix(double* matrix, int size) {
    for (int i = 0; i < size; i++) {
        for (int j = i; j < size; j++) {
            if (i == j)
                matrix[i * size + j] = dist(generator);
            else
                matrix[j * size + i] = matrix[i * size + j] = dist(generator);
        }
    }
}

void CreateVector(double* vector, int size) {
    for (int i = 0; i < size; i++) {
        vector[i] = dist(generator);
    }
}

void PrintVector(double* vector, int size) {
        for (int i = 0; i < size; i++) {
            std::cout << std::setprecision(3) << vector[i] << "\t";
        }
        std::cout << std::endl;
    }
void Print_SLU(double* matrix, double* vector, int size) {
        for (int i = 0; i < size; i++) {
            for (int j = 0; j < size; j++) {
                std::cout << std::setprecision(3) << matrix[i * size + j] << "\t";
            }
            std::cout << " | " << std::setprecision(3) << vector[i] << "\n";
        }
    }
// sequential
void MatrixVector(double* matrix, double* vector, double* result, int size) {
    for (int i = 0; i < size; i++) {
        result[i] = 0;
        for (int j = 0; j < size; j++)
            result[i] += matrix[i * size + j] * vector[j];
    }
}

double ScalVector(double* vec1, double* vec2, int size) {
    double result = 0;
    for (int i = 0; i < size; i++) {
        result += vec1[i] * vec2[i];
    }
    return result;
}
void SoprGradMethod(double* matrix, double* vector, double* x0,
    double eps, double* result, int* count, int maxIter, int size) {
    double* rPrev = new double[size];  // rPrev - neviazki tekuchego priblizhenie
    double* rNext = new double[size];  // rNext - neviazki sleduchego priblizhenie
    double* p = new double[size];  // p vector napravlenia
    double* y = new double[size];  // y vectornoe proizvedenie matrix*x0
    double* Ap = new double[size];  // Ap vectornoe proizvedenie matrix*p
    // check tekuchaia tochnost metoda, norm norma vectora vector
    // beta, alpha koefficient raschetnih formul
    double beta, alpha, check;
    double* swap;
    // initiallization metod
    *count = 0;
    // vicheslenie neviazki s nachalnim priblizhenie
    MatrixVector(matrix, x0, y, size);
    for (int i = 0; i < size; i++)
        rPrev[i] = vector[i] - y[i];
    std::copy(rPrev, rPrev + size, p);
    std::copy(x0, x0 + size, result);

    // iteracii metoda
    do {
        (*count)++;
        MatrixVector(matrix, p, Ap, size);
        alpha = ScalVector(rPrev, rPrev, size) / ScalVector(p, Ap, size);
        for (int i = 0; i < size; i++) {
            result[i] += alpha * p[i];
            rNext[i] = rPrev[i] - alpha * Ap[i];
        }
        beta = ScalVector(rNext, rNext, size) / ScalVector(rPrev, rPrev, size);
        // Norma neviazki
        check = sqrt(ScalVector(rNext, rNext, size));
        for (int j = 0; j < size; j++)
            p[j] = beta * p[j] + rNext[j];
        swap = rNext;
        rNext = rPrev;
        rPrev = swap;
    } while ((check > eps) && (*(count) <= maxIter));

    delete[] rPrev;
    delete[] rNext;
    delete[] p;
    delete[] y;
    delete[] Ap;
}
// omp
void MatrixVector_omp(double* matrix, double* vector, double* result, int size) {
#pragma omp parallel for
    for (int i = 0; i < size; i++) {
        result[i] = 0;
            for (int j = 0; j < size; j++) {
                result[i] += matrix[i * size + j] * vector[j];
            }
    }
}
double ScalVector_omp(double* vec1, double* vec2, int size) {
    double result = 0;
#pragma omp parallel for reduction(+:result)
    for (int i = 0; i < size; i++) {
        result += vec1[i] * vec2[i];
    }
    return result;
}
void SoprGradMethod_omp(double* matrix, double* vector, double* x0,
    double eps, double* result, int* count, int maxIter, int size) {
    double* rPrev = new double[size];  // rPrev - neviazki tekuchego priblizhenie
    double* rNext = new double[size];  // rNext - neviazki sleduchego priblizhenie
    double* p = new double[size];  // p vector napravlenia
    double* y = new double[size];  // y vectornoe proizvedenie matrix*x0
    double* Ap = new double[size];  // Ap vectornoe proizvedenie matrix*p
    // check tekuchaia tochnost metoda, norm norma vectora vector
    // beta, alpha koefficient raschetnih formul
    double beta, alpha, check;
    double* swap;
    // initiallization metod
    *count = 0;
    // vicheslenie neviazki s nachalnim priblizhenie
    MatrixVector_omp(matrix, x0, y, size);
    for (int i = 0; i < size; i++)
        rPrev[i] = vector[i] - y[i];
    std::copy(rPrev, rPrev + size, p);
    std::copy(x0, x0 + size, result);

    // iteracii metoda
    do {
        (*count)++;
        MatrixVector(matrix, p, Ap, size);
        alpha = ScalVector_omp(rPrev, rPrev, size) / ScalVector_omp(p, Ap, size);
        for (int i = 0; i < size; i++) {
            result[i] += alpha * p[i];
            rNext[i] = rPrev[i] - alpha * Ap[i];
        }
        beta = ScalVector_omp(rNext, rNext, size) / ScalVector_omp(rPrev, rPrev, size);
        // Norma neviazki
        check = sqrt(ScalVector_omp(rNext, rNext, size));
        for (int j = 0; j < size; j++)
            p[j] = beta * p[j] + rNext[j];
        swap = rNext;
        rNext = rPrev;
        rPrev = swap;
    } while ((check > eps) && (*(count) <= maxIter));
    delete[] rPrev;
    delete[] rNext;
    delete[] p;
    delete[] y;
    delete[] Ap;
}
int main(int argc, char **argv) {
    clock_t start_seq, finish_seq;
    double time_seq;
    double start_omp, finish_omp, time_omp;
    double eps = 0;  // tochnost
    int size = 0;  // razmer matrica
    int maxIter_seq = 0;  // max kol iteracii
    int maxIter_omp = 0;
    int count_seq;  // kol iteracii
    int count_omp;

    if (argc > 1) {
        omp_set_num_threads(atoi(argv[1]));
        size = atoi(argv[2]);
        eps = atoi(argv[3]);
    }

    double* matrix = new double[size * size];
    double* vector = new double[size];  // vector pravoi chasti
    double* result_omp = new double[size];  // vector result
    double* result_seq = new double[size];
    double* x0_seq = new double[size];  // nachal'noe priblizhenie
    double* x0_omp = new double[size];

    for (int i = 0; i < size; i++) {
        x0_seq[i] = 0;
        x0_omp[i] = 0;
    }
    if (maxIter_seq == 0)
        maxIter_seq = size * 10;
    if (maxIter_omp == 0)
        maxIter_omp = size * 10;

    CreateMatrix(matrix, size);
    CreateVector(vector, size);
    std::cout << "SYSTEM: " << std::endl;
    Print_SLU(matrix, vector, size);

    // Seq
    start_seq = clock();
    SoprGradMethod(matrix, vector, x0_seq, eps, result_seq, &count_seq, maxIter_seq, size);
    finish_seq = clock();
    time_seq = (finish_seq - start_seq) / CLOCKS_PER_SEC;

    // par
    start_omp = omp_get_wtime();
    SoprGradMethod_omp(matrix, vector, x0_omp, eps, result_omp, &count_omp, maxIter_omp, size);
    finish_omp = omp_get_wtime();
    time_omp = finish_omp - start_omp;

    #pragma omp critical

    std:: cout << "SEQUENTIAL ALGORITHM: " << std::endl;
    std::cout << "Solution: " << std::endl;
    PrintVector(result_seq, size);
    std::cout << "Count: " << count_seq << std::endl;
    std::cout << "Time: " << time_seq << " c" << std::endl;
    std::cout << "PARALLEL ALGORITHM: " << std::endl;
    std::cout << "Solution: " << std::endl;
    PrintVector(result_omp, size);
    std::cout << "Count: " << count_omp << std:: endl;
    std::cout << "Time: " << time_omp << " c" << std::endl;
    std::cout << "Acceleration: " << time_seq / time_omp << std::endl;

    delete[] matrix;
    delete[] vector;
    delete[] result_omp;
    delete[] result_seq;
    delete[] x0_omp;
    delete[] x0_seq;
}
