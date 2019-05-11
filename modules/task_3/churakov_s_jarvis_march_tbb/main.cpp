//  Copyright 2019 Churakov Sergey
#include <tbb/tbb.h>
#include <iostream>
#include <iomanip>
#include <ctime>
#include <cstdlib>
#include <cmath>
#define step 1000

void RandomizeArray(double* rarray, const int& size, const int& min = -50, const int& max = 50) {
    for (int i = 0; i < size; i++) {
        int part1 = std::rand();
        int part2 = part1 << (sizeof(int)*4);
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

class BLPointSearch {
 private:
    double* X;
    double* Y;
    int Result;

 public:
    explicit BLPointSearch(double* X_coord, double* Y_coord) {
        X = X_coord;
        Y = Y_coord;
        Result = 0;
    }

    BLPointSearch(const BLPointSearch& m, tbb::split) {
        X = m.X;
        Y = m.Y;
        Result = 0;
    }

    void operator()(const tbb::blocked_range<int>& r) {
        int begin = r.begin();
        int end = r.end();
        for (int i = begin; i < end; i++) {
            if (Y[i] < Y[Result]) {
                Result = i;
            } else {
                if (Y[i] == Y[Result] && X[i] < X[Result])
                    Result = i;
            }
        }
    }

    void join(const BLPointSearch& m) {
        if (Y[m.Result] < Y[Result]) {
            Result = m.Result;
        } else {
            if (Y[m.Result] == Y[Result] && X[m.Result] < X[Result])
                Result = m.Result;
        }
    }

    int getResult() {
        return Result;
    }
};

class MinAngleSearch {
 private:
    double* X;
    double* Y;
    double x1, y1, x2, y2;
    int Result;
    double maxCos;

 public:
    explicit MinAngleSearch(double* X_coord, double* Y_coord, const double& x1,
        const double& y1, const double& x2, const double& y2) {
        X = X_coord;
        Y = Y_coord;
        this->x1 = x1;
        this->y1 = y1;
        this->x2 = x2;
        this->y2 = y2;
        Result = -1;
        maxCos = -1.5;
    }

    MinAngleSearch(const MinAngleSearch& m, tbb::split) {
        X = m.X;
        Y = m.Y;
        x1 = m.x1;
        y1 = m.y1;
        x2 = m.x2;
        y2 = m.y2;
        Result = -1;
        maxCos = -1.5;
    }


    void join(const MinAngleSearch& m) {
        if (m.maxCos > maxCos) {
            Result = m.Result;
            maxCos = m.maxCos;
        }
    }

    void operator()(const tbb::blocked_range<int>& r) {
        int begin = r.begin();
        int end = r.end();
        double tmp;
        int tempResult = Result;
        double tempmaxCos = maxCos;
        for (int i = begin; i < end; i++) {
            if (!((X[i] == x1 && Y[i] == y1) || (X[i] == x2 && Y[i] == y2))) {
                if (((x2 - x1)*(Y[i] - y2) - (X[i] - x2)*(y2 - y1)) >= 0) {
                    tmp = (-1)*GetCos(x2, y2, x1, y1, X[i], Y[i]);
                    if (tmp > tempmaxCos) {
                        tempmaxCos = tmp;
                        tempResult = i;
                    }
                }
            }
        }
        Result = tempResult;
        maxCos = tempmaxCos;
    }

    void reinit(const double& x1, const double& y1,
        const double& x2, const double& y2) {
        this->x1 = x1;
        this->y1 = y1;
        this->x2 = x2;
        this->y2 = y2;
        Result = -1;
        maxCos = -1.5;
    }

    int getResult() {
        return Result;
    }
};

int main(int argc, char* argv[]) {
    tbb::task_scheduler_init init(tbb::task_scheduler_init::automatic);
    tbb::tick_count times, time_part, time_part_fin, timef;
    tbb::tick_count time_part_seq, time_part_seq_fin;
    times = tbb::tick_count::now();
    srand((unsigned int)time(NULL));
    int Size = 100;
    int numt = tbb::task_scheduler_init::default_num_threads();
    if (argc < 1 || argc > 5)
        return 1;
    if (argc > 1) {
        Size = atol(argv[1]);
        if (Size < 1)
            return 1;
    }
    double* X_coord = new double[Size];
    double* Y_coord = new double[Size];
    std::cout << std::endl;
    std::cout << tbb::task_scheduler_init::default_num_threads()
        << " threads are working by default" << std::endl;
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
        numt = atoi(argv[2]);
        if (numt > 0 && numt < 65) {
            init.terminate();
            init.initialize(numt);
            std::cout << "Number of threads was set to " << numt << std::endl;
        } else {
            return 1;
        }
    }
    if (argc == 5) {
        numt = atoi(argv[4]);
        if (numt > 0 && numt < 65) {
            init.terminate();
            init.initialize(numt);
            std::cout << "Number of threads was set to " << numt << std::endl;
        } else {
            return 1;
        }
    }
//  PrintPoints(X_coord, Y_coord, Size);
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
            time_part = tbb::tick_count::now();
            BLPointSearch searchBLP(X_coord, Y_coord);
            tbb::parallel_reduce(tbb::blocked_range<int>(0, Size), searchBLP);
            int FirstPoint = searchBLP.getResult();
//            int FirstPoint = FindBLPoint(X_coord, Y_coord, Size);
            Envelope[0] = FirstPoint;
            MinAngleSearch jarvis(X_coord, Y_coord, X_coord[FirstPoint] - 1,
                Y_coord[FirstPoint], X_coord[FirstPoint], Y_coord[FirstPoint]);
            tbb::parallel_reduce(tbb::blocked_range<int>(0, Size), jarvis);
            Envelope[1] = jarvis.getResult();
//            Envelope[1] = FindPWithMinAngleParallel(X_coord, Y_coord, Size, X_coord[FirstPoint] - 1,
 //               Y_coord[FirstPoint], X_coord[FirstPoint], Y_coord[FirstPoint]);
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
                jarvis.reinit(X_coord[Envelope[PNum - 2]], Y_coord[Envelope[PNum - 2]],
                    X_coord[Envelope[PNum - 1]], Y_coord[Envelope[PNum - 1]]);
                tbb::parallel_reduce(tbb::blocked_range<int>(0, Size), jarvis);
                Envelope[PNum] = jarvis.getResult();
//                Envelope[PNum] = FindPWithMinAngleParallel(X_coord, Y_coord, Size, X_coord[Envelope[PNum - 2]],
//                    Y_coord[Envelope[PNum - 2]], X_coord[Envelope[PNum - 1]], Y_coord[Envelope[PNum - 1]]);
            }
            ElimPointsOnLines(X_coord, Y_coord, Envelope, &PNum);
            time_part_fin = tbb::tick_count::now();
            dynsize = step;
//          Sequential implementation
            int* EnvelopeForCheck = static_cast<int*>(malloc(sizeof(int) * dynsize));
            int PNumForCheck = 1;
            int Correct = 1;
            time_part_seq = tbb::tick_count::now();
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
            time_part_seq_fin = tbb::tick_count::now();
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
            timef = tbb::tick_count::now();
            std::cout << std::endl;
            std::cout << "time: " << (timef - times).seconds() << std::endl;
            std::cout << "time without initialisation and preparations: " <<
                (time_part_fin - time_part).seconds() << std::endl;
            std::cout << "time of sequential execution of the same task: " <<
                (time_part_seq_fin - time_part_seq).seconds();
            std::cout << std::endl;
            double acc = (time_part_seq_fin - time_part_seq).seconds() /
                (time_part_seq - time_part).seconds();
            std::cout << "Acceleration: " << acc << std::endl;
            std::cout << "Efficiency: " << acc / numt << std::endl;
            free(Envelope);
            delete[] EnvelopeForCheck;
        }
    }
    delete[] X_coord;
    delete[] Y_coord;
    return 0;
}
