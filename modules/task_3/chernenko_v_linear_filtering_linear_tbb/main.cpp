// Copyright 2019 Chernenko Valery
#include <omp.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <tbb/tbb.h>
#include <iostream>
#include <fstream>
// #include <opencv2\core\core.hpp>
// #include <opencv2\highgui\highgui.hpp>

int clamp(int x, int min, int max) {
    if (x < min) return min;
    if (x > max) return max;
    return x;
}

class fI {
 private:
    unsigned char** imageR;
    unsigned char** imageG;
    unsigned char** imageB;
    int width, height;
    unsigned char** resultImageR;
    unsigned char** resultImageG;
    unsigned char** resultImageB;
    int n;
    double** filter;

 public:
    fI(unsigned char** iR, unsigned char** iG, unsigned char** iB, int _height, int _width, double** f, int _n) {
        width = _width;
        height = _height;
        imageR = new unsigned char*[height];
        imageG = new unsigned char*[height];
        imageB = new unsigned char*[height];
        for (int i = 0; i < height; i++) {
            imageR[i] = new unsigned char[width];
            imageG[i] = new unsigned char[width];
            imageB[i] = new unsigned char[width];
            for (int j = 0; j < width; j++) {
                imageR[i][j] = iR[i][j];
                imageG[i][j] = iG[i][j];
                imageB[i][j] = iB[i][j];
            }
        }
        resultImageR = new unsigned char*[height];
        resultImageG = new unsigned char*[height];
        resultImageB = new unsigned char*[height];
        for (int i = 0; i < height; i++) {
            resultImageR[i] = new unsigned char[width];
            resultImageG[i] = new unsigned char[width];
            resultImageB[i] = new unsigned char[width];
        }
        n = _n;
        filter = new double*[n];
        for (int i = 0; i < n; i++) {
            filter[i] = new double[n];
            for (int j = 0; j < n; j++)
                filter[i][j] = f[i][j];
        }
    }

    void operator() (const tbb::blocked_range<int>& r) const {
        int rad = n / 2;
        for (int i = r.begin(); i < r.end(); i++)
            for (int j = 0; j < width; j++) {
                double resultR = 0;
                double resultG = 0;
                double resultB = 0;
                for (int fi = -rad; fi <= rad; fi++)
                    for (int fj = -rad; fj <= rad; fj++) {
                        int currI = clamp(i + fi, 0, height - 1);
                        int currJ = clamp(j + fj, 0, width - 1);
                        resultR += imageR[currI][currJ] * filter[fi + rad][fj + rad];
                        resultG += imageG[currI][currJ] * filter[fi + rad][fj + rad];
                        resultB += imageB[currI][currJ] * filter[fi + rad][fj + rad];
                    }
                resultImageR[i][j] = clamp(static_cast<int>(resultR), 0, 255);
                resultImageG[i][j] = clamp(static_cast<int>(resultG), 0, 255);
                resultImageB[i][j] = clamp(static_cast<int>(resultB), 0, 255);
            }
    }

    unsigned char** getResultImageR() {
        unsigned char** res = new unsigned char*[height];
        for (int i = 0; i < height; i++) {
            res[i] = new unsigned char[width];
            for (int j = 0; j < width; j++) {
                res[i][j] = resultImageR[i][j];
            }
        }
        return res;
    }

    unsigned char** getResultImageG() {
        unsigned char** res = new unsigned char*[height];
        for (int i = 0; i < height; i++) {
            res[i] = new unsigned char[width];
            for (int j = 0; j < width; j++) {
                res[i][j] = resultImageG[i][j];
            }
        }
        return res;
    }

    unsigned char** getResultImageB() {
        unsigned char** res = new unsigned char*[height];
        for (int i = 0; i < height; i++) {
            res[i] = new unsigned char[width];
            for (int j = 0; j < width; j++) {
                res[i][j] = resultImageB[i][j];
            }
        }
        return res;
    }
};

unsigned char** getRandImage(int width, int height) {
    unsigned char** image = new unsigned char*[height];
    for (int i = 0; i < height; i++) {
        image[i] = new unsigned char[width];
        for (int j = 0; j < width; j++)
            image[i][j] = static_cast<unsigned char>(std::rand() % 256);
    }
    return image;
}

double** getGausseFilter(int n) {
    int rad = n / 2;
    double** result = new double*[n];
    for (int i = 0; i < n; i++)
        result[i] = new double[n];
    double coeff1 = 1 / (2 * 3.14159265 * rad * rad);
    for (int i = -rad; i <= rad; i++) {
        for (int j = -rad; j <= rad; j++) {
        result[i + rad][j + rad] = coeff1 * exp((-(i*i + j * j)) / (2 * rad * rad));
        }
    }
    double coeff2 = 0;
    for (int i = 0; i < n; i++)
        for (int j = 0; j < n; j++)
            coeff2 += result[i][j];
    for (int i = 0; i < n; i++)
        for (int j = 0; j < n; j++)
            result[i][j] /= coeff2;
    return result;
}

unsigned char getResultPixel(unsigned char** image, int width, int height, int x, int y, double** filter, int n) {
    double result = 0;
    int rad = n / 2;
    for (int i = -rad; i <= rad; i++)
        for (int j = -rad; j <= rad; j++) {
            double r = image[clamp(x + i, 0, height - 1)][clamp(y + j, 0, width - 1)] * filter[i + rad][j + rad];
            result += r;
        }
    return clamp(static_cast<int>(result), 0, 255);
}

unsigned char* getResultRows(unsigned char** image, int width, int height, int x, double** filter, int n) {
    unsigned char* resultRow = new unsigned char[width];
    for (int j = 0; j < width; j++) {
        resultRow[j] = getResultPixel(image, width, height, x, j, filter, n);
    }
    return resultRow;
}

unsigned char** getResultImage(unsigned char** image, int width, int height, double** filter, int n) {
    unsigned char** resultImage = new unsigned char*[height];
    int i = 0;
    for (i = 0; i < height; i++)
        resultImage[i] = getResultRows(image, width, height, i, filter, n);
    return resultImage;
}

/*
cv::Mat getRandImage(int width, int height) {
	cv::Mat resultImage(height, width, CV_8UC3);
	for (int i = 0; i < 3 * width * height; i++) {
		resultImage.data[i] = static_cast<unsigned char>(std::rand() % 256);
	}
	return resultImage;
}

cv::Mat filterImage(cv::Mat image, double** filter, int n) {
    int width = image.cols;
    int height = image.rows;
    int rad = n / 2;
    cv::Mat resultImage(height, width, CV_8UC3);
    for(int i = 0; i < height; i++)
        for (int j = 0; j < width; j++) {
            double resultR = 0;
            double resultG = 0;
            double resultB = 0;
            for(int fi = -rad; fi <= rad; fi++) 
                for (int fj = -rad; fj <= rad; fj++) {
                    int currI = clamp(i + fi, 0, height - 1);
                    int currJ = clamp(j + fj, 0, width - 1);
                    resultR += image.data[3 * (currI*width + currJ)] * filter[fi + rad][fj + rad];
                    resultG += image.data[3 * (currI*width + currJ) + 1] * filter[fi + rad][fj + rad];
                    resultB += image.data[3 * (currI*width + currJ) + 2] * filter[fi + rad][fj + rad];
                }
            resultImage.data[3 * (i*width + j)] = clamp(static_cast<int>(resultR), 0, 255);
            resultImage.data[3 * (i*width + j) + 1] = clamp(static_cast<int>(resultG), 0, 255);
            resultImage.data[3 * (i*width + j) + 2] = clamp(static_cast<int>(resultB), 0, 255);
        }
    return resultImage;
}

cv::Mat filterImageOmpPar(cv::Mat image, double** filter, int n) {
    int width = image.cols;
    int height = image.rows;
    int rad = n / 2;
    cv::Mat resultImage(height, width, CV_8UC3);
#pragma omp parallel num_threads(4) default(shared)
    {
#pragma omp for
    for (int i = 0; i < height; i++)
        for (int j = 0; j < width; j++) {
            double resultR = 0;
            double resultG = 0;
            double resultB = 0;
            for (int fi = -rad; fi <= rad; fi++)
                for (int fj = -rad; fj <= rad; fj++) {
                    int currI = clamp(i + fi, 0, height - 1);
                    int currJ = clamp(j + fj, 0, width - 1);
                    resultR += image.data[3 * (currI*width + currJ)] * filter[fi + rad][fj + rad];
                    resultG += image.data[3 * (currI*width + currJ) + 1] * filter[fi + rad][fj + rad];
                    resultB += image.data[3 * (currI*width + currJ) + 2] * filter[fi + rad][fj + rad];
                }
            resultImage.data[3 * (i*width + j)] = clamp(static_cast<int>(resultR), 0, 255);
            resultImage.data[3 * (i*width + j) + 1] = clamp(static_cast<int>(resultG), 0, 255);
            resultImage.data[3 * (i*width + j) + 2] = clamp(static_cast<int>(resultB), 0, 255);
        }
    }
    return resultImage;
}

class filteringImage {

private:

    cv::Mat image;
    int n;
    double** filter;
    cv::Mat* resultImage;

public:

    filteringImage(cv::Mat _image, double** _filter, int _n) {
        image = _image;
        resultImage = new cv::Mat(image.rows, image.cols, CV_8UC3);
        n = _n;
        filter = new double*[n];
        for (int i = 0; i < n; i++) {
            filter[i] = new double[n];
            for (int j = 0; j < n; j++)
                filter[i][j] = _filter[i][j];
        }
    }

    void operator() (const tbb::blocked_range<int>& r) const {
        int width = image.cols;
        int height = image.rows;
        int rad = n / 2;
        for (int i = r.begin(); i < r.end(); i++)
            for (int j = 0; j < width; j++) {
                double resultR = 0;
                double resultG = 0;
                double resultB = 0;
                for (int fi = -rad; fi <= rad; fi++)
                    for (int fj = -rad; fj <= rad; fj++) {
                        int currI = clamp(i + fi, 0, height - 1);
                        int currJ = clamp(j + fj, 0, width - 1);
                        resultR += image.data[3 * (currI*width + currJ)] * filter[fi + rad][fj + rad];
                        resultG += image.data[3 * (currI*width + currJ) + 1] * filter[fi + rad][fj + rad];
                        resultB += image.data[3 * (currI*width + currJ) + 2] * filter[fi + rad][fj + rad];
                    }
                resultImage->data[3 * (i*width + j)] = clamp(static_cast<int>(resultR), 0, 255);
                resultImage->data[3 * (i*width + j) + 1] = clamp(static_cast<int>(resultG), 0, 255);
                resultImage->data[3 * (i*width + j) + 2] = clamp(static_cast<int>(resultB), 0, 255);
            }
    }

    cv::Mat getResutlImage() {
        return *resultImage;
    }
};

cv::Mat filterImageTbbPar(cv::Mat image, double** filter, int n) {
    cv::Mat resultImage(image.rows, image.cols, CV_8UC3);
    filteringImage filtImg(image, filter, n);
    tbb::parallel_for(tbb::blocked_range<int>(0, image.rows, 1), filtImg);
    return filtImg.getResutlImage();
}
*/

void fITbbPar(unsigned char** iR, unsigned char** iG, unsigned char** iB, int width, int height, double** f, int n) {
    fI filtImg(iR, iG, iB, width, height, f, n);
    tbb::parallel_for(tbb::blocked_range<int>(0, height, 1), filtImg);
}

int main(int argv, char** argc) {
    srand((unsigned int)time(0));
    setlocale(LC_ALL, "Russian");

    int n = 3;
    if (n % 2 == 0) n++;
    double** filter = getGausseFilter(n);
    tbb::task_scheduler_init init(4);
    int width = 1000;
    int height = 1000;

    unsigned char** imageR = getRandImage(width, height);
    unsigned char** imageG = getRandImage(width, height);
    unsigned char** imageB = getRandImage(width, height);

    unsigned char** imageRTbb = getRandImage(width, height);
    unsigned char** imageGTbb = getRandImage(width, height);
    unsigned char** imageBTbb = getRandImage(width, height);

    tbb::tick_count start = tbb::tick_count::now();
    getResultImage(imageR, width, height, filter, n);
    getResultImage(imageG, width, height, filter, n);
    getResultImage(imageB, width, height, filter, n);
    tbb::tick_count finish = tbb::tick_count::now();
    printf("Linier Time :  %.4lf \n", (finish - start).seconds());

    tbb::tick_count startTbb = tbb::tick_count::now();
    fITbbPar(imageRTbb, imageGTbb, imageBTbb, width, height, filter, n);
    tbb::tick_count finishTbb = tbb::tick_count::now();
    printf("Parallel Tbb Time :  %.4lf \n", (finishTbb - startTbb).seconds());

    printf("Boost Tbb :  %.4lf \n", (finish - start).seconds()/(finishTbb - startTbb).seconds());

    for (int i = 0; i < height; i++) {
        delete[] imageR[i];
        delete[] imageG[i];
        delete[] imageB[i];
        delete[] imageRTbb[i];
        delete[] imageGTbb[i];
        delete[] imageBTbb[i];
    }
    delete[] imageR;
    delete[] imageG;
    delete[] imageB;
    delete[] imageRTbb;
    delete[] imageGTbb;
    delete[] imageBTbb;

/*
    char* path = "SS:\\Projects\\C++\\Files\\image.jpg";
    cv::Mat image = cv::imread(path);
    cv::Mat imageOmp = cv::imread(path);
    cv::Mat imageTbb = cv::imread(path);

    if (image.rows == 0 || image.cols == 0) {
        std::cout << "File not found! Generate rand image" << std::endl;
        image = getRandImage(1000, 1000);
        imageOmp = getRandImage(1000, 1000);
        imageTbb = getRandImage(1000, 1000);
        std::cout << "Image generated" << std::endl;
    }

    std::cout << "Ìàêñèìàëüíîå êîëè÷åñòâî ïîòîêîâ: " << omp_get_max_threads() << std::endl;
    std::cout << "Start work" << std::endl;

    double startTime = omp_get_wtime();
    cv::Mat resultImage = filterImage(image, filter, n);
    double finishTime = omp_get_wtime();
    std::cout << "Ñonsistent work completed. Elapsed time = " << finishTime - startTime << std::endl;
    cv:imwrite("S:\\Projects\\C++\\Files\\resultImage.bmp", resultImage);

    double startTimeOmpPar = omp_get_wtime();
    cv::Mat resultImageOmpPar = filterImageOmpPar(imageTbb, filter, n);
    double finishTimeOmpPar = omp_get_wtime();
    std::cout << "Parallel work completed. Elapsed time = " << finishTimeOmpPar - startTimeOmpPar << std::endl;
    cv::imwrite("S:\\Projects\\C++\\Files\\resultImageOmpPar.bmp", resultImageOmpPar);
    std::cout << "Boost Omp = " << (finishTime - startTime) / (finishTimeOmpPar - startTimeOmpPar) << std::endl;

    double startTimeTbbPar = omp_get_wtime();
    cv::Mat resultImageTbbPar = filterImageTbbPar(imageOmp, filter, n);
    double finishTimeTbbPar = omp_get_wtime();
    std::cout << "Parallel work completed. Elapsed time = " << finishTimeTbbPar - startTimeTbbPar << std::endl;
    cv::imwrite("S:\\Projects\\C++\\Files\\resultImageTbbPar.bmp", resultImageTbbPar);
    std::cout << "Boost Tbb = " << (finishTime - startTime) / (finishTimeTbbPar - startTimeTbbPar) << std::endl;
    */
}


/*
int main(int argv, char** argc) {
    srand((unsigned int)time(0));
    setlocale(LC_ALL, "Russian");

    std::cout << "Ìàêñèìàëüíîå êîëè÷åñòâî ïîòîêîâ: " << omp_get_max_threads() << std::endl;

    int n = 7;
    if (n % 2 == 0) n++;
    double** filter = getGausseFilter(n);

    cv::Mat frame;
    char* filename = "S:\\Projects\\C++\\Files\\test.mp4";
	cv::VideoCapture video(filename);
	video >> frame;

	double start = omp_get_wtime();
	frame = filterImageOmpPar(frame, filter, n);
	double finish = omp_get_wtime();
	std::cout << static_cast<int>(1 / (finish - start)) << " fps" << std::endl;

	cv::imshow("original", frame);

    while (1) {
		video >> frame;
        if (frame.empty()) {
            break;
        }

		double start = omp_get_wtime();

		frame = filterImageOmpPar(frame, filter, n);

		cv::imshow("original", frame);
		double finish = omp_get_wtime();
		std::cout << static_cast<int>(1 / (finish - start)) << " fps" << std::endl;

        char c = cvWaitKey(33);
        if (c == 27) {
            break;
        }
    }

    return 0;
}*/

/*
int main(int argv, char** argc) {
	srand((unsigned int)time(0));
	setlocale(LC_ALL, "Russian");
	tbb::task_scheduler_init init;

	int n = 7;
	if (n % 2 == 0) n++;
	double** filter = getGausseFilter(n);


	cv::Mat frame;
	char* filename = "S:\\Projects\\C++\\Files\\test.mp4";
	cv::VideoCapture video(filename);
	video >> frame;

	double start = omp_get_wtime();
	frame = filterImageTbbPar(frame, filter, n);
	double finish = omp_get_wtime();
	std::cout << static_cast<int>(1 / (finish - start)) << " fps" << std::endl;

	cv::imshow("original", frame);

	while (1) {
		video >> frame;
		if (frame.empty()) {
			break;
		}

		double start = omp_get_wtime();

		frame = filterImageTbbPar(frame, filter, n);

		cv::imshow("original", frame);
		double finish = omp_get_wtime();
		std::cout << static_cast<int>(1 / (finish - start)) << " fps" << std::endl;

		char c = cvWaitKey(33);
		if (c == 27) {
			break;
		}
	}

	return 0;
}*/
