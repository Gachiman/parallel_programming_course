// Copyright 2019 Kutovoi Vadim

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <tbb/tbb.h>
#include <tbb/tick_count.h>

#include <iostream>
#include <fstream>
#include <algorithm>
#include <string>


double middle_rectangle_integral(const double a1, const double b1,
                                                  const double h);
double middle_rectangle_integral(const double a1, const double b1,
                const double a2, const double b2, const double h);

inline double f(double x);
inline double f(double x, double y);

char* getCmdOption(char ** begin, char ** end, const std::string& option);
bool cmdOptionExists(char** begin, char** end, const std::string& option);

class Integrator {
 private:
    double a1, b1, h, res;

 public:
    explicit Integrator(double ta1, double tb1,
        double th) : a1(ta1), b1(tb1), h(th), res(0) {}

    Integrator(Integrator& m, tbb::split) : a1(m.a1), h(m.h), res(0) {
        double middle = (m.b1 - m.a1) / 2;
        b1 = middle;
        m.a1 = middle;
    }

    void operator()(const tbb::blocked_range<double>& r) {
        double begin = r.begin(), end = r.end();
        res += middle_rectangle_integral(begin, end, h);
    }

    void join(const Integrator& integrator) {
        res += integrator.res;
    }

    double Result() {
        return res;
    }
};

class Integrator2D {
 private:
    double a1, b1, a2, b2, h, res;

 public:
    explicit Integrator2D(double ta1, double tb1, double ta2, double tb2,
        double th) : a1(ta1), b1(tb1), a2(ta2), b2(tb2), h(th), res(0) {}

    Integrator2D(Integrator2D& m, tbb::split) :
     a1(m.a1), a2(m.a2), h(m.h), res(0) {
        double middle1 = (m.b1 - m.a1) / 2;
        double middle2 = (m.b2 - m.a2) / 2;
        b1 = middle1;
        b2 = middle2;
        m.a1 = middle1;
        m.a2 = middle2;
    }

    void operator()(const tbb::blocked_range2d<double>& r) {
        double begin1 = r.rows().begin(), end1 = r.rows().end();
        double begin2 = r.cols().begin(), end2 = r.cols().end();
        res += middle_rectangle_integral(begin1, end1, begin2, end2, h);
    }

    void join(const Integrator2D& integrator) {
        res += integrator.res;
    }

    double Result() {
        return res;
    }
};


inline double f(double x) {
    return sin(x);
}

inline double f(double x, double y) {
    return sin(x) * cos(y);
}

double middle_rectangle_integral(const double a1, const double b1,
                                                  const double h) {
    double sum = 0;
    int i = 0;

    for (i = 0; i < static_cast<int>((b1 - a1) / h); i++) {
        sum += f(a1 + i * h) * h;
    }

    return sum;
}

double middle_rectangle_integral(const double a1, const double b1,
                                 double a2, double b2, double h) {
    double sum = 0;
    int i, j = 0;

    for (i = 0; i < static_cast<int>((b1 - a1) / h); i++)
        for (j = 0; j < static_cast<int>((b2 - a2) / h); j++) {
            sum += f(a1 + i * h, a2 + j * h) * h * h;
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
    double a1 = 0, b1 = 16;
    double a2 = INFINITY, b2 = INFINITY;
    double h = 0.01;
    double res_middle = 0.0;
    bool verbose = false;
    bool log = false;
    int num_threads = 4;
    tbb::tick_count t0;
    tbb::tick_count t1;

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

    if (cmdOptionExists(argv, argv + argc, "-n")) {
        char *wcount = getCmdOption(argv, argv + argc, "-n");
        num_threads = atoi(wcount);
    }

    if (cmdOptionExists(argv, argv + argc, "-v")) {
        verbose = true;
    }

    if (cmdOptionExists(argv, argv + argc, "-log")) {
        log = true;
    }

    if (log) std::cout << "Logging into log.txt" << std::endl;

    if (verbose) {
        std::cout << "num_threads = " << num_threads << std::endl;
        std::cout << "h = " << h << std::endl;
        std::cout << "a1 = " << a1 << std::endl;
        std::cout << "b1 = " << b1 << std::endl;
    }

    tbb::task_scheduler_init init(num_threads);

    size_t GrainSize1 = static_cast<size_t>((b1 - a1) / num_threads + 1);

    if (a2 != INFINITY && b2 != INFINITY) {
        if (verbose) {
            std::cout << "a2 = " << a2 << std::endl;
            std::cout << "b2 = " << b2 << std::endl;
        }
        std::cout << "Two dimensional integral counting..." << std::endl;

        Integrator2D integr(a1, b1, a2, b2, h);

        size_t GrainSize2 = static_cast<size_t>((b2 - a2) / num_threads + 1);

        t0 = tbb::tick_count::now();
        tbb::parallel_reduce(tbb::blocked_range2d<double>(a1, b1,
                         GrainSize1, a2, b2, GrainSize2), integr);
        t1 = tbb::tick_count::now();

        std::cout << "Middle rectangle method : " <<
                      integr.Result()
                  << std::endl;
    } else {
        std::cout << "One dimensional integral counting..." << std::endl;

        Integrator integr(a1, b1, h);

        t0 = tbb::tick_count::now();
        tbb::parallel_reduce(tbb::blocked_range<double>(a1, b1,
                                           GrainSize1), integr);
        t1 = tbb::tick_count::now();

        std::cout << "Middle rectangle method : " <<
                      integr.Result()
                  << std::endl;
    }

    std::cout << "Time : " << (t1 - t0).seconds() << std::endl;

    if (log) {
        std::fstream log;
        log.open("log.txt", std::ios::out | std::ios::app);
        log << "Time : " << res_middle << std::endl;
        log.close();
    }

    return 0;
}
