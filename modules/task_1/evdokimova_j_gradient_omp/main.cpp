// Copyright 2019 Evdokimova Julia
#define _SCL_SECURE_NO_WARNINGS
#include <random>
#include <iostream>
#include <iomanip>
#include <ctime>
#include <algorithm>
#include <string>
std::default_random_engine generator((unsigned)time(0));
std::uniform_int_distribution <int> dist(0, 10);

void CreateMatrix(double** matrix, int size) {
    for (int i = 0; i < size; i++) {
        for (int j = i; j < size; j++) {
            if (i == j)
                matrix[i][j] = dist(generator);
            else
                matrix[j][i] = matrix[i][j] = dist(generator);
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

void Print_SLU(double** matrix, double* vector, int size) {
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            std::cout << std::setprecision(3) << matrix[i][j] << "\t";
        }
        std::cout << " | " << std::setprecision(3) << vector[i] << "\n";
    }
}

void MatrixVector(double** matrix, double* vector, double* result, int size) {
    for (int i = 0; i < size; i++) {
        result[i] = 0;
        for (int j = 0; j < size; j++)
            result[i] += matrix[i][j] * vector[j];
    }
}

double ScalVector(double* vec1, double* vec2, int size) {
    double result = 0;
    for (int i = 0; i < size; i++) {
        result += vec1[i] * vec2[i];
    }
    return result;
}
void SoprGradMethod(double** matrix, double* vector, double* x0,
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

int main(int argc, char **argv) {
    double** matrix;  // matrica
    double* vector;  // vector pravoi chasti
    double* result;  // vector result
    double* x0;  // nachal'noe priblizhenie
    double eps = 0;  // tochnost
    int size = 0;  // razmer matrica
    int maxIter = 0;  // max kol iteracii
    int count;  // kol iteracii

    if (argc > 1) {
        size = atoi(argv[1]);
        eps = atoi(argv[2]);
    }
    matrix = new double*[size];
    for (int i = 0; i < size; i++)
        matrix[i] = new double[size];
    vector = new double[size];
    result = new double[size];
    x0 = new double[size];
    for (int i = 0; i < size; i++)
        x0[i] = 0;
    if (maxIter == 0)
        maxIter = size * 10;
    // vivod sys
    CreateMatrix(matrix, size);
    CreateVector(vector, size);
    std::cout << "System" << std::endl;
    if (size < 5)
        Print_SLU(matrix, vector, size);
    else
        std::cout << "System size large" << std::endl;
    SoprGradMethod(matrix, vector, x0, eps, result, &count, maxIter, size);
    std::cout << "Solution" << std::endl;
    if (size < 15)
        PrintVector(result, size);
    std::cout << "Iterations=" << count << std::endl;
    delete[] matrix;
    delete[] vector;
    delete[] result;
    delete[] x0;
    return 0;
}
