// Copyright 2019 Kuznetsov Konstantin
// параллельная поразрядная сортировка целых положительных чисел
// сортировка разными потоками со слиянием "Разделяй и властвуй"

#include <tbb/tbb.h>

#include <algorithm>
#include <iostream>
#include <limits>
#include <cmath>
#include <random>
#include <vector>
#include <utility>

void print_data(const std::vector<unsigned int>& data, size_t n = 10) {
    if (data.size() < n) return;

    for (size_t i = 0; i < n; ++i)
        std::cout << data[i] << " ";
    std::cout << std::endl;
}

static std::vector<unsigned int> generate_data(size_t size) {
    using value_type = unsigned int;

    static std::uniform_int_distribution<value_type> distribution(
                (std::numeric_limits<value_type>::min)(),
                (std::numeric_limits<value_type>::max)());
    static std::default_random_engine generator;

    std::vector<value_type> data(size);
    std::generate(data.begin(), data.end(), []() {
        return distribution(generator); });

    return data;
}

void create_counters(unsigned int* data, unsigned int* counters,
                     size_t size) {
    unsigned char* byte_ptr = reinterpret_cast<unsigned char*>(data);
    unsigned char* data_end = reinterpret_cast<unsigned char*>(data + size);
    for (size_t i = 0; i < sizeof(int) * 256; ++i)
        counters[i] = 0;

    size_t i;
    while (byte_ptr != data_end)
        for (i = 0; i < sizeof(int); ++i)
            counters[256 * i + *byte_ptr++]++;
}

void byte_sort(unsigned int* data, unsigned int* temp,
               unsigned int* counter, size_t byte, size_t size) {
    unsigned char* byte_ptr;
    unsigned int c = 0;

    unsigned int sum = 0;
    for (size_t i = 0; i < 256; ++i) {
        c = counter[i];
        counter[i] = sum;
        sum += c;
    }

    byte_ptr = reinterpret_cast<unsigned char*>(data) + byte;
    for (size_t i = 0; i < size; ++i, byte_ptr += sizeof(int))
        temp[counter[*byte_ptr]++] = data[i];
}

// поразрядная сортировка целых положительных чисел
// сортировка выполняется по байтам
// для сортировки по каждому байту используется сортировка подсчетом
void radix_sort(unsigned int* data, size_t size) {
    unsigned int* temp = new unsigned int[size];
    unsigned int* counters = new unsigned int[sizeof(int) * 256];
    unsigned int* counter;

    create_counters(data, counters, size);
    for (size_t i = 0; i < sizeof(int); ++i) {
        counter = counters + 256 * i;
        byte_sort(data, temp, counter, i, size);
        std::swap(data, temp);
    }

    delete[] temp;
    delete[] counters;
}

class merge : public tbb::task {
 private:
    unsigned int* a;
    unsigned int* b;
    unsigned int* c;
    size_t n;
    size_t m;

 public:
    merge(unsigned int* _a, unsigned int* _b, unsigned int* _c,
               size_t _n, size_t _m): a(_a), b(_b), c(_c), n(_n), m(_m) { }

    tbb::task* execute() {
        size_t i, j, k;
        i = j = k = 0;

        while (i < n && j < m) {
            if ((a[i]) < (b[j]))
                c[k++] = a[i++];
            else
                c[k++] = b[j++];
        }

        while (i < n) c[k++] = a[i++];
        while (j < m) c[k++] = b[j++];

        return nullptr;
    }
};

size_t bin_search(unsigned int* data, size_t l, size_t r, unsigned int x) {
    if (l == r) return l;
    if (l + 1 == r) {
        if (x < data[l]) return l;
        else
            return r;
    }

    size_t m = (l + r) / 2;
    if (x < data[m]) {
        r = m;
    } else {
        if (x > data[m]) l = m;
        else
            return m;
    }

    return bin_search(data, l, r, x);
}

class tbb_sorter : public tbb::task {
 private:
    unsigned int* data;
    unsigned int* temp;
    size_t size;
    size_t portion;
    size_t num_threads;

 public:
    tbb_sorter(unsigned int* _data, unsigned int* _temp, size_t _size,
               size_t _portion, size_t _num_threads): data(_data),
    temp(_temp), size(_size), portion(_portion), num_threads(_num_threads) { }

    task* execute() {
        if (size <= portion) {
            radix_sort(data, size);
        } else {
            tbb_sorter& sorter1 = *new (allocate_child())
                    tbb_sorter(data, temp, size/2, portion, num_threads/2);

            tbb_sorter& sorter2 = *new (allocate_child())
                    tbb_sorter(data + size/2, temp + size/2,
                               size - size/2, portion, num_threads/2);

            set_ref_count(3);
            spawn(sorter1);
            spawn_and_wait_for_all(sorter2);

            merge** merge_threads = new merge*[num_threads - 1];
            size_t s = portion;
            s /= num_threads;
            size_t l1 = 0;
            size_t r1 = s;
            size_t l2 = 0;
            size_t r2 = 0;;

            for (size_t i = 0; i < num_threads - 1; ++i) {
                unsigned x = data[r1];
                r2 = bin_search(data + size/2, 0, size - size/2, x);
                merge_threads[i] = new (allocate_child())
                        merge(data + l1, data + size/2 + l2,
                              temp + l1 + l2, r1 - l1, r2 - l2);
                l1 += s;
                r1 += s;
                l2 = r2;
            }

            merge &merger = *new (allocate_child())
                    merge(data + l1, data + size/2 + l2,
                          temp + l1 + l2, size/2 - l1, size - size/2 - l2);

            set_ref_count(static_cast<int>(num_threads) + 1);
            for (size_t i = 0; i < num_threads - 1; i++)
                spawn(*(merge_threads[i]));

            spawn_and_wait_for_all(merger);
            for (size_t i = 0; i < size; ++i)
                data[i] = temp[i];

            delete[] merge_threads;
        }

        return nullptr;
    }
};

void tbb_radix_sort(unsigned int* data, size_t size, size_t num_threads) {
    size_t portion = size / num_threads;
    if (size % num_threads != 0)
        portion++;

    unsigned int* temp = new unsigned int[size];

    tbb_sorter& sorter = *new (tbb::task::allocate_root())
            tbb_sorter(data , temp, size, portion, num_threads);
    tbb::task::spawn_root_and_wait(sorter);

    delete[] temp;
}

bool check_sorting(unsigned int *data1, unsigned int *data2, size_t size) {
    for (size_t i = 0; i < size - 1; ++i) {
        if ((data1[i] > data1[i + 1]) || (data1[i] != data2[i]))
            return false;
    }

    if (data1[size - 1] != data2[size - 1])
        return false;

    return true;
}

int main(int argc, char* argv[]) {
    size_t data_size = 160000;
    size_t num_threads = 4;
    tbb::tick_count t1, t2;

    std::vector<unsigned int> data1 = generate_data(data_size);
    std::vector<unsigned int> data2 = data1;

    t1 = tbb::tick_count::now();
    tbb_radix_sort(data1.data(), data_size, 1);
    t2 = tbb::tick_count::now();
    std::cout << "non-parallel time = " << (t2 - t1).seconds() << std::endl;

    t1 = tbb::tick_count::now();
    tbb_radix_sort(data2.data(), data_size, num_threads);
    t2 = tbb::tick_count::now();
    std::cout << "parallel time (" << num_threads << " threads) = " <<
                 (t2 - t1).seconds() << std::endl;

    if (check_sorting(data1.data(), data2.data(), data_size))
        std::cout << "correct" << std::endl;
    print_data(data1);
    print_data(data2);
    return 0;
}
