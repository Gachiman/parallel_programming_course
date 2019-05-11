// Copyright 2019 Pavel Vorobev

#include <tbb/tbb.h>
#include <tbb/parallel_sort.h>
#include <vector>
#include <random>
#include <ctime>
#include <iostream>


struct point {
    int x;
    int y;
    friend bool operator < (point a, point c){
        return a.x - a.y
            + c.y - c.x
            + c.x * a.y - c.y * a.x > 0;
    }
};

bool moreOrEqualThan(point a, point c, point b) {
    return a.x * b.y - a.y * b.x
        + b.x * c.y - b.y * c.x
        + c.x * a.y - c.y * a.x <= 0;
}

bool notEquals(point a, point b) {
    if (a.x != b.x)
        return true;
    else if (a.y != b.y)
        return true;
    else
        return false;
}

int determinant(point a, point b, point c) {
    return (b.x - a.x) * (c.y - a.y)
        - (c.x - a.x) * (b.y - a.y);
}

int partition(std::vector<point> *a, int low, int high) {
    point pivot = a->at(low);
    int i = low + 1;
    for (int j = low + 1; j <= high; j++) {
        if (moreOrEqualThan(pivot, a->at(j), a->at(0))) {
            point tmp = a->at(i);
            a->at(i) = a->at(j);
            a->at(j) = tmp;
            i++;
        }
    }
    pivot = a->at(i - 1);
    a->at(i - 1) = a->at(low);
    a->at(low) = pivot;
    return i - 1;
}

void quickSort(std::vector<point> *a, int low, int high) {
    if (low < high) {
        int partIndex = partition(a, low, high);
        quickSort(a, low, partIndex - 1);
        quickSort(a, partIndex + 1, high);
    }
}

int grahamScanLinear(std::vector<point> *a) {
    point p;
    p.x = 1;
    p.y = 1;
    a->push_back(p);
    point c = a->at(0);
    int m = 0;
    for (size_t i = 1; i < a->size(); i++) {
        if (a->at(i).x < c.x) {
            c = a->at(i);
            m = i;
        } else if (a->at(i).x == c.x) {
            if (a->at(i).y < c.y) {
                c = a->at(i);
                m = i;
            }
        }
    }

    point tmp = a->at(0);
    a->at(0) = a->at(m);
    a->at(m) = tmp;
    m = 1;
    quickSort(a, 1, a->size() - 1);
    a->erase(a->begin());
    for (size_t i = 1; i < a->size(); i++) {
        if (notEquals(a->at(i), a->at(m))) {
            if (m >= 1) {
                while (m >= 1 && determinant(a->at(m - 1), a->at(m),
                    a->at(i)) >= 0) {
                    m = m - 1;
                }
            }
            m = m + 1;
            point tmp = a->at(m);
            a->at(m) = a->at(i);
            a->at(i) = tmp;
        }
    }
    return m + 1;
}

int grahamScanParallel(std::vector<point> *a) {
    point c = a->at(0);
    int m = 0;
    for (size_t i = 1; i < a->size(); i++) {
        if (a->at(i).x < c.x) {
            c = a->at(i);
            m = i;
        } else if (a->at(i).x == c.x) {
            if (a->at(i).y < c.y) {
                c = a->at(i);
                m = i;
            }
        }
    }

    point tmp = a->at(0);
    a->at(0) = a->at(m);
    a->at(m) = tmp;
    m = 1;
    tbb::parallel_sort(*a);
    for (size_t i = 1; i < a->size(); i++) {
        if (notEquals(a->at(i), a->at(m))) {
            if (m >= 1) {
                while (m >= 1 && determinant(a->at(m - 1), a->at(m),
                    a->at(i)) >= 0) {
                    m = m - 1;
                }
            }
            m = m + 1;
            point tmp = a->at(m);
            a->at(m) = a->at(i);
            a->at(i) = tmp;
        }
    }
    return m + 1;
}

int randomize(int value) {
    double min = 1;
    double max = 4;
    return value + (static_cast<int> (min) + std::rand() %
        (static_cast<int> (max) - static_cast<int> (min) + 1));
}

int main() {
    std::srand((unsigned)time(NULL));
    double numOfPoints = 10000;
    double min = 1;
    double max = 10000;
    std::vector<point>* data = new std::vector<point>;
    std::vector<point>* parallelData = new std::vector<point>;
    for (size_t i = 0; i < numOfPoints; i++) {
        point p;
        p.x = randomize(static_cast<int> (min) + std::rand() %
            (static_cast<int> (max) - static_cast<int> (min) + 1));
        p.y = randomize(static_cast<int> (min) + std::rand() %
            (static_cast<int> (max) - static_cast<int> (min) + 1));
        data->push_back(p);
        parallelData->push_back(p);
    }
    int num_threads = 4;
    tbb::task_scheduler_init init(num_threads);

    tbb::tick_count t1 = tbb::tick_count::now();
    int hullSize = grahamScanLinear(data);
    tbb::tick_count t2 = tbb::tick_count::now();
    double time = (t2 - t1).seconds();
    std::cout << std::endl << "linear time: " << time << std::endl;
    for (int i = 0; i < hullSize; i++) {
        std::cout << "( " << data->at(i).x << ", " << data->at(i).y << ") ";
    }

    tbb::tick_count t3 = tbb::tick_count::now();
    int size = grahamScanParallel(parallelData);
    tbb::tick_count t4 = tbb::tick_count::now();
    double parallelTime = (t4 - t3).seconds();
    std::cout << std::endl <<"parallel time: " << parallelTime << std::endl;
    for (int i = 0; i < size; i++) {
        std::cout << "( " << parallelData->at(i).x <<
            ", " << parallelData->at(i).y << ") ";
    }
    std::cout << std::endl;

    delete data;
    delete parallelData;
    return 0;
}

