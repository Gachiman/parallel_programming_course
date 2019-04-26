// Copyright 2019 Muravev Denis
// Hoar sorting with simple merge
#include <tbb/task_group.h>
#include <tbb/task_scheduler_init.h>
#include <tbb/tick_count.h>
#include <ctime>
#include <iostream>
#include <climits>
#include <cstring>
#define TYPE_MAS double
#define TYPE_SIZE int

TYPE_MAS* Create_mas(TYPE_SIZE _size) {
    if (_size < 1)
        return NULL;
    TYPE_MAS* _mas = new TYPE_MAS[_size];
    TYPE_SIZE left = 0, right = INT_MAX;
    TYPE_SIZE i;
    for (i = 0; i < _size; i++)
        _mas[i] = std::rand() % (left - right + 1) + left;
    return  _mas;
}

void Show_mas(TYPE_MAS* _mas, TYPE_SIZE _size) {
    if (_mas == NULL || _size < 1)
        return;
    TYPE_SIZE i;
    int delta = 7;
    if (_size < 50) {
        std::cout << "Array [" << _size << "]" << std::endl;
        for (i = 0; i < _size; i++) {
            std::cout.width(delta);
            std::cout << _mas[i];
        }
    }
}

int Check_not_decreasing(TYPE_MAS* _mas, TYPE_SIZE _size) {
    TYPE_SIZE i;
    for (i = 1; i < _size; i++)
        if (_mas[i - 1] > _mas[i])
            return 0;
    return 1;
}

void Quick_sort(TYPE_MAS * mas, TYPE_SIZE l, TYPE_SIZE r) {
    TYPE_SIZE i = l, j = r;
    TYPE_MAS e, t;
    e = mas[(r + l) / 2];

    while (i <= j) {
        while (mas[i] < e)
            i++;
        while (mas[j] > e)
            j--;

        if (i <= j) {
            if (i < j) {
                t = mas[i];
                mas[i] = mas[j];
                mas[j] = t;
            }
            i++;
            j--;
        }
    }

    if (j > l)
        Quick_sort(mas, l, j);
    if (r > i)
        Quick_sort(mas, i, r);
}

int* Create_reverse_notation(int _n, int _k) {
    int higher = -1;
    int* _rev = new int[_n];
    _rev[0] = 0;
    for (int i = 1; i < _n; i++) {
        if ((i & (i - 1)) == 0)
            higher++;
        _rev[i] = _rev[i ^ (1 << higher)];
        _rev[i] |= 1 << (_k - higher - 1);
    }
    return _rev;
}

struct BorderList {
    TYPE_SIZE l = 0;
    TYPE_SIZE r = 0;
};

BorderList* Create_border_list(TYPE_SIZE _size, int _num_thr, int _n, int _k) {
    BorderList* _border = new BorderList[_num_thr];
    TYPE_SIZE delta = _size / _num_thr;
    int *rev = Create_reverse_notation(_n, _k);

    int k = 0;
    for (int i = 0; i < _n; i++) {
        if (rev[i] >= _num_thr)
            continue;
        _border[rev[i]].l = k * delta;
        k++;
        _border[rev[i]].r = k * delta - 1;
    }

    if (_n == _num_thr)
        _border[_n - 1].r = _size - 1;
    else
        _border[rev[_n - 2]].r = _size - 1;

    delete[] rev;
    return _border;
}

void Merge(TYPE_MAS* mas, TYPE_MAS* tmp_mas, TYPE_SIZE l1, TYPE_SIZE r1, TYPE_SIZE l2, TYPE_SIZE r2) {
    TYPE_SIZE i = l1, j = l2, k = l1;
    while ((i <= r1) && (j <= r2)) {
        if (mas[i] < mas[j])
            tmp_mas[k++] = mas[i++];
        else
            tmp_mas[k++] = mas[j++];
    }
    if (i > r1) {
        for (; j <= r2; j++)
            tmp_mas[k++] = mas[j];
    } else {
        for (; i <= r1; i++)
            tmp_mas[k++] = mas[i];
    }
    for (i = l1; i <= r2; i++)
        mas[i] = tmp_mas[i];
}

class TaskSort {
 private:
    TYPE_MAS* mas;
    TYPE_SIZE l;
    TYPE_SIZE r;
 public:
    TaskSort(TYPE_MAS* _mas, TYPE_SIZE _l, TYPE_SIZE _r) : mas(_mas), l(_l), r(_r) {}
    void operator()() const {
        Quick_sort(mas, l, r);
    }
};

class TaskMerge {
 private:
    TYPE_MAS* mas;
    TYPE_MAS* tmp_mas;
    TYPE_SIZE l1;
    TYPE_SIZE r1;
    TYPE_SIZE l2;
    TYPE_SIZE r2;
 public:
    TaskMerge(TYPE_MAS* _mas, TYPE_MAS* _tmp_mas, TYPE_SIZE _l1, TYPE_SIZE _r1, TYPE_SIZE _l2, TYPE_SIZE _r2)
        : mas(_mas), tmp_mas(_tmp_mas), l1(_l1), r1(_r1), l2(_l2), r2(_r2) {}
    void operator()() const {
        Merge(mas, tmp_mas, l1, r1, l2, r2);
    }
};

void Task_sort_tbb(TYPE_MAS* _mas, TYPE_SIZE _size, TYPE_MAS* _tmp_mas, int _num_thr) {
    tbb::task_scheduler_init init(_num_thr);
    int n = 1;
    int k = 0;
    while (n < _num_thr) {
        n = n << 1;
        k++;
    }
    BorderList* border = Create_border_list(_size, _num_thr, n, k);


    tbb::task_group tg;
    for (int i = 0; i < _num_thr; i++) {
        tg.run(TaskSort(_mas, border[i].l, border[i].r));
    }
    tg.wait();


    int max_rank = n >> 1;
    int source;
    if (k > 0) {
        for (int rank = 0; rank < max_rank; rank++) {
            source = rank ^ max_rank;
            if (source < _num_thr) {
                tg.run(TaskMerge(_mas, _tmp_mas, border[rank].l, border[rank].r, border[source].l, border[source].r));
                border[rank].r = border[source].r;
            }
        }
    }
    for (int i = k - 2; i >= 0; i--) {
        tg.wait();
        max_rank = max_rank >> 1;
        for (int rank = 0; rank < max_rank; rank++) {
            source = rank ^ (1 << i);
            tg.run(TaskMerge(_mas, _tmp_mas, border[rank].l, border[rank].r, border[source].l, border[source].r));
            border[rank].r = border[source].r;
        }
    }
    tg.wait();
}

int main(int argc, char * argv[]) {
    TYPE_MAS* mas = NULL;
    TYPE_SIZE size = (1 << 20);
    int num_thr = 4;


    if (argc == 3 || argc == 5) {
        if (strcmp(argv[1], "-size") == 0)
            size = atoi(argv[2]);
        if (strcmp(argv[1], "-n") == 0)
            num_thr = atoi(argv[2]);
        if (argc == 5) {
            if (strcmp(argv[3], "-size") == 0)
                size = atoi(argv[4]);
            if (strcmp(argv[3], "-n") == 0)
                num_thr = atoi(argv[4]);
        }
    } else {
        std::cout << "Initial data is missing or entered incorrectly" << std::endl;
    }


    std::srand((unsigned)time(NULL));
    mas = Create_mas(size);
    if (mas == NULL) {
        std::cout << "Error! Incorrect input data for array";
        return -1;
    }
    TYPE_MAS* tmp_mas = new TYPE_MAS[size];


    tbb::tick_count time_start = tbb::tick_count::now();
    if (size < 2 * num_thr) {
        std::cout << "Size array: " << size << "; Numb Threads: 1" << std::endl;
        Quick_sort(mas, 0, size - 1);
    } else {
        std::cout << "Size array: " << size << "; Numb Threads: " << num_thr << std::endl;
        Task_sort_tbb(mas, size, tmp_mas, num_thr);
    }
    tbb::tick_count time_end = tbb::tick_count::now();

    std::cout << "Spend time algorithm (tbb parallel version Hoar sorting): "
        << (time_end - time_start).seconds() << "sec" << std::endl;
    if (Check_not_decreasing(mas, size))
        std::cout << "Array is sorted by not decreasing" << std::endl;
    else
        std::cout << "Array isn't sorted by not decreasing" << std::endl;
    Show_mas(mas, size);

    delete[] mas;
    delete[] tmp_mas;
    return 0;
}
