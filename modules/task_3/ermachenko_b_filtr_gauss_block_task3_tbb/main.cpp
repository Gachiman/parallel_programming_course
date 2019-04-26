// Copyright Ermachenko Boris 2019
// 1
#include <math.h>
#include <tbb/tbb.h>
#include <iostream>
#include <ctime>
#define THREADS 12
const double PI = 3.1415;
int blockX = 0;
int blockY = 0;
double** getNewArr(int width, int height) {
    double** new_arrImage = new double*[height];
    for (int i = 0; i < height; i++) {
        new_arrImage[i] = new double[width];
    }
    return new_arrImage;
}
double** LinierFilterGauss(double** arrImage, double** new_arrImage,
    int width, int height) {
    int n = 3;
    double** w = getNewArr(n, n);
    double sigma = 1.0;
    double r;
    double s = 2.0 * sigma * sigma;
    for (int i = -n / 2; i < n / 2; i++) {
        for (int j = -n / 2; j < n / 2; j++) {
            r = sqrt(i * i + j * j);
            w[i + n / 2][j + n / 2] = (exp(-(r*r) / s)) / (PI * s);
        }
    }
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            new_arrImage[i][j] = arrImage[i][j];
        }
    }
    for (int yi = 0; yi < height; yi++) {
        for (int xj = 0; xj < width; xj++) {
            double color = 0;
            double kSum = 0;
            for (int i = -n / 2; i < n / 2; i++) {
                for (int j = -n / 2; j < n / 2; j++) {
                    kSum += w[i + n / 2][j + n / 2];
                    if ((i + yi) >= 0 && (i + yi) < height
                        && (j + xj) >= 0 && (j + xj) < width)
                        color += arrImage[static_cast<int>(i + yi)][static_cast<int>(j + xj)]
                        * w[i + n / 2][j + n / 2];
                }
            }
            if (kSum <= 0) kSum = 1;
            color /= kSum;
            if (color < 0) color = 0;
            if (color > 255) color = 255;
            if ((yi) >= 0 && (yi) < height && (xj) >= 0 && (xj) < width)
                new_arrImage[static_cast<int>(yi)][static_cast<int>(xj)] = color;
        }
    }
    return new_arrImage;
}
void getBlockNum(int throwds) {
    int tmp = static_cast<int>(sqrt(throwds));
    while (throwds % tmp != 0) {
        tmp--;
    }
    blockX = tmp;
    blockY = throwds / tmp;
}
int* arrayLengthBlocksX(int width) {
    int* rezArrX = new int[blockX];
    int rest = width % blockX;
    for (int i = 0; i < blockX; i++) {
        rezArrX[i] = width / blockX;
    }
    if (rest != 0) {
        int k = 0;
        while (rest != 0) {
            rezArrX[k%rest]++;
            rest--;
            k++;
        }
    }
    return rezArrX;
}
int * arrayLengthBlocksY(int height) {
    int* rezArrY = new int[blockY];
    int rest = height % blockY;
    for (int i = 0; i < blockY; i++) {
        rezArrY[i] = height / blockY;
    }
    if (rest != 0) {
        int k = 0;
        while (rest != 0) {
            rezArrY[k%rest]++;
            rest--;
            k++;
        }
    }
    return rezArrY;
}
double** ParallelFilterGaussTBB(double** arrImage,
    double** new_arrImage, int width, int height) {
    int n = 3;
    double** w = getNewArr(n, n);
    double sigma = 1.0;
    double r;
    double s = 2.0 * sigma * sigma;
    for (int i = -n / 2; i < n / 2; i++) {
        for (int j = -n / 2; j < n / 2; j++) {
            r = sqrt(i * i + j * j);
            w[i + n / 2][j + n / 2] = (exp(-(r*r) / s)) / (PI * s);
        }
    }
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            new_arrImage[i][j] = arrImage[i][j];
        }
    }
    getBlockNum(THREADS);
    int* lengthX = arrayLengthBlocksX(width);
    int* lengthY = arrayLengthBlocksY(height);
    tbb::parallel_for(tbb::blocked_range2d<int>(0, height, lengthY[0], 0, width, lengthX[0]),
        [&](const tbb::blocked_range2d<int, int>& r) {
            // printf("1 \n");
            for (int yi = r.rows().begin(); yi < r.rows().end(); yi++) {
                for (int xj = r.cols().begin(); xj < r.cols().end(); xj++) {
                    double color = 0;
                    double kSum = 0;
                    for (int i = -n / 2; i < n / 2; i++) {
                        for (int j = -n / 2; j < n / 2; j++) {
                            kSum += w[i + n / 2][j + n / 2];
                            if ((i + yi) >= 0 && (i + yi) < height
                                && (j + xj) >= 0 && (j + xj) < width) {
                                color += arrImage[static_cast<int>(i + yi)][static_cast<int>(j + xj)]
                                * w[i + n / 2][j + n / 2];
                            }
                        }
                    }
                    if (kSum <= 0) kSum = 1;
                    color /= kSum;
                    if (color < 0) color = 0;
                    if (color > 255) color = 255;
                    if ((yi) >= 0 && (yi) < height && (xj) >= 0 && (xj) < width) {
                    new_arrImage[static_cast<int>(yi)][static_cast<int>(xj)]
                        = color;
                    }
                }
            }
        });
    return new_arrImage;
}
double** getImage(int width, int height) {
    srand((unsigned int)time(0));
    double** arrImage = getNewArr(width, height);
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            arrImage[i][j] = static_cast<int>(std::rand()) % 256;
        }
    }
    return arrImage;
}
void ShowArr(double** arr, int width, int height) {
    std::cout << "\n";
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            std::cout << arr[i][j] << " ";
        }
        std::cout << "\n";
    }
}
int main() {
    int height = 10;
    int width = 20;
    double** arrImage = getImage(width, height);
    double** new_arrImage_Liner = getNewArr(width, height);
    // liner
    tbb::tick_count tStart = tbb::tick_count::now();
    new_arrImage_Liner = LinierFilterGauss(arrImage,
        new_arrImage_Liner, width, height);
    tbb::tick_count tEnd = tbb::tick_count::now();
    // float tmp = (tEnd - tStart).seconds();
    printf("Time linier :  %.4lf \n", (tEnd - tStart).seconds());
    tbb::task_scheduler_init init(tbb::task_scheduler_init::deferred);
    init.initialize(THREADS);
    double** new_arrImage_Parallel_TBB = getNewArr(width, height);
    tbb::tick_count start = tbb::tick_count::now();
    new_arrImage_Parallel_TBB = ParallelFilterGaussTBB(arrImage,
        new_arrImage_Parallel_TBB, width, height);
    tbb::tick_count end = tbb::tick_count::now();
    printf("Time parallel TBB:  %.4lf \n", (end - start).seconds());
    // printf("Koef :  %.4lf \n", tmp / (end - start).seconds());
     ShowArr(arrImage, width, height);
     printf("Liner:\n");
     ShowArr(new_arrImage_Liner, width, height);
     printf("TBB\n");
    // ShowArr(new_arrImage_Parallel, width, height);
    // printf(" \n");
     ShowArr(new_arrImage_Parallel_TBB, width, height);
    return 0;
}
