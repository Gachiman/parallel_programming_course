// A sequential version of linear filtration algorithm with Gauss kernel
// Copyright 2019 Ponomarev Alexey

#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <iostream>
#include <algorithm>
#include <vector>
#include <string>
#include <ctime>
#include <fstream>
#include <random>
#include <array>
#include <cstring>


// for image output
// #include <opencv2\imgcodecs.hpp>
// #include <opencv\cv.hpp>
// #include <opencv2/core/core.hpp>
// #include <opencv2/highgui/highgui.hpp>
// using namespace cv;


// constants
const int IMAGE_WIDTH = 3200;
const int IMAGE_HEIGHT = 1800;
// const int IMAGE_SIZE = IMAGE_WIDTH * IMAGE_WIDTH;
// const int IMAGE_COMPONENTS_COUNT = 3;
const char* GENERATED_IMAGE_NAME = "generated image";
const char* FILTERD_IMAGE_NAME = "generated image";
const double MATH_PI = 3.14159265359;
const int KERNEL_RADIUS = 1;
const char* KERNEL_NULL_ERROR = "ERROR: kernel is null";
const char* KERNEL_ROW_NULL_ERROR = "ERROR: nullable kernel row#";
const char* ALLOCATING_IMAGE_MEMORY_ERROR = "ERROR: error with allocating memory for image";
const char* IMAGE_NULL_ERROR = "ERROR: image is null";
const double DEFAULT_SIGMA = 1;
// const char* DEFAULT_FILE = "azamat.jpg";
const char* DEFAULT_FILE = "lines.jpg";

struct Pixel {
    int r;
    int g;
    int b;
};

Pixel** allocateImageMemory(int width, int height) {
    // allocate memory
    Pixel** image = new Pixel*[height];
    for (int i = 0; i < height; i++) {
        image[i] = new Pixel[width];
    }
    return image;
}

// generating random image
Pixel** generateImage(int width, int height) {
    // allocate memory
    Pixel** image = allocateImageMemory(width, height);
    // init array
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            image[i][j].r = std::rand() % 255;
            image[i][j].g = std::rand() % 255;
            image[i][j].b = std::rand() % 255;
        }
    }
    return image;
}


// generating Gauss kernel
double** generateGaussKernel(int radius, double sigma = DEFAULT_SIGMA) {
    // formula:  kernel(u, v) = (1/(2*pi*r^2))*e^(-(u^2 + v^2)/(2 * r^2)))
    /* defining kernel and allocating memory for it */
    double **kernel = new double*[radius * 2 + 1];
    for (int i = 0; i < 2 * radius + 1; i++) {
        kernel[i] = new double[2 * radius + 1];
    }

    /* init coefficients */
    double k = 2 * radius * radius;
    double k_pi = MATH_PI * k;

    /* summ processing */
    for (int u = -radius; u <= radius; u++) {
        for (int v = -radius; v <= radius; v++) {
            kernel[u + radius][v + radius] = exp(-(u * u + v * v) / k);  // now  without sigma
            kernel[u + radius][v + radius] /= k_pi;
        }
    }

    return kernel;
}

bool deleteKernel(double **kernel, int radius) {
    if (kernel == NULL) {
        return false;
    }
    for (int i = 0; i < 2 * radius + 1; i++) {
        if (kernel[i] == NULL) {
            return false;
        }
        delete[] kernel[i];
    }
    return true;
    delete[] kernel;
}

// null checking function for image
bool isImageNull(Pixel** image, int imHeight) {
    if (image == NULL) {
        return true;
    }
    for (int i = 0; i < imHeight; i++) {
        if (image[i] == NULL) {
            return true;
        }
    }
    return false;
}


// wrapper function for null image checking
void checkIsImageNull(Pixel** image, int imHeight,
    const char* locationMessage = "", const char* errorMessage = IMAGE_NULL_ERROR) {
    if (isImageNull(image, imHeight)) {
        std::cout << "*** " << errorMessage << " for " << locationMessage << std::endl;
    }
}


int clamp(int x, int max = 255, int min = 0) {
    return (x > max) ? max :
        (x < min) ? min : x;
}

// filter for one pixel
Pixel seqPixelFiltering(Pixel** genImage, int width, int height, double** kernel, int kerRadius, int x, int y) {
    Pixel outPixel;

    outPixel.r = 0;
    outPixel.b = 0;
    outPixel.g = 0;

    double r = 0.0;
    double g = 0.0;
    double b = 0.0;

    for (int u = -kerRadius; u <= kerRadius; u++) {
        for (int v = -kerRadius; v <= kerRadius; v++) {
            if (x + v >= 0 && x + v < width && y + u >= 0 && y + u < height) {
                r += kernel[u + kerRadius][v + kerRadius] *
                    static_cast<double>(genImage[y + u][x + v].r);
                g += kernel[u + kerRadius][v + kerRadius] *
                    static_cast<double>(genImage[y + u][x + v].g);
                b += kernel[u + kerRadius][v + kerRadius] *
                    static_cast<double>(genImage[y + u][x + v].b);
            }
        }
    }

    outPixel.r = clamp(static_cast<int>(r));
    outPixel.g = clamp(static_cast<int>(g));
    outPixel.b = clamp(static_cast<int>(b));

    /*for (int u = -kerRadius; u <= kerRadius; u++) {
        for (int v = -kerRadius; v <= kerRadius; v++) {
            if (x + v >= 0 && x + v < width && y + u >= 0 && y + u < height) {
                outPixel.r += static_cast<int>(kernel[u + kerRadius][v + kerRadius] *
                    static_cast<double>(genImage[y + u][x + v].r));
                outPixel.g += static_cast<int>(kernel[u + kerRadius][v + kerRadius] *
                    static_cast<double>(genImage[y + u][x + v].g));
                outPixel.b += static_cast<int>(kernel[u + kerRadius][v + kerRadius] *
                    static_cast<double>(genImage[y + u][x + v].b));
            }
        }
    }*/

    return outPixel;
}

// sequential version
Pixel** seqFilter(Pixel** genImage, int width, int height, double** kernel, int kerRadius) {
    Pixel** filteredImage = allocateImageMemory(width, height);
    checkIsImageNull(genImage, height, " for generated image in seq filter ");
    checkIsImageNull(filteredImage, height, " for filtered image in seq filter ", ALLOCATING_IMAGE_MEMORY_ERROR);

    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            filteredImage[i][j] = seqPixelFiltering(genImage, width, height, kernel, kerRadius, j, i);
        }
    }

    return filteredImage;
}


// parallel version
Pixel** parFilter(Pixel** genImage, int width, int height, double** kernel, int kerRadius, int threadsNumber) {
    Pixel** filteredImage = allocateImageMemory(width, height);
    checkIsImageNull(genImage, height, " for generated image in seq filter ");
    checkIsImageNull(filteredImage, height, " for filtered image in seq filter ", ALLOCATING_IMAGE_MEMORY_ERROR);

    omp_set_num_threads(threadsNumber);

#pragma omp parallel for schedule(dynamic)
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            filteredImage[i][j] = seqPixelFiltering(genImage, width, height, kernel, kerRadius, j, i);
        }
    }

    return filteredImage;
}

bool deleteImage(int height, Pixel** image) {
    if (image == NULL) {
        return false;
    }
    for (int i = 0; i < height; i++) {
        if (image[i] == NULL) {
            return false;
        }
        delete[] image[i];
    }
    delete[] image;
    return true;
}

void tryDeleteImage(int height, Pixel** image, const char* imageName) {
    std::cout << "*** LOG: try deleting image with name = " << imageName << " ***" << std::endl;
    if (deleteImage(height, image)) {
        std::cout << "*** " << imageName << " successfully deleted ***" << std::endl;
    } else {
        std::cout << "*** error with deleting " << imageName << " ***" << std::endl;
    }
}

void tryDeleteKernel(int radius, double** kernel) {
    std::cout << "*** LOG: try deleting kernel ***" << std::endl;
    if (deleteKernel(kernel, radius)) {
        std::cout << "*** kernel successfully deleted ***" << std::endl;
    } else {
        std::cout << "*** error with deleting kernel ***" << std::endl;
    }
}

void printImage(Pixel** image, int width, int height) {
    checkIsImageNull(image, height, " in printing function ");
    std::cout << std::endl;
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            std::cout << "("
                << image[i][j].r << ", "
                << image[i][j].g << ", "
                << image[i][j].b
                << ")";
        }
        std::cout << std::endl;
    }
    std::cout << std::endl;
    std::cout << std::endl;
}


void printKernel(double **kernel, int radius) {
    if (kernel == NULL) {
        std::cout << KERNEL_NULL_ERROR << std::endl;
        return;
    }

    std::cout << std::endl;
    std::cout << std::endl;
    std::cout << "Kernel: " << std::endl;

    for (int u = -radius; u <= radius; u++) {
        if (kernel[u + radius] == NULL) {
            std::cout << KERNEL_ROW_NULL_ERROR << u << std::endl;
            return;
        }
        for (int v = -radius; v <= radius; v++) {
            std::cout << kernel[u + radius][v + radius] << ", ";
        }
        std::cout << std::endl;
    }

    std::cout << std::endl;
    std::cout << std::endl;
}

/* Pixel** pixelArrayFromMat(const Mat& mat) {
    std::cout << "start" << std::endl;
    Pixel** image = allocateImageMemory(mat.cols, mat.rows);
    std::cout << "start1" << std::endl;
    for (int y = 0; y < mat.rows; y++) {
        for (int x = 0; x < mat.cols; x++) {
            //std::cout << x << ", " << y << std::endl;
            Vec3b vecPixel = mat.at<Vec3b>(y, x);
            Pixel pixel;
            pixel.r = vecPixel[2];
            pixel.g = vecPixel[1];
            pixel.b = vecPixel[0];
            // Pixel pixel = { vecPixel[2], vecPixel[1], vecPixel[0]  };
            image[y][x] = pixel;
        }
    }
    return image;
} */

/* Mat matFromPixelArray(Pixel** image, int width, int height, unsigned int type) {
    Mat mat = Mat(height, width, type);
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            Pixel pixel = image[y][x];
            Vec3b vecPixel;
            vecPixel[0] = (unsigned char) pixel.b;
            vecPixel[1] = (unsigned char) pixel.g;
            vecPixel[2] = (unsigned char) pixel.r;
            mat.at<Vec3b>(y, x) = vecPixel;
        }
    }
    return mat;
} */


// cvShow
/* void cvShow(const char* window_name, const Mat& image) {
    namedWindow(window_name, CV_WINDOW_AUTOSIZE);
    imshow(window_name, image);
} */

std::string findArg(const std::string& argStr, const std::string& templ) {
    size_t from = argStr.find(templ);
    if (from != std::string::npos) {
        std::string founded =
            argStr.substr(from + strlen(templ.c_str()), argStr.size() - from - templ.size() + 1);
        std::cout
            << "found " + templ.substr(0, templ.size() - 1) + ": "
            << founded
            << std::endl;
        return founded;
    }
    return "";
}

char* strCpy(const char* s) {
    size_t size = strlen(s);
    char* s1 = new char[size];
    for (size_t i = 0; i < size; i++) {
        s1[i] = s[i];
    }
    return s1;
}

void takeArguments(int* _kerRadius, double* _sigma, char **_fileName, int _argc, char** _argv) {
    std::cout << "argc = " << _argc << std::endl;
    for (int i = 0; i < _argc; i++) {
        std::cout << "argv[" << i << "] = " << _argv[i] << std::endl;
        // std::replace
        // size_t argLen = strlen(_argv[i]);
        // std::vector<char> argVec(_argv, _argv + argLen);
        std::string argStr = std::string(_argv[i]);
        // std::string::iterator strIterFile = std::find(argStr.begin(), argStr.end(), "file=");
        std::string fileName = findArg(argStr, "file=");
        if (fileName != "") {
            // strcpy_s(_fileName, fileName.size(), fileName.c_str());
            *_fileName = strCpy(fileName.c_str());
        }
        std::string sigmaStr = findArg(argStr, "sigma=");
        if (sigmaStr != "") {
            *_sigma = std::stod(sigmaStr);
        }
        std::string kerRadiusStr = findArg(argStr, "radius=");
        if (kerRadiusStr != "") {
            *_kerRadius = std::stoi(kerRadiusStr);
        }
    }
}


/* entry point */
int main(int argc, char* argv[]) {
    // define variables
    Pixel **genImage = NULL;  // image before filtering
    Pixel **filteredImage = NULL;  // image after filtering
    int imWidth = IMAGE_WIDTH;  // image width
    int imHeight = IMAGE_HEIGHT;  // image height
    int kerRadius = KERNEL_RADIUS;
    double **kernel = NULL;  // Gauss kernel
    // int matType = 0;
    double sigma = DEFAULT_SIGMA;  // filter sigma parameter
    char* fileName = strCpy(DEFAULT_FILE);  // reading file name

    /* define extensional variables for different kernel radius */

    /* radius 7 */
    // int kerRadius7 = 7;
    // double **kernel7 = NULL;
    // Pixel **filteredImage7 = NULL;
    // Mat filterMat7;

    /* radius 14 */
    // int kerRadius14 = 14;
    // double **kernel14 = NULL;
    // Pixel **filteredImage14 = NULL;
    // Mat filterMat14;

    // for output image
    // Mat imageMat, filterMat;


    /* initialize random seed: */
    srand(static_cast<int>(time(NULL)));

    takeArguments(&kerRadius, &sigma, &fileName, argc, argv);
    sigma = DEFAULT_SIGMA;  // for tests
    kerRadius = KERNEL_RADIUS;  // for tests

    // imageMat = cvarrToMat(cvLoadImage(DEFAULT_FILE, CV_LOAD_IMAGE_COLOR));
    // matType = imageMat.type();
    // imWidth = imageMat.cols;
    // imHeight = imageMat.rows;

    // cvShow("image", imageMat);
    // cvWaitKey(0);

    // genImage = pixelArrayFromMat(imageMat);
    genImage = generateImage(imWidth, imHeight);

    kernel = generateGaussKernel(kerRadius, sigma);

    // kernel7 = generateGaussKernel(kerRadius7, sigma);
    // kernel14 = generateGaussKernel(kerRadius14, sigma);

    // printKernel(kernel, kerRadius);
    // printKernel(kernel7, kerRadius7);
    // printKernel(kernel14, kerRadius14);

    double startTimeSeq = omp_get_wtime();
    filteredImage = seqFilter(genImage, imWidth, imHeight, kernel, kerRadius);
    std::cout << "Elapsed time for seq version = " << omp_get_wtime() - startTimeSeq << std::endl;
    // filterMat = matFromPixelArray(filteredImage, imWidth, imHeight, matType);

    double startTimePar = omp_get_wtime();
    filteredImage = parFilter(genImage, imWidth, imHeight, kernel, kerRadius, 8);
    std::cout << "Elapsed time for par version = " << omp_get_wtime() - startTimePar << std::endl;

    // filteredImage7 = seqFilter(genImage, imWidth, imHeight, kernel7, kerRadius7);
    // filterMat7 = matFromPixelArray(filteredImage7, imWidth, imHeight, matType);

    // filteredImage14 = seqFilter(genImage, imWidth, imHeight, kernel14, kerRadius14);
    // filterMat14 = matFromPixelArray(filteredImage14, imWidth, imHeight, matType);

    // cvShow("filtered(radius=" + kerRadius, filterMat);
    // cvWaitKey(0);

    // cvShow("filtered(radius=" + kerRadius7, filterMat7);
    // cvWaitKey(0);

    // cvShow("filtered(radius=" + kerRadius14, filterMat14);
    // cvWaitKey(0);

    /* deleting main image */
    tryDeleteImage(imHeight, genImage, GENERATED_IMAGE_NAME);
    tryDeleteImage(imHeight, filteredImage, FILTERD_IMAGE_NAME);

    /* deleting extensional images*/
    // tryDeleteImage(
    //      imHeight,
    //      filteredImage7,
    //      (std::string(FILTERD_IMAGE_NAME) + " with radius 7").c_str()
    //  ); // radius 7
    // tryDeleteImage(
    //        imHeight,
    //        filteredImage14,
    //        (std::string(FILTERD_IMAGE_NAME) + " with radius 14").c_str()
    //    ); // radius 14

    /* deleting main kernel */
    tryDeleteKernel(kerRadius, kernel);

    /* deleting extensional kernels */
    // tryDeleteKernel(kerRadius7, kernel7);  // radius 7
    // tryDeleteKernel(kerRadius14, kernel14);  // radius 14

    // cvWaitKey(0);

    // system("pause");

    return 0;
}




