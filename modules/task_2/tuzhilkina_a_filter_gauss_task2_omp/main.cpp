// Copyright 2019 Tuzhilkina Anastasia

#include <stdlib.h>
#include <stdio.h>
#include <omp.h>
#include <math.h>
#include <iostream>

void InitMatr(int rows, int cols, int** m) {
    for (int i = 0; i < rows; i++)
        for (int j = 0; j < cols; j++)
            m[i][j] = static_cast<int>(std::rand() % 255);
}


void InitKern(double kernel[3][3], int radius, double sigma) {
    double norm = 0;

    for (int i = -radius; i <= radius; i++)
        for (int j = -radius; j <= radius; j++) {
            kernel[i + radius][j + radius] = (exp(-(i * i + j * j) / (2 * sigma * sigma)));
            norm += kernel[i + radius][j + radius];
        }

    for (int i = 0; i < 3; i++)
        for (int j = 0; j < 3; j++)
            kernel[i][j] /= norm;
}

inline int Clamp(int value, int min, int max) {
    if (value < min)
        return min;

    if (value > max)
        return max;

    return value;
}

void Gauss_par(int rows, int cols, double kernel[3][3], int **picture1, int **picture2) {
    double temp = 0.0;
    omp_set_num_threads(2);

#pragma omp parallel for firstprivate(temp)
    for (int j = 0; j < cols; j++)
        for (int i = 0; i < rows; i++) {
            temp = 0.0;
            for (int q = -1; q <= 1; q++)
                for (int l = -1; l <= 1; l++) {
                    int idX = Clamp(i + q, 0, rows - 1);
                    int idY = Clamp(j + l, 0, cols - 1);
                    temp += picture1[idX][idY] * kernel[q + 1][l + 1];
                }
            picture2[i][j] = static_cast<int>(temp);
        }
}

int main() {
    int rows, cols;
    double st, end;
    double kernel[3][3];

    rows = 5000, cols = 5000;

    int** picture = new int*[rows];
    int** res_par = new int*[rows];

    for (int i = 0; i < rows; i++) {
        picture[i] = new int[cols];
        res_par[i] = new int[cols];
    }

    InitMatr(rows, cols, picture);
    InitKern(kernel, 1, 1.0);

    st = omp_get_wtime();
    Gauss_par(rows, cols, kernel, picture, res_par);
    end = omp_get_wtime();
    std::cout << "Time par:" << end - st << std::endl;

    for (int i = 0; i < rows; i++) {
        delete[] picture[i];
        delete[] res_par[i];
    }

    delete[] picture;
    delete[] res_par;

    return 0;
}

