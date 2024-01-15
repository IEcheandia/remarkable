#include "filter/lineSmooth.h"

#include <numeric>
#include <random>

#include <opencv2/opencv.hpp>

namespace precitec {
namespace filter {

std::vector<double> smoothLine(const std::vector<double>& line, int fmax)
{
    if (fmax < 0)
    {
        return line;
    }

    cv::Mat fftLine;
    std::vector<double> lineSmoothed;

    cv::dft(line, fftLine, cv::DFT_COMPLEX_OUTPUT);

    int size = line.size();

    for (int i = 2 * (fmax + 1); i < 2 * (size - fmax); ++i)
    {
        fftLine.at<double>(i) = 0;
    }

    cv::dft(fftLine, lineSmoothed, cv::DFT_INVERSE | cv::DFT_REAL_OUTPUT | cv::DFT_SCALE);

    return lineSmoothed;
}

std::vector<double> smoothLine(const std::vector<double> &line, int fmax, int iteration, double threshold)
{
    if (fmax < 0)
    {
        return line;
    }

    const int N = line.size();
    const int M = fmax * 2 + 1;

    if (iteration < 1  || threshold <= 0.0 || N < M)
    {
        return smoothLine(line, fmax);
    }

    auto *A = new double[N * M]; // N rows, M cols

    // 1.0 sin(f1x1) cos(f*x1) sin(2f*x1) cos(2f*x1) ... sin(mf*x1) cos(mf*x1)
    for (int i = 0; i < N; ++i)
    {
        A[i * M] = 1.0;

        for (int j = 1; j <= fmax; ++j)
        {
            const auto arg = 2 * M_PI / N * j * i; //f*x
            A[i * M + 2 * j - 1] = sin(arg);
            A[i * M + 2 * j] = cos(arg);
        }
    }

    std::vector<int> randomIndices;

    const int n = (M * iteration) / N + 1;

    for (int i = 0; i < n; ++i)
    {
        std::vector<int> r(N);
        std::iota(r.begin(), r.end(), 0);
        std::mt19937 seed(i);
        std::shuffle(r.begin(), r.end(), seed);
        randomIndices.insert(randomIndices.end(), r.begin(), r.end());
    }

    auto *subA = new double[M * M];
    auto *subB = new double[M];

    auto *maxValid = new uint8_t[N];
    int maxValidCount = 0;

    const cv::Mat cvA(N, M, CV_64FC1, A);
    const cv::Mat cvB(N, 1, CV_64FC1, (void *)line.data());
    const cv::Mat cvSubA(M, M, CV_64FC1, subA);
    const cv::Mat cvSubB(M, 1, CV_64FC1, subB);
    cv::Mat cvValid(N, 1, CV_8UC1);

    for (int i = 0; i < iteration; ++i)
    {
        // prepare submatrix by copying random rows from A to subA
        for (int j = 0; j < M; ++j)
        {
            const auto index = randomIndices[i * M + j];
            std::memcpy(&subA[j * M], &A[index * M], sizeof(*A) * M);
            subB[j] = line[index];
        }

        cvValid = cv::abs(cvA * cvSubA.inv() * cvSubB - cvB) <= threshold;
        const auto validCount = cv::countNonZero(cvValid);

        if (validCount > maxValidCount)
        {
            maxValidCount = validCount;
            cv::Mat cvMaxValid{N, 1, CV_8UC1, maxValid};
            cvValid.copyTo(cvMaxValid);
        }
    }

    if (maxValidCount < M)
    {
        return line;
    }

    // fit all valid data
    cv::Mat cvSubAValid(maxValidCount, M, CV_64FC1);
    cv::Mat cvSubBValid(maxValidCount, 1, CV_64FC1);

    int row = 0;

    for (int i = 0; i < N; ++i)
    {
        if (maxValid[i])
        {
            cvA.row(i).copyTo(cvSubAValid.row(row));
            cvB.row(i).copyTo(cvSubBValid.row(row));
            ++row;
        }
    }

    const auto model = (cvSubAValid.t() * cvSubAValid).inv() * cvSubAValid.t() * cvSubBValid;

    std::vector<double> smoothedLine(N);

    cv::Mat cvSmoothedLine(N, 1, CV_64FC1, smoothedLine.data());
    cvSmoothedLine = cvA * model;

    delete[] maxValid;
    delete[] subB;
    delete[] subA;
    delete[] A;
    return smoothedLine;
}

}
}
