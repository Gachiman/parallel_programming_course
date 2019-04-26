//  Copyright 2019 Senina Anastasia
#include <tbb/tbb.h>
#include <iostream>
#include <vector>
#include <random>
#include <algorithm>



struct ccsMatrix {
    std::vector<double> value;
    std::vector<size_t> row;
    std::vector<size_t> indexCol;
    ccsMatrix(size_t N, size_t countNZ);
    explicit ccsMatrix(std::vector<std::vector<double>>);
    ccsMatrix() {}
    bool operator==(const ccsMatrix& B);
};

bool ccsMatrix::operator==(const ccsMatrix& B) {
    return (value == B.value) && (row == B.row) && (indexCol == B.indexCol);
}

ccsMatrix::ccsMatrix(size_t N, size_t countNZ) {
    value.resize(countNZ);
    row.resize(countNZ);
    indexCol.resize(N + 1);
}

ccsMatrix::ccsMatrix(std::vector<std::vector<double>> matrix) {
    size_t n = matrix.size();
    int countNZ = 0;
    for (size_t j = 0; j < n; ++j) {
        indexCol.push_back(value.size());
        for (size_t i = 0; i < n; ++i) {
            if (matrix[i][j] != 0) {
                value.push_back(matrix[i][j]);
                row.push_back(i);
                countNZ++;
            }
        }
    }
    indexCol.push_back(countNZ);
}

std::vector<std::vector<double>> generateMatrix(size_t N) {
    std::vector<std::vector<double>> matrix;
    std::default_random_engine generator;
    std::uniform_real_distribution<double> distribution(0.0, 10.0);
    for (size_t i = 0; i < N; ++i) {
        std::vector<double> row;
        for (size_t j = 0; j < N; ++j) {
            double v = distribution(generator);
            if (v > 2.5) {
                row.push_back(0);
            } else {
                row.push_back(v);
            }
        }
        matrix.push_back(row);
    }
    return matrix;
}


ccsMatrix transpon(const ccsMatrix& A) {
    ccsMatrix AT(A.indexCol.size() - 1, A.value.size());
    int r = 0;
    for (size_t i = 0; i < A.indexCol.size() - 1; ++i) {
        AT.indexCol[i] = r;
        for (size_t j = 0; j < A.row.size(); ++j) {
            if (A.row[j] == i) {
                AT.value[r] = A.value[j];
                size_t k = 0;
                for (; k < A.indexCol.size(); ++k)
                    if (j < A.indexCol[k]) {
                        --k;
                        break;
                    }
                AT.row[r] = k;
                r++;
            }
        }
    }
    *(AT.indexCol.end() - 1) = *(A.indexCol.end() - 1);
    return AT;
}

void mulccsMatrix(const ccsMatrix& A, const ccsMatrix& B, ccsMatrix* resultC) {
    tbb::tick_count t1 = tbb::tick_count::now();
    ccsMatrix AT = transpon(A);
    tbb::tick_count t2 = tbb::tick_count::now();
    std::cout << " time transpose : " << (t2 - t1).seconds() << std::endl;
    size_t m = B.indexCol.size(), n = AT.indexCol.size();
    size_t r = 0;
    for (size_t j = 0; j < m - 1; ++j) {
        for (size_t i = 0; i < n - 1; ++i) {
            double sum = 0;
            size_t x = AT.indexCol[i + 1], y = B.indexCol[j + 1];
            for (size_t k = AT.indexCol[i]; k < x; ++k) {
                for (size_t l = B.indexCol[j]; l < y; ++l) {
                    if (AT.row[k] == B.row[l]) {
                        sum += AT.value[k] * B.value[l];
                        break;
                    }
                }
            }
            if (sum != 0) {
                resultC->value.push_back(sum);
                resultC->row.push_back(i);
                resultC->indexCol[j + 1]++;
                ++r;
            }
        }
        resultC->indexCol[j + 1] += resultC->indexCol[j];
    }
}
void mulccsMatrixParallelVersion(const ccsMatrix& A,
                                 const ccsMatrix& B,
                                 ccsMatrix* C) {
    size_t size = A.indexCol.size() - 1;
    std::vector< std::vector<double>> values(size);
    std::vector<std::vector<size_t>> row(size);
    std::vector<size_t> index_col(size + 1);

    tbb::tick_count t1 = tbb::tick_count::now();
    ccsMatrix AT = transpon(A);
    tbb::tick_count t2 = tbb::tick_count::now();
    std::cout << " time transpose : " << (t2 - t1).seconds() << std::endl;
    size_t m = B.indexCol.size(), n = AT.indexCol.size();

    tbb::parallel_for(0, static_cast<int>(m - 1), [&](size_t j) {
        size_t i = 0, x = 0, y = 0, l = 0, k = 0;
        for ( ; i < n - 1; ++i) {
            double sum = 0;
            x = AT.indexCol[i + 1], y = B.indexCol[j + 1];
            for (k = AT.indexCol[i]; k < x; ++k) {
                for (l = B.indexCol[j]; l < y; ++l) {
                    if (AT.row[k] == B.row[l]) {
                        sum += AT.value[k] * B.value[l];
                        break;
                    }
                }
            }
            if (sum != 0) {
                values[j].push_back(sum);
                row[j].push_back(i);
                index_col[j]++;
            }
        }
    });
    size_t NZ = 0;
    for (size_t i = 0; i < size; ++i) {
        size_t p = index_col[i];
        index_col[i] = NZ;
        NZ += p;
    }
    C->value.resize(NZ);
    C->row.resize(NZ);
    C->indexCol.resize(size + 1);
    std::vector<std::vector<double>>::iterator itv = values.begin();
    std::vector<std::vector<size_t>>::iterator itr = row.begin();
    std::vector<double>::iterator itCv = C->value.begin();
    std::vector<size_t>::iterator itCr = C->row.begin();
    size_t count;
    for ( ; itv != values.end(); ++itv, ++itr) {
        count = (*itr).size();
        std::copy(itv->begin(), itv->end(), itCv);
        std::copy(itr->begin(), itr->end(), itCr);
        itCr += count;
        itCv += count;
    }
    index_col[size] = NZ;
    std::copy(index_col.begin(), index_col.end(), C->indexCol.begin());
}
std::vector<std::vector<double>> mulMatrix(std::vector<std::vector<double>> A,
    std::vector<std::vector<double>> B) {
    size_t N = A.size();
    std::vector<std::vector<double>> C(N);

    for (size_t i = 0; i < N; ++i) {
        for (size_t j = 0; j < N; ++j) {
            C[i].push_back(0);
            for (size_t k = 0; k < N; ++k)
                C[i][j] += A[i][k] * B[k][j];
        }
    }
    return C;
}

int main(int argc, char** argv) {
    //  tbb::task_scheduler_init(4);
    size_t size = 50;
    if (argc == 2) {
        size = atoi(argv[1]);
    }
    std::vector<std::vector<double>> A = generateMatrix(size);
    std::cout << " matrix A was generated " << std::endl;
    ccsMatrix ccsA(A);
    std::vector<std::vector<double>> B = generateMatrix(size);
    std::cout << " matrix B was generated " << std::endl;
    ccsMatrix ccsB(B);

    std::cout << " start A * B .." << std::endl;
    tbb::tick_count t1 = tbb::tick_count::now();
    std::vector<std::vector<double>> C = mulMatrix(A, B);
    tbb::tick_count t2 = tbb::tick_count::now();
    std::cout << " time A * B : " << (t2 - t1).seconds() << std::endl;

    ccsMatrix serial_C(size, 0);
    std::cout << " start ccsA * ccsB .." << std::endl;
    t1 = tbb::tick_count::now();
    mulccsMatrix(ccsA, ccsB, &serial_C);
    t2 = tbb::tick_count::now();
    std::cout << " time ccsA * ccsB : " << (t2 - t1).seconds() << std::endl;

    std::cout << " start  parallel ccsA * ccsB .." << std::endl;

    ccsMatrix parallel_C(size, 0);
    t1 = tbb::tick_count::now();
    mulccsMatrixParallelVersion(ccsA, ccsB, &parallel_C);
    t2 = tbb::tick_count::now();
    std::cout << " time parallel ccsA * ccsB : " << (t2 - t1).seconds()
              << std::endl;

    std::cout << " NZ ccsA :" << ccsA.value.size() << std::endl;
    std::cout << " NZ ccsB :" << ccsB.value.size() << std::endl;
    std::cout << " NZ ccsC :" << parallel_C.value.size() << std::endl;
    ccsMatrix ccsCtrue(C);
    std::cout << (ccsCtrue == parallel_C) << std::endl;
    return EXIT_SUCCESS;
}
