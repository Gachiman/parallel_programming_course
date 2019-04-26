// Copyright 2019 Volkov Daniel
#include<omp.h>
#include<iostream>

double oneDimensionalIntegral(double x, double func(double, double), double a, double b, int n);
double Trapezoid(double func(double, double), double ax, double bx,
double TopLimit(double), double LowerLimit(double), int n) {
    double step = (bx - ax) / n;
    double result = 0;
    for (int i = 1; i < n - 1; i++) {
        result += oneDimensionalIntegral(ax + i * step, func, TopLimit(ax + i * step), LowerLimit(ax + i * step), n);
    }
    result *= 2;
    result += oneDimensionalIntegral(ax, func, TopLimit(ax), LowerLimit(ax), n) +
    oneDimensionalIntegral(bx, func, TopLimit(bx), LowerLimit(bx), n);
    result *= (step / 2);
    return result;
}
double oneDimensionalIntegral(double x, double func(double, double), double b, double a, int n) {
    double step = (b - a) / n;
    double result = 0;
    for (int i = 1; i <= n - 1; i++) {
        result += func(x, a + i * step);
    }
    result *= 2;
    result += func(x, a) + func(x, b);
    result *= (step / 2);
    return result;
}
double MainFunction1(double x, double y) {
    return x + y;
}
double TopFunction(double x) {
    return 10;
}
double LowerFunction(double x) {
    return 0;
}
int main() {
    double start = omp_get_wtime();
    std::cout << Trapezoid(MainFunction1, 0, 10, TopFunction, LowerFunction, 10000);
    std::cout << "\nTime: " << omp_get_wtime() - start;
}
