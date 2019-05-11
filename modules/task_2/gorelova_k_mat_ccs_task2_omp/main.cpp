//  Copyright 2019 Gorelova Ksenia

#include <omp.h>
#include <iostream>
#include <vector>
#include <random>
#include <cstring>
#include <ctime>

using std::vector;
struct ccsMatrix {
    int N;
    int NZ;
    double* val;
    int* row;
    int* ColIndex;
};

void initMatrix(ccsMatrix *mtx, int nz, int n) {
    mtx->NZ = nz;
    mtx->N = n;
    mtx->val = new double[nz];
    mtx->row = new int[nz];
    mtx->ColIndex = new int[n + 1];
}

void freeMatrix(ccsMatrix *mtx) {
    delete[] mtx->val;
    delete[] mtx->row;
    delete[] mtx->ColIndex;
}

void transMatrix(ccsMatrix A, ccsMatrix *AT) {
    int sum = 0;
    int index, colIn;
    double v;
    memset(AT->ColIndex, 0, (AT->N + 1) * sizeof(int));
    for (int i = 0; i < A.NZ; i++)
        AT->ColIndex[A.row[i] + 1]++;
    for (int i = 1; i <= A.N; i++) {
        int tmp = AT->ColIndex[i];
        AT->ColIndex[i] = sum;
        sum += tmp;
    }
    for (int i = 0; i < A.N; i++) {
        int row = i;
        for (int j = A.ColIndex[i]; j < A.ColIndex[i + 1]; j++) {
            v = A.val[j];
            colIn = A.row[j];
            index = AT->ColIndex[colIn + 1];
            AT->val[index] = v;
            AT->row[index] = row;
            AT->ColIndex[colIn + 1]++;
        }
    }
}

void generateMatrix(ccsMatrix *mtx, int n, int cntInCol) {
    int rows;
    bool exist;
    int nz = cntInCol * n;
    initMatrix(mtx, nz, n);
    for (int col = 0; col < n; col++) {
        for (int row = 0; row < cntInCol; row++) {
            do {
                rows = std::rand() % n;
                exist = false;
                for (int i = 0; i < row; i++)
                    if (rows == mtx->row[col*cntInCol + i])
                        exist = true;
                break;
            } while (exist == true);
            mtx->row[col*cntInCol + row] = rows;
        }
        for (int row = 0; row < cntInCol - 1; row++)
            for (int val = 0; val < cntInCol - 1; val++)
                if (mtx->row[col*cntInCol + val] >
                    mtx->row[col*cntInCol + val + 1]) {
                    int tmp = mtx->row[col*cntInCol + val];
                    mtx->row[col*cntInCol + val] =
                        mtx->row[col*cntInCol + val + 1];
                    mtx->row[col*cntInCol + val + 1] = tmp;
                }
    }
    for (int val = 0; val < nz; val++)
        mtx->val[val] = (std::rand() % 10000) / 1000.0f;
    int c = 0;
    for (int col = 0; col <= n; col++) {
        mtx->ColIndex[col] = c;
        c += cntInCol;
    }
}

void printMatrix(int n, ccsMatrix *mtx) {
    int i;
    int k = mtx[0].NZ;
    std::cout << "\n Value: ";
    for (i = 0; i < k; i++)
        std::cout << " " << mtx[0].val[i];
    std::cout << "\n Row: ";
    for (i = 0; i < k; i++)
        std::cout << " " << mtx[0].row[i];
    std::cout << "\n ColIndex: ";
    for (i = 0; i < n + 1; i++)
        std::cout << " " << mtx[0].ColIndex[i];
    std::cout << std::endl;
}

int multiply(ccsMatrix A, ccsMatrix B, ccsMatrix *C) {
    if (A.N != B.N)
        return 1;
    int N = A.N;
    vector<int> rows;
    vector<double> value;
    vector<int> columnIndex;

    int nz = 0;
    int *temp = new int[N];
    columnIndex.push_back(0);

    for (int i = 0; i < N; i++) {
        memset(temp, -1, N * sizeof(int));
        int ind1 = A.ColIndex[i], ind2 = A.ColIndex[i + 1];
        for (int j = ind1; j < ind2; j++) {
            int row = A.row[j];
            temp[row] = j;
        }
        for (int j = 0; j < N; j++) {
            double sum = 0;
            int ind3 = B.ColIndex[i], ind4 = B.ColIndex[i + 1];
            for (int k = ind3; k < ind4; k++) {
                int brow = B.row[k];
                int aind = temp[brow];
                if (aind != -1)
                    sum += A.val[aind] * B.val[k];
            }
            if (fabs(sum) > 0) {
                rows.push_back(j);
                value.push_back(sum);
                nz++;
            }
        }
        columnIndex.push_back(nz);
    }

    initMatrix(C, nz, N);
    for (int j = 0; j < nz; j++) {
        C->row[j] = rows[j];
        C->val[j] = value[j];
    }
    for (int i = 0; i <= N; i++)
        C->ColIndex[i] = columnIndex[i];
    return 0;
}

int multiplyOmp(ccsMatrix A, ccsMatrix B, ccsMatrix *C) {
    if (A.N != B.N)
        return 1;
    int N = A.N;
    vector<int> rows;
    vector<double> value;
    vector<int> columnIndex;
    columnIndex.push_back(0);
    int nz = 0;
#pragma omp parallel
    {
        int *temp = new int[N];
#pragma omp for
        for (int i = 0; i < N; i++) {
            memset(temp, -1, N * sizeof(int));
            int ind1 = A.ColIndex[i], ind2 = A.ColIndex[i + 1];
            for (int j = ind1; j < ind2; j++) {
                int row = A.row[j];
                temp[row] = j;
            }
            for (int j = 0; j < N; j++) {
                double sum = 0;
                int ind3 = B.ColIndex[i], ind4 = B.ColIndex[i + 1];
                for (int k = ind3; k < ind4; k++) {
                    int brow = B.row[k];
                    int aind = temp[brow];
                    if (aind != -1)
                        sum += A.val[aind] * B.val[k];
                }
                if (fabs(sum) > 0) {
                    rows.push_back(j);
                    value.push_back(sum);
                    nz++;
                }
            }
            columnIndex.push_back(nz);
        }
    }
    initMatrix(C, nz, N);
    for (int j = 0; j < nz; j++) {
        C->row[j] = rows[j];
        C->val[j] = value[j];
    }
    for (int i = 0; i <= N; i++)
        C->ColIndex[i] = columnIndex[i];
    return 0;
}

int main(int argc, char** argv) {
    omp_set_num_threads(2);
    int N = (argc != 1) ? atoi(argv[1]) : 10;
    int cntInCol = (argc != 1) ? atoi(argv[2]) : 5;
    ccsMatrix A, B, AT, C;
    generateMatrix(&A, N, cntInCol);
    initMatrix(&AT, A.NZ, A.N);
    transMatrix(A, &AT);
    freeMatrix(&A);
    generateMatrix(&B, N, cntInCol);
    initMatrix(&C, cntInCol, N);
    double startTime = omp_get_wtime();
    multiply(AT, B, &C);
    double finishTime = omp_get_wtime();
    std::cout << "Time usual is " << finishTime - startTime << std::endl;

    startTime = omp_get_wtime();
    multiplyOmp(AT, B, &C);
    finishTime = omp_get_wtime();
    std::cout << "Time omp is " << finishTime - startTime << std::endl;

    freeMatrix(&AT);
    freeMatrix(&B);
    freeMatrix(&C);
    return 0;
}
