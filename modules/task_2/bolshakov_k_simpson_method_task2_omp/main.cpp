// Copyright 2019 Bolshakov Konstantin
#include <omp.h>

#include <stdio.h>
#include <math.h>
#include <iostream>

double func1(double x, double y) {
    return x + y;
}
double func5(double x, double y) {
    return 2*x+y;
}

double func2(double x, double y) {
    return x * x + y * y;
}

double func3(double x, double y) {
    return 2;
}

double func4(double x, double y) {
    return sin(x * y) + exp(cos(y * x));
}

double func_simpson(double f(double, double),
 double xp, double yp, double x, double y,
    double xn, double yn) {
    return (f(xp, yp) + 4 * f(x, y) + 2* f(xn, yn));
}
double integrate_linear(double f(double, double),
    double lower_bound_x, double lower_bound_y,
     double upper_bound_x, double upper_bound_y,
    double h_x, double h_y, int x_n, int y_n) {
    h_x = (upper_bound_x - lower_bound_x) / x_n;
    h_y = (upper_bound_y - lower_bound_x) / y_n;
    double* x = new double[x_n + 1];
    double* y = new double[y_n + 1];
    double sum = 0;
    for (int i = 0; i <= x_n; i++) {
        x[i] = lower_bound_x + i * h_x;
    }
    for (int i = 0; i <= y_n; i++) {
        y[i] = lower_bound_y + i * h_y;
    }
    for (int i = 1; i < x_n; i += 2) {
        for (int j = 1; j < y_n; j += 2) {
            sum += func_simpson(f,
                x[i - 1], y[j - 1], x[i - 1], y[j], x[i - 1], y[j + 1]) +
                4 * func_simpson(f,
                    x[i], y[j - 1], x[i], y[j], x[i], y[j + 1]) +
                func_simpson(f,
                    x[i + 1], y[j - 1], x[i + 1], y[j], x[i + 1], y[j + 1]);
        }
    }
    return sum * h_x * h_y / 9;
}
double integrate_parallel(double f(double, double),
    double lower_bound_x, double lower_bound_y,
    double upper_bound_x,
    double upper_bound_y, double h_x,
    double h_y, int x_n, int y_n) {

    h_x = (upper_bound_x - lower_bound_x) / x_n;
    h_y = (upper_bound_y - lower_bound_x) / y_n;
    double* x = new double[x_n + 1];
    double* y = new double[y_n + 1];
    double sum = 0;
    for (int i = 0; i <= x_n; i++) {
        x[i] = lower_bound_x + i * h_x;
    }
    for (int i = 0; i <= y_n; i++) {
        y[i] = lower_bound_y + i * h_y;
    }

#pragma omp parallel for reduction(+:sum)
    for (int i = 1; i < x_n; i += 2) {
        for (int j = 1; j < y_n; j += 2) {
            sum +=
            func_simpson(f,
                x[i - 1], y[j - 1], x[i - 1], y[j], x[i - 1], y[j + 1]) +
                4 * func_simpson(f,
                    x[i], y[j - 1], x[i], y[j], x[i], y[j + 1]) +
                func_simpson(f,
                    x[i + 1], y[j - 1], x[i + 1], y[j], x[i + 1], y[j + 1]);
        }
    }
    return sum * h_x * h_y / 9;
}
int main(int argc, char* argv[]) {
    double t1 = omp_get_wtime();
    double linear_result = integrate_linear
    (func1, 11, 22, 33, 44, 0.0001, 0.0001, 300, 300);
    double linear_time = omp_get_wtime() - t1;

    omp_set_num_threads(4);
    t1 = omp_get_wtime();
    double parallel_result = integrate_parallel
    (func1, 11, 22, 33, 44, 0.0001, 0.0001, 300, 300);
    double parallel_time = omp_get_wtime() - t1;

    double boost = linear_time / parallel_time;

    std::cout << "linear time is " << linear_time<< " and result is "
    << linear_result << std::endl;
    std::cout << "parallel time is " << parallel_time << " and result is "
    << parallel_result << std::endl;
    std::cout << "boost is " << boost << std::endl;
    return 0;
}
