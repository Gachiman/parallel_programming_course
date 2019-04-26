//  Copyright 2019 Churakov Sergey
#include <omp.h>
#include <iostream>
#include <iomanip>
#include <ctime>
#include <cstdlib>
#include <cmath>
#define step 1000
#define chunk 2500

void RandomizeArray(double* rarray, const int& size, const int& min = -50, const int& max = 50) {
    for (int i = 0; i < size; i++) {
        int part1 = std::rand();
        int part2 = part1 << (sizeof(int) );
        part2 |= std::rand();
        *(rarray + i) = part2 % static_cast<int>(max - min + 1) + min;
    }
}


void PrintPoints(double* X, double* Y, const int& Size) {
    for (int i = 0; i < Size; i++) {
        std::cout << std::setw(3) << X[i] << ", " << Y[i] << "; ";
    }
}


void ReinitEqPoints(double* X, double* Y, const int& Size, const int& min = -50, const int& max = 50) {
    for (int i = 0; i < Size - 1; i++)
        for (int j = i + 1; j < Size; j++) {
            if (X[i] == X[j] && Y[i] == Y[j]) {
                int flag = 1;
                while (flag) {
                    flag = 0;
                    X[i] = std::rand() % (max - min + 1) + min;
                    Y[i] = std::rand() % (max - min + 1) + min;
                    for (int k = 0; k < Size; k++) {
                        if (X[i] == X[k] && Y[i] == Y[k] && i != k) {
                            flag = 1;
                            break;
                        }
                    }
                }
                break;
            }
        }
}

int FindBLPoint(double* X, double* Y, const int& Size) {
    if (Size < 1) { return -1; }
    int pointInd = 0;
    for (int i = 1; i < Size; i++) {
        if (Y[i] < Y[pointInd]) {
            pointInd = i;
        } else {
            if (Y[i] == Y[pointInd] && X[i] < X[pointInd])
                pointInd = i;
        }
    }
    return pointInd;
}


int FindBLPointParallel(double* X, double* Y, const int& Size) {
    if (Size < 1) { return -1; }
    int pointInd = 0;
    int i;
    int threads = omp_get_max_threads();
    int* pointIndOwn = new int[threads];
    for (int i = 0; i < threads; i++) {
        pointIndOwn[i] = 0;
    }
    #pragma omp parallel private(i)
    {
        int cur_thread = omp_get_thread_num();
        #pragma omp for schedule(dynamic, chunk)
        for (i = 0; i < Size; i++) {
            if (Y[i] < Y[pointIndOwn[cur_thread]]) {
                pointIndOwn[cur_thread] = i;
            } else {
                if (Y[i] == Y[pointIndOwn[cur_thread]] && X[i] < X[pointIndOwn[cur_thread]])
                    pointIndOwn[cur_thread] = i;
            }
        }
    }
    pointInd = pointIndOwn[0];
    for (int i = 1; i < threads; i++) {
        if (Y[pointIndOwn[i]] < Y[pointInd]) {
            pointInd = pointIndOwn[i];
        } else {
            if (Y[pointIndOwn[i]] == Y[pointInd] && X[pointIndOwn[i]] < X[pointInd])
                pointInd = pointIndOwn[i];
        }
    }
    return pointInd;
}

int FindTRPoint(double* X, double* Y, const int& Size) {
    if (Size < 1) { return -1; }
    int pointInd = 0;
    for (int i = 1; i < Size; i++) {
        if (Y[i] > Y[pointInd]) {
            pointInd = i;
        } else {
            if (Y[i] == Y[pointInd] && X[i] > X[pointInd])
                pointInd = i;
        }
    }
    return pointInd;
}

double FindDist(const double& x1, const double& y1, const double& x2, const double& y2) {
    return sqrt((x1 - x2)*(x1 - x2) + (y1 - y2)*(y1 - y2));
}

double GetCos(const double& x1, const double& y1, const double& x2,
    const double& y2, const double& x3, const double& y3) {
    return ((x2 - x1)*(x3 - x1) + (y2 - y1)*(y3 - y1)) / (FindDist(x1, y1, x2, y2)*FindDist(x1, y1, x3, y3));
}


int FindPWithMinAngle(double* X, double* Y, const int& Size,
    const double& x1, const double& y1, const double& x2, const double& y2) {
    double maxCos = -1.5;
    int NextPointInd = -1;
    double tmp;
    for (int i = 0; i < Size; i++) {
        if (!((X[i] == x1 && Y[i] == y1) || (X[i] == x2 && Y[i] == y2))) {
            if (((x2 - x1)*(Y[i] - y2) - (X[i] - x2)*(y2 - y1)) >= 0) {
                tmp = (-1)*GetCos(x2, y2, x1, y1, X[i], Y[i]);
                if (tmp > maxCos) {
                    maxCos = tmp;
                    NextPointInd = i;
                }
            }
        }
    }
    return NextPointInd;
}

int FindPWithMinAngleParallel(double* X, double* Y, const int& Size,
    const double &x1, const double& y1, const double& x2, const double& y2) {
    int NextPointInd = -1;
    int i;
    double tmp;
    int threads = omp_get_max_threads();
    int* NextPointIndOwn = new int[threads];
    double* maxCosOwn = new double[threads];
    for (int i = 0; i < threads; i++) {
        NextPointIndOwn[i] = -1;
        maxCosOwn[i] = -1.5;
    }
    #pragma omp parallel private(i, tmp)
    {
        int cur_thread = omp_get_thread_num();
        #pragma omp for schedule(dynamic, chunk)
        for (i = 0; i < Size; i++) {
            if (!((X[i] == x1 && Y[i] == y1) || (X[i] == x2 && Y[i] == y2))) {
                if (((x2 - x1)*(Y[i] - y2) - (X[i] - x2)*(y2 - y1)) >= 0) {
                    tmp = (-1)*GetCos(x2, y2, x1, y1, X[i], Y[i]);
                    if (tmp > maxCosOwn[cur_thread]) {
                        maxCosOwn[cur_thread] = tmp;
                        NextPointIndOwn[cur_thread] = i;
                    }
                }
            }
        }
    }
    tmp = maxCosOwn[0];
    NextPointInd = NextPointIndOwn[0];
    for (int i = 1; i < threads; i++) {
        if (tmp < maxCosOwn[i]) {
            tmp = maxCosOwn[i];
            NextPointInd = NextPointIndOwn[i];
        }
    }
    return NextPointInd;
}

void ElimPointsOnLines(double* X, double* Y, int* Envelope, int* Size) {
    int i = 1;
    while (i != *Size) {
        if (static_cast<double>(static_cast<double>(X[Envelope[i]] - X[Envelope[i - 1]])*
            static_cast<double>(Y[Envelope[i + 1]] - Y[Envelope[i]])) ==
            static_cast<double>(static_cast<double>(X[Envelope[i + 1]] - X[Envelope[i]])*
                static_cast<double>(Y[Envelope[i]] - Y[Envelope[i - 1]]))) {
            int j = 0;
            while (i + j < *Size) {
                Envelope[i + j] = Envelope[i + j + 1];
                j++;
            }
            *Size = *Size - 1;
            i--;
        }
        i++;
    }
}

int main(int argc, char* argv[]) {
    double times = 0, time_part = 0, time_part_fin = 0, timef = 0;
    double time_part_seq = 0, time_part_seq_fin = 0;
    times = omp_get_wtime();
    srand((unsigned int)time(NULL));
    int Size = 100;
    if (argc < 1 || argc > 5)
        return 1;
    if (argc > 1) {
        Size = atol(argv[1]);
        if (Size < 1)
            return 1;
    }
    double* X_coord = new double[Size];
    double* Y_coord = new double[Size];
    if (argc == 4 || argc == 5) {
        int minRand, maxRand;
        minRand = atol(argv[2]);
        maxRand = atol(argv[3]);
        if (maxRand < minRand)
            return 1;
        RandomizeArray(X_coord, Size, minRand, maxRand);
        RandomizeArray(Y_coord, Size, minRand, maxRand);
        //        if (Size >(maxRand - minRand + 1)*(maxRand - minRand + 1))
        //            return 1;
        //        ReinitEqPoints(X_coord, Y_coord, Size, minRand, maxRand);
    } else {
        RandomizeArray(X_coord, Size);
        RandomizeArray(Y_coord, Size);
        //        if (Size > 101 * 101)
        //            return 1;
        //        ReinitEqPoints(X_coord, Y_coord, Size);
    }
    if (argc == 3) {
        int numt = atoi(argv[2]);
        if (numt > 0 && numt < 65) {
            omp_set_num_threads(numt);
        } else {
            return 1;
        }
    }
    if (argc == 5) {
        int numt = atoi(argv[4]);
        if (numt > 0 && numt < 65) {
            omp_set_num_threads(numt);
        } else {
            return 1;
        }
    }
//  PrintPoints(X_coord, Y_coord, Size);
    std::cout << std::endl;
    std::cout << omp_get_max_threads() << " threads are working" << std::endl;
    if (Size == 1) {
        std::cout << "Result chain of points is a single point:" << std::endl;
        std::cout << std::setw(3) << X_coord[0] << ", " << Y_coord[0] << "; ";
    } else {
        if (Size == 2) {
            std::cout << "Result chain of points is" << std::endl;
            std::cout << std::setw(3) << X_coord[0] << ", " << Y_coord[0] << "; ";
            std::cout << std::setw(3) << X_coord[1] << ", " << Y_coord[1] << "; ";
        } else {
            int dynsize = step;
            int* Envelope = static_cast<int*>(malloc(sizeof(int) * dynsize));
            int PNum = 1;
            time_part = omp_get_wtime();
            int FirstPoint = FindBLPointParallel(X_coord, Y_coord, Size);
            Envelope[0] = FirstPoint;
            Envelope[1] = FindPWithMinAngleParallel(X_coord, Y_coord, Size, X_coord[FirstPoint] - 1,
                Y_coord[FirstPoint], X_coord[FirstPoint], Y_coord[FirstPoint]);
            if (Envelope[1] == -1) {
                std::cout << "Result may be a single point or error" << std::endl;
                std::cout << "Result point may be: ";
                std::cout << X_coord[Envelope[0]] << " " << Y_coord[Envelope[0]] << std::endl;
                free(Envelope);
                delete[] X_coord;
                delete[] Y_coord;
                return 1;
            }
            while (Envelope[PNum] != FirstPoint && ((X_coord[FirstPoint] != X_coord[Envelope[PNum]])
                ||(Y_coord[FirstPoint] != Y_coord[Envelope[PNum]]))) {
                PNum++;
                if (PNum == dynsize) {
                    dynsize += step;
                    Envelope = static_cast<int*>(realloc(Envelope, sizeof(int) * (dynsize)));
                }
                Envelope[PNum] = FindPWithMinAngleParallel(X_coord, Y_coord, Size, X_coord[Envelope[PNum - 2]],
                    Y_coord[Envelope[PNum - 2]], X_coord[Envelope[PNum - 1]], Y_coord[Envelope[PNum - 1]]);
            }
            ElimPointsOnLines(X_coord, Y_coord, Envelope, &PNum);
            time_part_fin = omp_get_wtime();
            dynsize = step;
//          Sequential implementation
            int* EnvelopeForCheck = static_cast<int*>(malloc(sizeof(int) * dynsize));
            int PNumForCheck = 1;
            int Correct = 1;
            time_part_seq = omp_get_wtime();
            int FirstPointForCheck = FindBLPoint(X_coord, Y_coord, Size);
            EnvelopeForCheck[0] = FirstPointForCheck;
            EnvelopeForCheck[1] = FindPWithMinAngle(X_coord, Y_coord, Size, X_coord[FirstPointForCheck] - 1,
                Y_coord[FirstPointForCheck], X_coord[FirstPointForCheck], Y_coord[FirstPointForCheck]);
            if (Envelope[1] == -1) {
                std::cout << "Result may be a single point or error" << std::endl;
                std::cout << "Result point may be: ";
                std::cout << X_coord[Envelope[0]] << " " << Y_coord[Envelope[0]] << std::endl;
                free(EnvelopeForCheck);
                free(Envelope);
                delete[] X_coord;
                delete[] Y_coord;
                return 1;
            }
            while (EnvelopeForCheck[PNumForCheck] != FirstPointForCheck && Correct &&
                ((X_coord[FirstPointForCheck] != X_coord[EnvelopeForCheck[PNumForCheck]])
                || (Y_coord[FirstPointForCheck] != Y_coord[EnvelopeForCheck[PNumForCheck]]))) {
                PNumForCheck++;
                if (PNumForCheck == dynsize) {
                    dynsize += step;
                    EnvelopeForCheck = static_cast<int*>(realloc(EnvelopeForCheck, sizeof(int) * (dynsize)));
                }
                EnvelopeForCheck[PNumForCheck] = FindPWithMinAngle(X_coord, Y_coord, Size,
                    X_coord[EnvelopeForCheck[PNumForCheck - 2]],
                    Y_coord[EnvelopeForCheck[PNumForCheck - 2]],
                    X_coord[EnvelopeForCheck[PNumForCheck - 1]],
                    Y_coord[EnvelopeForCheck[PNumForCheck - 1]]);
            }
            ElimPointsOnLines(X_coord, Y_coord, EnvelopeForCheck, &PNumForCheck);
            time_part_seq_fin = omp_get_wtime();
            if (PNum != PNumForCheck) {
                Correct = 0;
            } else {
                for (int i = 0; i < PNum; i++)
                    if (X_coord[Envelope[i]] != X_coord[EnvelopeForCheck[i]] &&
                        Y_coord[Envelope[i]] != Y_coord[EnvelopeForCheck[i]]) {
                        Correct = 0;
                        break;
                    }
            }
            if (Correct)
                std::cout << "Results checked successfully" << std::endl;
            else
                std::cout << "Results dont match" << std::endl;
            if (PNum < 3) {
                std::cout << "Result chain of points is a line" << std::endl;
                int point;
                point = FindBLPoint(X_coord, Y_coord, Size);
                std::cout << std::setw(3) << X_coord[point] << ", " << Y_coord[point] << "; ";
                point = FindTRPoint(X_coord, Y_coord, Size);
                std::cout << std::setw(3) << X_coord[point] << ", " << Y_coord[point] << "; ";
            } else {
                std::cout << "Result chain of points is" << std::endl;
                for (int i = 0; i < PNum; i++) {
                    std::cout << std::setw(3) << X_coord[Envelope[i]] << ", " << Y_coord[Envelope[i]] << "; ";
                }
            }
            timef = omp_get_wtime();
            std::cout << std::endl;
            std::cout << "time: " << timef - times << std::endl;
            std::cout << "time without initialisation and preparations: " << time_part_fin - time_part << std::endl;
            std::cout << "time of sequential execution of the same task: " << time_part_seq_fin - time_part_seq;
            std::cout << std::endl;
            double acc = (time_part_seq_fin - time_part_seq) / (time_part_seq - time_part);
            std::cout << "Acceleration: " << acc << std::endl;
            std::cout << "Efficiency: " << acc / omp_get_max_threads() << std::endl;
            free(Envelope);
            delete[] EnvelopeForCheck;
        }
    }
    delete[] X_coord;
    delete[] Y_coord;
    return 0;
}
