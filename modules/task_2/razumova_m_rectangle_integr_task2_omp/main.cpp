// Copyright 2019 Razumova Maria
#include <omp.h>
#include <math.h>
#include <iostream>

double xplusyplusz(double x, double y, double z) {
    return x + y + z;
}

double sphere(double x, double y, double z) {
    return x * x + y * y + z * z;
}

double constantTwo(double x, double y, double z) {
    return 2;
}

double ff(double x, double y, double z) {
    return exp(-x * x - y * y - z * z);
}

double trignometricChaos(double x, double y, double z) {
    return sin(x*y) + cos(z) + exp(cos(y*z));
}

double tripleIntegral(double f(double, double, double),
    double x1, double y1, double z1, double hx, double hy, double hz, int n) {

    double sum = 0;

    #pragma omp parallel for  reduction (+:sum)
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            for (int k = 0; k < n; k++) {
                double x = x1 + i * hx + hx / 2.0;
                double y = y1 + j * hy + hy / 2.0;
                double z = z1 + k * hz + hz / 2.0;
                sum += f(x, y, z)*hx*hy*hz;
            }
        }
    }

    return sum;
}

int main(int argc, char* argv[]) {
    int n;
    double x1, x2, y1, y2, z1, z2;
    double hx, hy, hz;
    if (argc == 1) {
        omp_set_num_threads(4);
        std::cout << tripleIntegral(xplusyplusz, 0, 0, 0, 0.01, 0.01, 0.01, 100) << std::endl;
    } else {
        x1 = atof(argv[1]);
        x2 = atof(argv[2]);
        y1 = atof(argv[3]);
        y2 = atof(argv[4]);
        z1 = atof(argv[5]);
        z2 = atof(argv[6]);
        n = atoi(argv[7]);
        hx = (x2 - x1) / n;
        hy = (y2 - y1) / n;
        hz = (z2 - z1) / n;
        omp_set_num_threads(4);
        double t1 = omp_get_wtime();
        double integral = tripleIntegral(ff, x1, y1, z1, hx, hy, hz, n);
        double t2 = omp_get_wtime();
        std::cout << integral;
        double parallelTime = t2 - t1;
        std::cout << std::endl << parallelTime << std::endl;
        omp_set_num_threads(1);
        t1 = omp_get_wtime();
        integral = tripleIntegral(ff, x1, y1, z1, hx, hy, hz, n);
        t2 = omp_get_wtime();
        std::cout << integral;
        std::cout << std::endl << t2 - t1 << std::endl << (t2 - t1) / parallelTime;
    }

    return 0;
}
