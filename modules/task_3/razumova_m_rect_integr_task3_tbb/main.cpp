// Copyright 2019 Razumova Maria
#include <math.h>
#include <tbb/tbb.h>
#include <iostream>
#include <functional>

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
    return tbb::parallel_reduce(
        tbb::blocked_range3d<int>(0, n, 0, n, 0, n), static_cast<double>(0),
        [&](const tbb::blocked_range3d<int> &range, double res)->double {
        for (int i = range.pages().begin(); i != range.pages().end(); ++i) {
            for (int j = range.rows().begin(); j != range.rows().end(); ++j) {
                for (int k = range.cols().begin(); k != range.cols().end(); ++k) {
                    double x = x1 + i * hx + hx / 2.0;
                    double y = y1 + j * hy + hy / 2.0;
                    double z = z1 + k * hz + hz / 2.0;
                    res += f(x, y, z)*hx*hy*hz;
                }
            }
        }
        return res;
    }, std::plus<double>());
}

double tripleIntegralLinear(double f(double, double, double),
    double x1, double y1, double z1, double hx, double hy, double hz, int n) {

    double sum = 0;
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            for (int k = 0; k < n; k++) {
                double x = x1 + i * hx + hx / 2.0;
                double y = y1 + j * hy + hy / 2.0;
                double z = z1 + k * hz + hz / 2.0;
                sum += f(x, y, z) * hx * hy * hz;
            }
        }
    }

    return sum;
}

int main(int argc, char* argv[]) {
    // tbb::task_scheduler_init init(4);
    int n;
    double x1, x2, y1, y2, z1, z2;
    double hx, hy, hz;
    if (argc == 1) {
        tbb::tick_count t1 = tbb::tick_count::now();
        std::cout << tripleIntegral(xplusyplusz, 0, 0, 0, 0.01, 0.01, 0.01, 100) << std::endl;
        tbb::tick_count t2 = tbb::tick_count::now();
        std::cout << std::endl << (t2 - t1).seconds();
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
        tbb::tick_count t1 = tbb::tick_count::now();
        double integral = tripleIntegral(ff, x1, y1, z1, hx, hy, hz, n);
        tbb::tick_count t2 = tbb::tick_count::now();
        double parallelTime = (t2 - t1).seconds();
        std::cout << "integration result " << integral << " parallel time: " << parallelTime << std::endl;
        t1 = tbb::tick_count::now();
        integral = tripleIntegralLinear(ff, x1, y1, z1, hx, hy, hz, n);
        t2 = tbb::tick_count::now();
        double linearTime = (t2 - t1).seconds();
        std::cout << "integration result " << integral << " Linear time: " << linearTime << std::endl;
        std::cout << "spead up " << linearTime / parallelTime;
    }

    return 0;
}
