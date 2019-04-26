// Copyright 2019 Kutovoi Vadim
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <omp.h>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <string>



double middle_rectangle_integral(const double a1, const double b1, const double h);
double middle_rectangle_integral(const double a1, const double b1, const double a2, const double b2, const double h);

inline double f(double x);
inline double f(double x, double y);

char* getCmdOption(char ** begin, char ** end, const std::string& option);
bool cmdOptionExists(char** begin, char** end, const std::string& option);

inline double f(double x) {
    return sin(x);
}

inline double f(double x, double y) {
    return sin(x) * cos(y);
}

double middle_rectangle_integral(const double a1, const double b1, const double h) {
    double sum = 0;
    int i = 0;

#pragma omp parallel
    {
        if (omp_get_thread_num() == 0)
            std::cout << "Threads : " << omp_get_num_threads() << std::endl;

#pragma omp for private(i) reduction(+: sum)
        for (i = 0; i < static_cast<int>((b1 - a1) / h); i++) {
            sum += f(a1 + i * h) * h;
        }
    }
    return sum;
}

double middle_rectangle_integral(const double a1, const double b1, const double a2, const double b2, const double h) {
    double sum = 0;
    int i, j = 0;

#pragma omp parallel
    {
        if (omp_get_thread_num() == 0)
            std::cout << "Threads : " << omp_get_num_threads() << std::endl;

#pragma omp for private(i, j) reduction(+: sum)
        for (i = 0; i < static_cast<int>((b1 - a1) / h); i++)
            for (j = 0; j < static_cast<int>((b2 - a2) / h); j++) {
                sum += f(a1 + i * h, a2 + j * h) * h * h;
            }
    }
    return sum;
}

char* getCmdOption(char **begin, char **end, const std::string& option) {
    char **itr = std::find(begin, end, option);
    if (itr != end && ++itr != end)
        return *itr;
    return 0;
}

bool cmdOptionExists(char** begin, char** end, const std::string& option) {
    return std::find(begin, end, option) != end;
}

int main(int argc, char *argv[]) {
    double a1 = 0;
    double b1 = 16;
    double a2 = INFINITY;
    double b2 = INFINITY;
    double h = 0.1;
    double res_middle = 0;
    bool verbose = false;
    bool log = false;

    if (cmdOptionExists(argv, argv + argc, "-a1")) {
        char *wcount = getCmdOption(argv, argv + argc, "-a1");
        a1 = atof(wcount);
    }

    if (cmdOptionExists(argv, argv + argc, "-b1")) {
        char *wcount = getCmdOption(argv, argv + argc, "-b1");
        b1 = atof(wcount);
    }

    if (cmdOptionExists(argv, argv + argc, "-a2")) {
        char *wcount = getCmdOption(argv, argv + argc, "-a2");
        a2 = atof(wcount);
    }

    if (cmdOptionExists(argv, argv + argc, "-b2")) {
        char *wcount = getCmdOption(argv, argv + argc, "-b2");
        b2 = atof(wcount);
    }

    if (cmdOptionExists(argv, argv + argc, "-h")) {
        char *wcount = getCmdOption(argv, argv + argc, "-h");
        h = atof(wcount);
    }

    if (cmdOptionExists(argv, argv + argc, "-v")) {
        verbose = true;
    }

    if (cmdOptionExists(argv, argv + argc, "-log")) {
        log = true;
    }

    if (log) std::cout << "Logging into log.txt" << std::endl;

    if (verbose) {
        std::cout << "h = " << h << std::endl;
        std::cout << "a1 = " << a1 << std::endl;
        std::cout << "b1 = " << b1 << std::endl;
    }

    if (a2 != INFINITY && b2 != INFINITY) {
        if (verbose) {
            std::cout << "a2 = " << a2 << std::endl;
            std::cout << "b2 = " << b2 << std::endl;
        }
        std::cout << "Two dimensional integral counting..." << std::endl;
    } else {
        std::cout << "One dimensional integral counting..." << std::endl;
    }

    double time = omp_get_wtime();
    if (a2 == INFINITY || b2 == INFINITY) {
        res_middle = middle_rectangle_integral(a1, b1, h);
    } else {
        res_middle = middle_rectangle_integral(a1, b1, a2, b2, h);
    }
    time = omp_get_wtime() - time;

    std::cout << "Middle rectangle method : " << res_middle << std::endl;

    std::cout << "Time : " << time << std::endl;

    if (log) {
        std::fstream log;
        log.open("log.txt", std::ios::out | std::ios::app);
        log << "Middle rectangle method : " << res_middle << std::endl;
        log.close();
    }

    return 0;
}
