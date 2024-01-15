#include "coordinates/fieldDistortionMapping.h"

#include <iostream>
#include <cmath>
#include <numeric>
#include <algorithm>
#include <random>

#include <opencv2/opencv.hpp>

static void matrixCofactor(const double M[6][6], double C[6][6], int p, int q,
int n)
{
    int i = 0;
    int j = 0;
    for (int r= 0; r< n; r++)
    {
        //Copy only those elements which are not in given row r and column c
        for (int c = 0; c < n; c++)
        {
            if (r != p && c != q)
            {
                //If row is filled increase r index and reset c index
                C[i][j++] = M[r][c];
                if (j == n - 1)
                {
                    j = 0;
                    i++;
                }
            }
        }
    }
}

static double matrixDeterminant(double M[6][6], std::size_t n)
{
    auto determinant = 0.0;
    if (n == 1)
    {
        return M[0][0];
    }
    double C[6][6];
    int sign = 1;

    for (std::size_t f = 0; f < n; f++)
    {
        matrixCofactor(M, C, 0, f, n);
        determinant += sign * M[0][f] * matrixDeterminant(C, n - 1);
        sign = -sign;
    }
    return determinant;
}

static void matrixAdjucate(double M[6][6], double A[6][6])
{
    int sign = 1;
    double C[6][6];
    for (std::size_t i=0; i < 6; i++)
    {
        for (std::size_t j = 0; j < 6; j++)
        {
            matrixCofactor(M, C, i, j, 6);
            sign = ((i + j) % 2 == 0) ? 1 : -1;
            A[j][i] = sign * matrixDeterminant(C, 6 - 1);
        }
    }
}

static bool matrixInverse(double M[6][6], double inverse[6][6])
{
    double determinant = matrixDeterminant(M, 6);
    if (determinant == 0)
    {
        std::cout << "can't find its inverse" << std::endl;
        return false;
    }

    double adjucate[6][6];
    matrixAdjucate(M, adjucate);
    for (std::size_t i = 0; i < 6; i++)
    {
        for (std::size_t j = 0; j < 6; j++)
        {
            inverse[i][j] = adjucate[i][j] / determinant;
        }
    }

    return true;
}

template<class T> static void print(T A[6][6])
{
    for (int i = 0; i < 6; i++)
    {
        for (int j = 0; j < 6; j++)
        {
            std::cout << A[i][j] << " ";
        }
        std::cout << std::endl;
    }
}

std::vector<std::pair<int, int>> detectGridPosition(const uint8_t* image,
std::size_t height, std::size_t width, std::ptrdiff_t stride, double minScore,
int minPixelDistance, std::size_t templateSize)
{
    if (templateSize < 5)
    {
        templateSize = 5;
    }
    if (templateSize > width - 1)
    {
        templateSize = width - 1;
    }
    if (templateSize % 2 == 0)
    {
        templateSize += 1;
    }

    std::vector<std::pair<int, int>> gridPosition;
    cv::Mat cvImage(height, width, CV_8UC1, (void*)image, stride);

    //generate template image for a cross
    //1 1 0 1 1
    //1 1 0 1 1
    //0 0 0 0 0
    //1 1 0 1 1
    //1 1 0 1 1
    const std::size_t crossThickness = 3;
    uint8_t templateImage[templateSize * templateSize];
    memset(templateImage, 255, templateSize * templateSize);
    for (std::size_t i = (templateSize - crossThickness) / 2;
         i < (templateSize + crossThickness) / 2; ++i)
    {
        for (std::size_t j = 0; j < templateSize; ++j)
        {
            templateImage[j * templateSize + i] = 0;
        }

        memset(templateImage + i * templateSize, 0, templateSize);
    }

    const cv::Mat cvTemplateImage(templateSize, templateSize, CV_8UC1, (void*)templateImage);

    cv::Mat xcor;
    cv::matchTemplate(cvImage, cvTemplateImage, xcor, cv::TM_CCORR_NORMED);

    cv::Mat dilateImage;
    const cv::Mat dilateElement = cv::getStructuringElement(cv::MORPH_RECT, {minPixelDistance, minPixelDistance});
    cv::dilate(xcor, dilateImage, dilateElement);

    cv::Mat peakImage = (dilateImage == xcor) & (xcor > minScore);

    std::vector<cv::Point2i> peakLocation;
    cv::findNonZero(peakImage, peakLocation);

    gridPosition.reserve(peakLocation.size());
    for(auto peak : peakLocation)
    {
        gridPosition.emplace_back(peak.x + templateSize / 2, peak.y + templateSize / 2);
    }

    return gridPosition;
}

std::pair<int, int> findClosest(const std::vector<std::pair<int, int>>& gridPosition, double x, double y)
{
    double distanceMin = std::numeric_limits<double>::infinity();
    int closestXMin = 0;
    int closestYMin = 0;

    for (std::size_t i = 0; i < gridPosition.size(); ++i)
    {
        const auto distance = std::abs(gridPosition[i].first - x) + std::abs(gridPosition[i].second - y);
        if (distance < distanceMin)
        {
            distanceMin = distance;
            closestXMin = gridPosition[i].first;
            closestYMin = gridPosition[i].second;
        }
    }

    return {closestXMin, closestYMin};
}

std::vector<std::pair<int, int>> subtractGridCenter(const std::vector<std::pair<int, int>>& gridPosition, double centerX, double centerY)
{
    std::vector<std::pair<int, int>> gridPositionRelative(gridPosition.size());
    for (std::size_t i = 0; i < gridPosition.size(); ++i)
    {
        gridPositionRelative[i] = {gridPosition[i].first - centerX, gridPosition[i].second - centerY};
    }

    return gridPositionRelative;
}

std::vector<std::pair<double, double>> assignGridCoordinate(const std::vector<std::pair<int, int>>& gridPosition, double gridLength, bool invertedX, bool invertedY)
{
    const auto size = gridPosition.size();

    if (size < 4)
    {
        return std::vector<std::pair<double, double>>(gridPosition.begin(), gridPosition.end());
    }

    std::vector<double> vnax(size);
    std::vector<double> vnay(size);
    std::vector<double> vnbx(size);
    std::vector<double> vnby(size);

    // for each point in grid, find the relative position of "left neighbor" (nax, nay)
    // and relative position of "top neighbor" (nbx, nby)
    for (std::size_t i = 0; i < size; ++i)
    {
        //1. find the closest 3 neighbors, for grid structures 2 neighbors should lie on the same axis
        const auto ix = gridPosition[i].first;
        const auto iy = gridPosition[i].second;
        std::vector<int> distance(size);
        std::iota(distance.begin(), distance.end(), 0);
        std::partial_sort(distance.begin(), distance.begin() + 4, distance.end(), [&](int a, int b) {
            const auto distanceA = std::abs(gridPosition[a].first - ix) + std::abs(gridPosition[a].second - iy);
            const auto distanceB = std::abs(gridPosition[b].first - ix) + std::abs(gridPosition[b].second - iy);
            return distanceA < distanceB;
        });

        const auto p1 = distance.at(1);
        const auto p2 = distance.at(2);
        const auto p3 = distance.at(3);

        //2. determine the 2 neighbors such that they do not lie on the same axis
        double nax = gridPosition[p1].first - gridPosition[i].first;
        double nay = gridPosition[p1].second - gridPosition[i].second;
        double nbx = gridPosition[p2].first - gridPosition[i].first;
        double nby = gridPosition[p2].second - gridPosition[i].second;
        const double ncx = gridPosition[p3].first - gridPosition[i].first;
        const double ncy = gridPosition[p3].second - gridPosition[i].second;

        if (std::abs(nax * nbx + nay * nby) > std::abs(nax * ncx + nay * ncy))
        {
            nbx = ncx;
            nby = ncy;
        }

        // 3. set the neighbor close to the x-axis as na, and the neighbor close
        // to the y-axis as nb.
        if (std::abs(nax) < std::abs(nbx))
        {
            std::swap(nax, nbx);
            std::swap(nay, nby);
        }

        if (nax < 0)
        {
            nax = -nax;
            nay = -nay;
        }

        if (nby < 0)
        {
            nbx = -nbx;
            nby = -nby;
        }

        vnax[i] = nax;
        vnay[i] = nay;
        vnbx[i] = nbx;
        vnby[i] = nby;
    }

    const auto n = size / 2;
    std::nth_element(vnax.begin(), vnax.begin() + n, vnax.end());
    std::nth_element(vnay.begin(), vnay.begin() + n, vnay.end());
    std::nth_element(vnbx.begin(), vnbx.begin() + n, vnbx.end());
    std::nth_element(vnby.begin(), vnby.begin() + n, vnby.end());

    const auto nax = vnax.at(n);
    const auto nay = vnay.at(n);
    const auto nbx = vnbx.at(n);
    const auto nby = vnby.at(n);

    const auto detN = nax * nby - nay * nbx;

    std::vector<std::pair<double, double>> gridCoordinate(size);

    // Use the median of the relative position na and nb to predict the world
    // coordinate of each pixel position
    for (std::size_t i = 0; i < size; ++i)
    {
        const auto& position = gridPosition[i];
        const auto ix = position.first;
        const auto iy = position.second;
        gridCoordinate[i] =
        {
            std::round((nby * ix - nbx * iy) / detN) * gridLength * (invertedX ? -1 : 1),
            std::round((nax * iy - nay * ix) / detN) * gridLength * (invertedY ? -1 : 1)
        };
    }

    return gridCoordinate;
}

std::vector<double> optimizeDistortionCoefficient(const std::vector<std::pair<int, int>>&
gridPixelPosition, const std::vector<std::pair<double, double>>& gridWorldPosition)
{
    const auto count = gridPixelPosition.size();

    auto A = new double [count][6];
    double B[6][6];
    double inverseB[6][6];
    auto D = new double [count][6];
    double x[6];

    std::vector<double> gridDistortionCoefficient(12);

    // optimize ax, bx, cx, dx, ex, fx

    for (std::size_t i = 0; i < count; i++)
    {
        const auto u = gridWorldPosition[i].first;
        const auto v = gridWorldPosition[i].second;
        A[i][0] = u;
        A[i][1] = v;
        A[i][2] = u * u;
        A[i][3] = v * v;
        A[i][4] = u * v;
        A[i][5] = u * (u * u + v * v);
    }

    for (std::size_t i = 0; i < 6; ++i)
    {
        for (std::size_t j = i; j < 6; ++j)
        {
            B[i][j] = 0;
            for (std::size_t k = 0; k < count; ++k)
            {
                B[i][j] += A[k][i] * A[k][j];
            }
            B[j][i] = B[i][j];
        }
    }

    matrixInverse(B, inverseB);

    for (std::size_t i = 0; i < 6; ++i)
    {
        for (std::size_t j = 0; j < count; ++j)
        {
            D[j][i] = 0;
            for (std::size_t k = 0; k < 6; ++k)
            {
                D[j][i] += inverseB[i][k] * A[j][k];
            }
        }
    }

    for (std::size_t i = 0; i < 6; ++i)
    {
        x[i] = 0;
        for (std::size_t j = 0; j < count; ++j)
        {
            x[i] += D[j][i] * gridPixelPosition[j].first;
        }
        gridDistortionCoefficient[i] = x[i];
    }


    // optimize ay, by, cy, dy, ey, fy

    for (std::size_t i = 0; i < count; i++)
    {
        const auto u = gridWorldPosition[i].first;
        const auto v = gridWorldPosition[i].second;
        A[i][0] = v;
        A[i][1] = u;
        A[i][2] = v * v;
        A[i][3] = u * u;
        A[i][4] = u * v;
        A[i][5] = v * (u * u + v * v);
    }

    for (std::size_t i = 0; i < 6; ++i)
    {
        for (std::size_t j = i; j < 6; ++j)
        {
            B[i][j] = 0;
            for (std::size_t k = 0; k < count; ++k)
            {
                B[i][j] += A[k][i] * A[k][j];
            }
            B[j][i] = B[i][j];
        }
    }

    matrixInverse(B, inverseB);

    for (std::size_t i = 0; i < 6; ++i)
    {
        for (std::size_t j = 0; j < count; ++j)
        {
            D[j][i] = 0;
            for (std::size_t k = 0; k < 6; ++k)
            {
                D[j][i] += inverseB[i][k] * A[j][k];
            }
        }
    }

    for (std::size_t i = 0; i < 6; ++i)
    {
        x[i] = 0;
        for (std::size_t j = 0; j < count; ++j)
        {
            x[i] += D[j][i] * gridPixelPosition[j].second;
        }
        gridDistortionCoefficient[i + 6] = x[i];
    }

    delete[] A;
    delete[] D;
    return gridDistortionCoefficient;
}

std::vector<double> optimizeDistortionCoefficient(const std::vector<std::pair<int, int>>&
gridPixelPosition, const std::vector<std::pair<double, double>>& gridWorldPosition, double threshold, int iteration)
{
    auto size = gridPixelPosition.size();
    int maxCount = 0;
    const unsigned int minSamples = 6;
    std::vector<double> maxModel(12);

    if (size < minSamples)
    {
        return maxModel;
    }

    std::vector<int> randomIndices(size);
    std::iota(randomIndices.begin(), randomIndices.end(), 0);
    for (int it = 0; it < iteration; ++it)
    {
        std::random_shuffle(randomIndices.begin(), randomIndices.end());
        std::vector<std::pair<int, int>> gridPixelPositionSample(minSamples);
        std::vector<std::pair<double, double>> gridWorldPositionSample(minSamples);
        for (std::size_t i = 0; i < minSamples; ++i)
        {
            gridPixelPositionSample[i] = gridPixelPosition[randomIndices[i]];
            gridWorldPositionSample[i] = gridWorldPosition[randomIndices[i]];
        }
        const auto sampleModel = optimizeDistortionCoefficient(gridPixelPositionSample, gridWorldPositionSample);

        int count = 0;

        for (std::size_t i = 0; i < size; ++i)
        {
            const auto pixel = worldToPixel(gridWorldPosition[i].first, gridWorldPosition[i].second, sampleModel);
            if ((std::abs(pixel.first - gridPixelPosition[i].first) + std::abs(pixel.second - gridPixelPosition[i].second)) < threshold)
            {
                ++count;
            }
        }

        if (count > maxCount)
        {
            maxCount = count;
            maxModel = sampleModel;
        }

    }

    std::vector<std::pair<int, int>> validGridPixelPosition;
    std::vector<std::pair<double, double>> validGridWorldPosition;

    for (std::size_t i = 0; i < size; ++i)
    {
        const auto pixel = worldToPixel(gridWorldPosition[i].first, gridWorldPosition[i].second, maxModel);
        if ((std::abs(pixel.first - gridPixelPosition[i].first) + std::abs(pixel.second - gridPixelPosition[i].second)) < threshold)
        {
            validGridPixelPosition.emplace_back(gridPixelPosition[i]);
            validGridWorldPosition.emplace_back(gridWorldPosition[i]);
        }
    }

    return optimizeDistortionCoefficient(validGridPixelPosition, validGridWorldPosition);
}

double constantRegression(const std::vector<double>& y)
{
    double sum_1 = y.size();
    double sum_y = 0.0;

    for (std::size_t i = 0; i < y.size(); ++i)
    {
        sum_y += y[i];
    }

    double k = sum_y / sum_1;

    return k;
}

double linearRegression(const std::vector<double>& x, const std::vector<double>& y)
{
    double sum_x2 = 0.0;
    double sum_x_y = 0.0;

    for (std::size_t i = 0; i < x.size(); ++i)
    {
        sum_x2 += x[i] * x[i];
        sum_x_y += x[i] * y[i];
    }

    const double det = sum_x2;

    if (std::abs(det) < 1e-16)
    {
        return constantRegression(y);
    }

    double k = sum_x_y / det;

    return k;
}

std::pair<double, double> linearOffsetRegression(const std::vector<double>& x, const std::vector<double>& y)
{
    double sum_1 = x.size();
    double sum_x = 0.0;
    double sum_x2 = 0.0;
    double sum_x_y = 0.0;
    double sum_y = 0.0;

    for (std::size_t i = 0; i < x.size(); ++i)
    {
        sum_x += x[i];
        sum_x2 += x[i] * x[i];
        sum_x_y += x[i] * y[i];
        sum_y += y[i];
    }

    const double det = sum_1 * sum_x2 - sum_x * sum_x;

    if (std::abs(det) < 1e-16)
    {
        return {constantRegression(y), 0};
    }

    double k1 = (sum_x2 * sum_y - sum_x * sum_x_y) / det;
    double k2 = (-sum_x * sum_y + sum_1 * sum_x_y) / det;

    return {k1, k2};
}

std::pair<double, double> ka(const std::vector<double>& scannerPositionX, const std::vector<double>& scannerPositionY, const std::vector<double>& a)
{
    std::vector<double> s2(scannerPositionX.size());

    for (std::size_t i = 0; i < scannerPositionX.size(); ++i)
    {
        const auto sx = scannerPositionX[i];
        const auto sy = scannerPositionY[i];
        s2[i] = sx * sx + sy * sy;
    }

    return linearOffsetRegression(s2, a);
}

std::pair<double, double> kb(const std::vector<double>& scannerPositionX, const std::vector<double>& scannerPositionY, const std::vector<double>& b)
{
    std::vector<double> s2(scannerPositionX.size());

    for (std::size_t i = 0; i < scannerPositionX.size(); ++i)
    {
        const auto sx = scannerPositionX[i];
        const auto sy = scannerPositionY[i];
        s2[i] = sx * sy;
    }

    return linearOffsetRegression(s2, b);
}

std::vector<double> scannerPositionToDistortionCoefficient(double sx, double sy, const std::vector<double>& k)
{
    const auto kax1 = k[0];
    const auto kax2 = k[1];
    const auto kbx1 = k[2];
    const auto kbx2 = k[3];
    const auto kcx = k[4];
    const auto kdx = k[5];
    const auto kex = k[6];
    const auto kfx = k[7];
    const auto kay1 = k[8];
    const auto kay2 = k[9];
    const auto kby1 = k[10];
    const auto kby2 = k[11];
    const auto kcy = k[12];
    const auto kdy = k[13];
    const auto key = k[14];
    const auto kfy = k[15];

    const auto ax = kax1 + kax2 * (sx * sx + sy * sy);
    const auto bx = kbx1 + kbx2 * sx * sy;
    const auto cx = kcx * sx;
    const auto dx = kdx * sx;
    const auto ex = kex * sy;
    const auto fx = kfx;
    const auto ay = kay1 + kay2 * (sx * sx + sy * sy);
    const auto by = kby1 + kby2 * sx * sy;
    const auto cy = kcy * sy;
    const auto dy = kdy * sy;
    const auto ey = key * sx;
    const auto fy = kfy;

    return {ax, bx, cx, dx, ex, fx, ay, by, cy, dy, ey, fy};
}

std::pair<double, double> worldToPixel(double u, double v, const std::vector<double>& distortionCoefficient)
{
    const auto ax = distortionCoefficient[0];
    const auto bx = distortionCoefficient[1];
    const auto cx = distortionCoefficient[2];
    const auto dx = distortionCoefficient[3];
    const auto ex = distortionCoefficient[4];
    const auto fx = distortionCoefficient[5];
    const auto ay = distortionCoefficient[6];
    const auto by = distortionCoefficient[7];
    const auto cy = distortionCoefficient[8];
    const auto dy = distortionCoefficient[9];
    const auto ey = distortionCoefficient[10];
    const auto fy = distortionCoefficient[11];

    const auto x = ax * u + bx * v + cx * u * u + dx * v * v + ex * u * v + fx * u * (u * u + v * v);
    const auto y = ay * v + by * u + cy * v * v + dy * u * u + ey * u * v + fy * v * (u * u + v * v);

    return {x, y};
}

std::pair<double, double> pixelToWorld(double x, double y, const std::vector<double>& distortionCoefficient)
{
    const auto ax = distortionCoefficient[0];
    const auto bx = distortionCoefficient[1];
    const auto cx = distortionCoefficient[2];
    const auto dx = distortionCoefficient[3];
    const auto ex = distortionCoefficient[4];
    const auto fx = distortionCoefficient[5];
    const auto ay = distortionCoefficient[6];
    const auto by = distortionCoefficient[7];
    const auto cy = distortionCoefficient[8];
    const auto dy = distortionCoefficient[9];
    const auto ey = distortionCoefficient[10];
    const auto fy = distortionCoefficient[11];

    auto u = x / ax;
    auto v = y / ay;

    const int maxRepitition = 10;
    for (int i = 0; i < maxRepitition; ++i)
    {
        const auto f1 = ax * u + bx * v + cx * u * u + dx * v * v + ex * u * v + fx * u * (u * u + v * v) - x;
        const auto f2 = ay * v + by * u + cy * v * v + dy * u * u + ey * u * v + fy * v * (u * u + v * v) - y;
        const auto D11 = ax + 2 * cx * u + ex * v + fx * (3 * u * u + v * v);
        const auto D12 = bx + 2 * dx * v + ex * u + fx * (2 * u * v);
        const auto D21 = by + 2 * dy * u + ey * v + fy * (2 * u * v);
        const auto D22 = ay + 2 * cy * v + ey * u + fy * (3 * v * v + u * u);

        const auto detD = D11 * D22 - D12 * D21;
        const auto du = -(D22 * f1 - D12 * f2) / detD;
        const auto dv = -(-D21 * f1 + D11 * f2) / detD;

        u = u + du;
        v = v + dv;

        const double epsilon = 1e-6;
        if (std::abs(du) < epsilon && std::abs(dv) < epsilon)
        {
            break;
        }
    }
    return {u, v};
}

std::pair<double, double> imageShift(const uint8_t *imageA, const uint8_t *imageB, int height, int width, std::ptrdiff_t strideA, std::ptrdiff_t strideB)
{
    cv::Mat cvImageA(height, width, CV_8UC1, (void *)imageA, strideA);
    cv::Mat cvImageB(height, width, CV_8UC1, (void *)imageB, strideB);

    cv::Mat cvImageAf;
    cv::Mat cvImageBf;
    cvImageA.convertTo(cvImageAf, CV_64F);
    cvImageB.convertTo(cvImageBf, CV_64F);

    cv::Mat cvFftA;
    cv::Mat cvFftB;
    cv::dft(cvImageAf, cvFftA);
    cv::dft(cvImageBf, cvFftB);

    cv::Mat cvXcor;
    cv::mulSpectrums(cvFftB, cvFftA, cvXcor, 0, true);
    cv::dft(cvXcor, cvXcor, cv::DFT_INVERSE | cv::DFT_SCALE);

    cv::Point maxLocation;
    cv::minMaxLoc(cvXcor, nullptr, nullptr, nullptr, &maxLocation);

    cv::normalize(cvXcor, cvXcor, 0, 1, cv::NORM_MINMAX);

    const double colShift = maxLocation.x < width / 2 ? maxLocation.x : maxLocation.x - width;
    const double rowShift = maxLocation.y < height / 2 ? maxLocation.y : maxLocation.y - height;

    return {colShift, rowShift};
}

void preprocessMinMax(const double *image, int height, int width, std::ptrdiff_t stride, uint8_t *minMax)
{
    const cv::Mat cvImage(height, width, CV_64FC1, (void *)(image), static_cast<std::size_t>(stride));
    cv::Mat cvMinMax{height, width, CV_8UC1, static_cast<void *>(minMax)};

    const auto element = cv::getStructuringElement(cv::MORPH_RECT, {3, 1}); //[1 1 1] structuring element

    cv::Mat dilateImage;
    cv::Mat erodeImage;
    cv::dilate(cvImage, dilateImage, element);
    cv::erode(cvImage, erodeImage, element);

    // ((D == I) | (E == I)) & (D != E)
    cv::bitwise_and(
        (dilateImage == cvImage) | (erodeImage == cvImage),
        dilateImage != erodeImage, cvMinMax);
}

static std::vector<int> localMaximum(const int *minMaxIndex, const double *minMaxValue, int minMaxCount, double thresholdLeft, double thresholdRight)
{
    std::vector<uint8_t> valid(minMaxCount, 1);

    if (minMaxCount > 0)
    {
        valid[0] = 0;
        valid[minMaxCount - 1] = 0;
    }

    for (int i = 0; i < minMaxCount; ++i)
    {
        if (!valid[i])
        {
            continue;
        }

        int j = i + 1;
        for (; j < minMaxCount; ++j)
        {
            if (minMaxValue[j] < minMaxValue[i] + thresholdRight)
            {
                for (int k = i + 1; k <= j; ++k)
                {
                    valid[k] &= (minMaxValue[k] > minMaxValue[i]);
                    valid[i] &= (minMaxValue[k] <= minMaxValue[i]);
                }
                break;
            }
        }

        if (j == minMaxCount)
        {
            valid[i] = 0;
        }

        if (!valid[i])
        {
            continue;
        }

        j = i - 1;
        for (; j >= 0; --j)
        {
            if (minMaxValue[j] < minMaxValue[i] + thresholdLeft)
            {
                for (int k = i - 1; k >= j; --k)
                {
                    valid[k] &= (minMaxValue[k] >= minMaxValue[i]);
                    valid[i] &= (minMaxValue[k] < minMaxValue[i]);
                }
                break;
            }
        }

        if (j == -1)
        {
            valid[i] = 0;
        }
    }

    std::vector<int> index;

    for (int i = 0; i < minMaxCount; ++i)
    {
        if (valid[i])
        {
            index.emplace_back(minMaxIndex[i]);
        }
    }

    return index;
}

static std::vector<int> localMaximum(const double *line, const uint8_t *minMax, int width, double thresholdLeft, double thresholdRight)
{
    auto minMaxIndex = new int [width];
    auto minMaxValue = new double [width];
    int minMaxCount = 0;

    for (int i = 0; i < width; ++i)
    {
        if (minMax[i] > 0)
        {
            minMaxIndex[minMaxCount] = i;
            minMaxValue[minMaxCount] = line[i];
            ++minMaxCount;
        }
    }

    const auto index = localMaximum(minMaxIndex, minMaxValue, minMaxCount, thresholdLeft, thresholdRight);

    delete[] minMaxIndex;
    delete[] minMaxValue;

    return index;
}

int horizontalPeriod(uint8_t *image, int height, int width, std::ptrdiff_t stride)
{
    cv::Mat cvImage(height, width, CV_8UC1, (void *)image, stride);
    cv::Mat cvImageCopy;
    cv::copyMakeBorder(cvImage, cvImageCopy, 0, 0, 0, width, cv::BORDER_CONSTANT, 0);
    cvImageCopy.convertTo(cvImageCopy, CV_64F);
    cv::Mat cvXcor(height, 2 * width, CV_64FC1);

    cv::dft(cvImageCopy, cvXcor, cv::DFT_ROWS);
    cv::mulSpectrums(cvXcor, cvXcor, cvXcor, cv::DFT_ROWS, true);
    cv::dft(cvXcor, cvXcor, cv::DFT_ROWS | cv::DFT_INVERSE | cv::DFT_SCALE, cvXcor.rows);

    cv::Mat cvSum;
    cv::reduce(cvXcor, cvSum, 0, cv::REDUCE_SUM, CV_64F);
    cv::normalize(cvSum, cvSum, 1, 0, cv::NORM_INF);

    cv::Mat cvMinMax(1, cvSum.cols, CV_8UC1);
    preprocessMinMax((double *)cvSum.data, 1, cvSum.cols, cvSum.step, cvMinMax.data);
    const auto index = localMaximum((double *)cvSum.data, cvMinMax.data, cvSum.cols, 0.001, 0.001);

    int iMax = 0;
    double max = 0.0;

    for (const auto i : index)
    {
        if (i < width && cvSum.at<double>(i) > max)
        {
            iMax = i;
            max = cvSum.at<double>(i);
        }
    }

    return iMax;
}

std::pair<float, float> subpixelPeak(cv::Mat image)
{
    const auto rows = image.rows;
    const auto cols = image.cols;
    std::vector<float> A(rows * cols * 6);
    std::vector<float> B(rows * cols);

    for (int j = 0; j < rows; ++j)
    {
        for (int i = 0; i < cols; ++i)
        {
            A[(j * cols + i) * 6 + 0] = 1.0;
            A[(j * cols + i) * 6 + 1] = i;
            A[(j * cols + i) * 6 + 2] = j;
            A[(j * cols + i) * 6 + 3] = i * i;
            A[(j * cols + i) * 6 + 4] = j * j;
            A[(j * cols + i) * 6 + 5] = i * j;

            B[j * cols + i] = image.at<float>(j, i);
        }
    }

    cv::Mat cvA(rows * cols, 6, CV_32F, (void *)A.data());
    cv::Mat cvB(rows * cols, 1, CV_32F, (void *)B.data());

    cv::Mat cvInverseA;
    cv::invert(cvA, cvInverseA, cv::DECOMP_SVD);

    std::vector<float> k(6);
    cv::Mat cvK(6, 1, CV_32FC1, (void *)k.data());
    cvK = cvInverseA * cvB;

    const auto det = 4 * k[3] * k[4] - k[5] * k[5];
    const auto x = std::abs(det) < 1e-16 ? cols / 2 : (k[2] * k[5] - 2 * k[1] * k[4]) / det;
    const auto y = std::abs(det) < 1e-16 ? rows / 2 : (k[1] * k[5] - 2 * k[2] * k[3]) / det;

    // returns the subpixel position relative to image center
    return {x - (rows - 1) / 2.0, y - (cols - 1) / 2.0};
}

std::vector<Corner> findChessBoardCorners(uint8_t *image, int height, int width, std::ptrdiff_t stride, int period)
{
    cv::Mat cvImage(height, width, CV_8UC1, (void*)image, stride);

    // generate template image
    // 1 1 0 0
    // 1 1 0 0
    // 0 0 1 1
    // 0 0 1 1
    const int templateHalfSize = std::round(period / 16.0);
    const auto templateSize = templateHalfSize * 2;

    auto *templateImage = new uint8_t [templateSize * templateSize];
    memset(templateImage, 0, templateSize * templateSize);

    for (int i = 0; i < templateHalfSize; ++i)
    {
        memset(templateImage + i * templateSize, 255, templateHalfSize);
        memset(templateImage + (i + templateHalfSize) * templateSize + templateHalfSize, 255, templateHalfSize);
    }

    const cv::Mat cvTemplateImage(templateSize, templateSize, CV_8UC1, (void*)templateImage);

    cv::Mat cvXcor;
    cv::matchTemplate(cvImage, cvTemplateImage, cvXcor, cv::TM_CCOEFF_NORMED);

    cv::Mat cvAbsXcor;
    cvAbsXcor = cv::abs(cvXcor);

    const int dilateSize = period * 0.8;
    cv::Mat cvDilateImage;
    const cv::Mat cvDilateElement = cv::getStructuringElement(cv::MORPH_RECT, {dilateSize, dilateSize});
    cv::dilate(cvAbsXcor, cvDilateImage, cvDilateElement);

    cv::Mat cvPeakImage = (cvDilateImage == cvAbsXcor);

    std::vector<cv::Point2i> peakLocation;
    cv::findNonZero(cvPeakImage, peakLocation);

    // corner type
    // type 0:
    // 0 1
    // 1 0
    // type 1:
    // 1 0
    // 0 1
    std::vector<Corner> corner;
    corner.reserve(peakLocation.size());
    for(auto peak : peakLocation)
    {
        // subpixel interpolation
        double dx = 0;
        double dy = 0;
        if (peak.x >= 2 && peak.x <= cvAbsXcor.cols - 3 && peak.y >= 2 && peak.y <= cvAbsXcor.rows - 3)
        {
            cv::Mat cvRoi(cvAbsXcor, cv::Rect(peak.x - 2, peak.y - 2, 5, 5));
            const auto subpixelLocation = subpixelPeak(cvRoi);
            dx = subpixelLocation.first;
            dy = subpixelLocation.second;
        }

        const auto x = (double)peak.x + dx + templateHalfSize - 0.5;
        const auto y = (double)peak.y + dy + templateHalfSize - 0.5;

        if (x > templateHalfSize && x < width - 1 - templateHalfSize && y > templateHalfSize && y < height - 1 - templateHalfSize)
        {
            corner.push_back({x, y, cvXcor.at<float>(peak) > 0.0});
        }
    }

    return corner;
}

std::vector<Corner> findSquares(const std::vector<Corner>& corner, int period)
{
    std::vector<Corner> square; // 4xN corners

    for (std::size_t i = 0; i < corner.size(); ++i)
    {
        if (corner[i].cornerType == false) // corner type 0
        {
            const auto x = corner[i].x;
            const auto y = corner[i].y;

            const auto left = x - period * 0.25;
            const auto right = x + period * 0.75;
            const auto top = y - period * 0.25;
            const auto bottom = y + period * 0.75;

            std::vector<Corner> neighbor;
            std::copy_if(corner.begin(), corner.end(), std::back_inserter(neighbor), [&](auto c)
            {
                return c.x >= left && c.x <= right && c.y >= top && c.y <= bottom && (c.x != x || c.y != y);
            });

            if (neighbor.size() == 3 &&
                std::count_if(neighbor.begin(), neighbor.end(), [](auto& n) {return n.cornerType == false;}) == 1)
            {
                // sort neighbors in clockwise order
                std::sort(neighbor.begin(), neighbor.end(), [&](auto& lhs, auto& rhs)
                {
                    return (lhs.x - x) * (rhs.y - y) > (lhs.y - y) * (rhs.x - x);
                });

                square.emplace_back(corner[i]);
                square.insert(square.end(), neighbor.begin(), neighbor.end());
            }
        }
    }

    return square;
}

std::vector<std::pair<double, double>> analyseSquare(uint8_t *image, int height, int width, std::ptrdiff_t stride, const std::vector<Corner> square, double squareSideMm, int minSpotSize, int margin)
{
    const auto p1 = square[0];
    const auto p2 = square[1];
    const auto p3 = square[2];
    const auto p4 = square[3];

    const auto cx = (p1.x + p2.x + p3.x + p4.x) / 4;
    const auto cy = (p1.y + p2.y + p3.y + p4.y) / 4;

    const auto ux = (p2.x - p1.x + p3.x - p4.x) / 2;
    const auto uy = (p2.y - p1.y + p3.y - p4.y) / 2;
    const auto vx = (p3.x - p2.x + p4.x - p1.x) / 2;
    const auto vy = (p3.y - p2.y + p4.y - p1.y) / 2;
    const auto det = ux * vy - vx * uy;

    std::vector<std::size_t> index = {0, 1, 2, 3};
    std::sort(std::begin(index), std::end(index),
            [&square](const auto& lhs, const auto& rhs)
            {
                return square[lhs].x < square[rhs].x;
            });

    const int left = square[index[1]].x + margin;
    const int right = square[index[2]].x - margin;
    const int roiWidth = right >= left ? right - left + 1 : 0;

    std::sort(std::begin(index), std::end(index),
            [&square](const auto& lhs, const auto& rhs)
            {
                return square[lhs].y < square[rhs].y;
            });

    const int top = square[index[1]].y + margin;
    const int bottom = square[index[2]].y - margin;
    const int roiHeight = bottom >= top ? bottom - top + 1 : 0;

    cv::Mat cvImage(height, width, CV_8UC1, (void *)image, stride);
    cv::Mat cvRoi(cvImage, cv::Rect(left, top, roiWidth, roiHeight));

    cv::Mat cvGauss;
    cv::GaussianBlur(cvRoi, cvGauss, {3, 3}, 3);

    cv::Mat cvHistogram;
    const int histogramSize = 256;
    const float histogramRange[] = {0, histogramSize};
    const float* ranges[] = {histogramRange};
    cv::calcHist(&cvGauss, 1, 0, cv::Mat(), cvHistogram, 1, &histogramSize, ranges, true, false);

    int median = histogramSize - 1;
    for (int i = 1; i < histogramSize; ++i)
    {
        cvHistogram.at<float>(i) += cvHistogram.at<float>(i - 1);
        if (cvHistogram.at<float>(i) > 0.5 * roiHeight * roiWidth)
        {
            median = i - 1;
            break;
        }
    }

    double threshold = median * 1.5;
    cv::Mat cvBinary;
    cv::threshold(cvGauss, cvBinary, threshold, 255, cv::THRESH_BINARY);
    const cv::Mat cvElement = cv::getStructuringElement(cv::MORPH_RECT, {5, 5});
    cv::morphologyEx(cvBinary, cvBinary, cv::MORPH_CLOSE, cvElement);

    cv::Mat cvLabels;
    cv::Mat cvStats;
    cv::Mat cvCentroids;
    cv::connectedComponentsWithStats(cvBinary, cvLabels, cvStats, cvCentroids);

    std::vector<std::pair<double, double>> result;
    result.emplace_back(cx, cy);

    std::vector<std::size_t> areaIndex(cvCentroids.rows - 1); // first row (background) excluded
    std::iota(areaIndex.begin(), areaIndex.end(), 1);

    std::sort(std::begin(areaIndex), std::end(areaIndex),
            [&cvStats](const auto& lhs, const auto& rhs)
            {
                const auto lhsArea = cvStats.at<int32_t>(lhs, cv::CC_STAT_AREA);
                const auto rhsArea = cvStats.at<int32_t>(rhs, cv::CC_STAT_AREA);
                return lhsArea > rhsArea;
            });

    for (const auto i : areaIndex)
    {
        const auto area = cvStats.at<int32_t>(i, cv::CC_STAT_AREA);
        if (area >= minSpotSize)
        {
            const auto x = cvCentroids.at<double>(i, 0) + left;
            const auto y = cvCentroids.at<double>(i, 1) + top;
            const auto dx = x - cx;
            const auto dy = y - cy;
            const auto dxMm = (vy * dx - vx * dy) / det * squareSideMm;
            const auto dyMm = (ux * dy - uy * dx) / det * squareSideMm;

            result.emplace_back(x, y);
            result.emplace_back(dxMm, dyMm);
        }
        else
        {
            break;
        }
    }

    return result;
}

std::vector<std::vector<std::pair<double, double>>> findAnalyseSquares(uint8_t *image, int height, int width, std::ptrdiff_t stride, int period)
{
    const auto corner = findChessBoardCorners(image, height, width, stride, period);
    const auto square = findSquares(corner, period);

    std::vector<std::vector<std::pair<double, double>>> imageResult;

    const std::size_t N = square.size() / 4;

    const double squareSizeMm = 5.0;
    const int margin = std::ceil(period / 64.0);
    const double minSpotDiameterMm = 70e-3;
    const int minSpotAreaPx2 = minSpotDiameterMm * minSpotDiameterMm * period * period / squareSizeMm / squareSizeMm / 4.0;

    for (std::size_t i = 0; i < N; ++i)
    {
        const auto result = analyseSquare(image, height, width, stride, {square.begin() + i * 4, square.begin() + (i + 1) * 4}, squareSizeMm, minSpotAreaPx2, margin);
        imageResult.emplace_back(result);
    }

    return imageResult;
}

std::vector<float> generateLoGKernel(float sigma, int kernelSize)
{
    std::vector<float> LoG(kernelSize * kernelSize);

    const float a = 1 / (2 * sigma * sigma);
    const float norm = std::exp(1.0) / M_PI * a; // normalize such that the sum of positive part is 1.0

    float sum = 0;

    for (int j = 0; j < kernelSize; ++j)
    {
        const float y2 = (j - kernelSize / 2) * (j - kernelSize / 2);
        for (int i = 0; i < kernelSize; ++i)
        {
            const float x2 = (i - kernelSize / 2) * (i - kernelSize / 2);
            LoG[j * kernelSize + i] = norm * (1 - a * (x2 + y2)) * std::exp(-a * (x2 + y2));

            sum += LoG[j * kernelSize + i];
        }
    }

    const float avg = sum / LoG.size();

    for (auto& p : LoG)
    {
        p = p - avg;
    }

    return LoG;
}

std::vector<std::pair<double, double>> analyseSquareLoG(cv::Mat cvImage, const std::vector<Corner> square, double squareSideMm, int margin)
{
    const auto p1 = square[0];
    const auto p2 = square[1];
    const auto p3 = square[2];
    const auto p4 = square[3];

    const auto cx = (p1.x + p2.x + p3.x + p4.x) / 4;
    const auto cy = (p1.y + p2.y + p3.y + p4.y) / 4;

    const auto ux = (p2.x - p1.x + p3.x - p4.x) / 2;
    const auto uy = (p2.y - p1.y + p3.y - p4.y) / 2;
    const auto vx = (p3.x - p2.x + p4.x - p1.x) / 2;
    const auto vy = (p3.y - p2.y + p4.y - p1.y) / 2;
    const auto det = ux * vy - vx * uy;

    std::vector<std::size_t> index = {0, 1, 2, 3};
    std::sort(std::begin(index), std::end(index),
            [&square](const auto& lhs, const auto& rhs)
            {
                return square[lhs].x < square[rhs].x;
            });

    const int left = square[index[1]].x + margin;
    const int right = square[index[2]].x - margin;
    const int roiWidth = right >= left ? right - left + 1 : 0;

    std::sort(std::begin(index), std::end(index),
            [&square](const auto& lhs, const auto& rhs)
            {
                return square[lhs].y < square[rhs].y;
            });

    const int top = square[index[1]].y + margin;
    const int bottom = square[index[2]].y - margin;
    const int roiHeight = bottom >= top ? bottom - top + 1 : 0;

    cv::Mat cvRoi(cvImage, cv::Rect(left, top, roiWidth, roiHeight));

    double maxValue;
    cv::Point maxLocation;
    cv::minMaxLoc(cvRoi, nullptr, &maxValue, nullptr, &maxLocation);

    cv::Mat cvRoi2(cvImage, cv::Rect(left + maxLocation.x - 2, top + maxLocation.y - 2, 5, 5));
    const auto subpixelLocation = subpixelPeak(cvRoi2);

    std::vector<std::pair<double, double>> result;
    result.emplace_back(cx, cy);

    if (maxValue < 5)
    {
        return result;
    }

    const auto x = subpixelLocation.first + maxLocation.x + left;
    const auto y = subpixelLocation.second + maxLocation.y + top;
    const auto dx = x - cx;
    const auto dy = y - cy;
    const auto dxMm = (vy * dx - vx * dy) / det * squareSideMm;
    const auto dyMm = (ux * dy - uy * dx) / det * squareSideMm;

    result.emplace_back(x, y);
    result.emplace_back(dxMm, dyMm);

    return result;
}

std::vector<std::vector<std::pair<double, double>>> findAnalyseSquaresLoG(uint8_t *image, int height, int width, std::ptrdiff_t stride, int period)
{
    const auto corner = findChessBoardCorners(image, height, width, stride, period);
    const auto square = findSquares(corner, period);

    std::vector<std::vector<std::pair<double, double>>> imageResult;

    const double squareSideMm = 5.0;
    const int margin = std::ceil(period / 64.0);

    const float spotDiameterMm = 500e-3f; // for calculation of sigma parameter of LoG. Assume constant spot size of 500um because the LoG filter is less sensitve to spot size.
    const float spotDiameterPx = spotDiameterMm * period / (squareSideMm * 2);
    const int kernelSize = (int)spotDiameterPx * 2 + 1; // kernel size is 2x the spot size
    const float sigma = spotDiameterPx / (2 * std::sqrt(2)); // D = 2 * sqrt(2) * sigma

    const auto LoGKernel = generateLoGKernel(sigma, kernelSize);
    cv::Mat cvLoGKernel(kernelSize, kernelSize, CV_32F, (void *)LoGKernel.data());
    cv::Mat cvImage(height, width, CV_8UC1, (void *)image, stride);

    cv::Mat cvLoG;
    cv::filter2D(cvImage, cvLoG, CV_32F, cvLoGKernel);

    const std::size_t N = square.size() / 4;
    for (std::size_t i = 0; i < N; ++i)
    {
        const auto result = analyseSquareLoG(cvLoG, {square.begin() + i * 4, square.begin() + (i + 1) * 4}, squareSideMm, margin);
        imageResult.emplace_back(result);
    }

    return imageResult;
}

std::vector<double> polyFit2D(const std::vector<double>& x, const std::vector<double>& y, const std::vector<double>& z, int order, double threshold, int iteration)
{
    const int N = x.size();
    const int M = (order + 1) * (order + 2) * 0.5;

    std::vector<double> A(N * M); // N rows, M cols

    const auto xMax = std::abs(*std::max_element(x.begin(), x.end(), [](const auto& lhs, const auto& rhs){return std::abs(lhs) < std::abs(rhs);}));
    const auto yMax = std::abs(*std::max_element(y.begin(), y.end(), [](const auto& lhs, const auto& rhs){return std::abs(lhs) < std::abs(rhs);}));

    for (int i = 0; i < N; ++i)
    {
        const auto xh = x[i] / xMax; // normalize x and y for better numerical accuracy
        const auto yh = y[i] / yMax;
        int j = 0;
        for (int u = 0; u <= order; ++u)
        {
            for (int v = 0; v <= order - u; ++v, ++j)
            {
                const auto xu = std::pow(xh, u);
                const auto yv = std::pow(yh, v);
                A[i * M + j] = xu * yv;
            }
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

    std::vector<double> subA(M * M);
    std::vector<double> subB(M);

    std::vector<uint8_t> maxValid(N);
    int maxValidCount = 0;

    const cv::Mat cvA(N, M, CV_64FC1, (void *)A.data());
    const cv::Mat cvB(N, 1, CV_64FC1, (void *)z.data());
    const cv::Mat cvSubA(M, M, CV_64FC1, (void *)subA.data());
    const cv::Mat cvSubB(M, 1, CV_64FC1, (void *)subB.data());
    cv::Mat cvValid(N, 1, CV_8UC1);

    for (int i = 0; i < iteration; ++i)
    {
        // prepare submatrix by copying random rows from A to subA
        for (int j = 0; j < M; ++j)
        {
            const auto index = randomIndices[i * M + j];
            std::memcpy(&subA[j * M], &A[index * M], sizeof(*A.data()) * M);
            subB[j] = z[index];
        }

        cvValid = cv::abs(cvA * cvSubA.inv() * cvSubB - cvB) <= threshold;
        const auto validCount = cv::countNonZero(cvValid);

        if (validCount > maxValidCount)
        {
            maxValidCount = validCount;
            cv::Mat cvMaxValid{N, 1, CV_8UC1, (void *)maxValid.data()};
            cvValid.copyTo(cvMaxValid);
        }
    }

    if (maxValidCount < M)
    {
        return z;
    }

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

    const cv::Mat cvModel = (cvSubAValid.t() * cvSubAValid).inv() * cvSubAValid.t() * cvSubBValid;

    std::vector<double> zModel(N);

    cv::Mat cvBModel(N, 1, CV_64FC1, zModel.data());

    cvBModel = cvA * cvModel;

    return zModel;
}

std::vector<double> linearFit2D(const std::vector<double>& x, const std::vector<double>& y, const std::vector<double>& z, double threshold, int iteration)
{
    const int N = x.size();

    if (N < 3)
    {
        return {};
    }

    std::vector<double> A(N * 3); // N rows, 3 cols

    for (int i = 0; i < N; ++i)
    {
        A[i * 3] = 1.0;
        A[i * 3 + 1] = x[i];
        A[i * 3 + 2] = y[i];
    }

    std::vector<int> randomIndices;

    const int n = (3 * iteration) / N + 1;

    for (int i = 0; i < n; ++i)
    {
        std::vector<int> r(N);
        std::iota(r.begin(), r.end(), 0);
        std::mt19937 seed(i);
        std::shuffle(r.begin(), r.end(), seed);
        randomIndices.insert(randomIndices.end(), r.begin(), r.end());
    }

    std::vector<double> subA(3 * 3);
    std::vector<double> subB(3);

    std::vector<uint8_t> maxValid(N);
    int maxValidCount = 0;

    const cv::Mat cvA(N, 3, CV_64FC1, (void *)A.data());
    const cv::Mat cvB(N, 1, CV_64FC1, (void *)z.data());
    const cv::Mat cvSubA(3, 3, CV_64FC1, (void *)subA.data());
    const cv::Mat cvSubB(3, 1, CV_64FC1, (void *)subB.data());
    cv::Mat cvValid(N, 1, CV_8UC1);

    for (int i = 0; i < iteration; ++i)
    {
        // prepare submatrix by copying random rows from A to subA
        for (int j = 0; j < 3; ++j)
        {
            const auto index = randomIndices[i * 3 + j];
            std::memcpy(&subA[j * 3], &A[index * 3], sizeof(*A.data()) * 3);
            subB[j] = z[index];
        }

        cvValid = cv::abs(cvA * cvSubA.inv() * cvSubB - cvB) <= threshold;
        const auto validCount = cv::countNonZero(cvValid);

        if (validCount > maxValidCount)
        {
            maxValidCount = validCount;
            cv::Mat cvMaxValid{N, 1, CV_8UC1, (void *)maxValid.data()};
            cvValid.copyTo(cvMaxValid);
        }
    }

    if (maxValidCount < 3)
    {
        return {};
    }

    cv::Mat cvSubAValid(maxValidCount, 3, CV_64FC1);
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

    const cv::Mat model = (cvSubAValid.t() * cvSubAValid).inv() * cvSubAValid.t() * cvSubBValid;

    return model;
}

std::vector<std::pair<double, double>> postprocessSquareResult(const std::vector<std::vector<std::vector<std::pair<double, double>>>>& data, const std::vector<std::pair<double, double>>& scannerPosition, double TCP0x, double TCP0y)
{
    const auto tolerance = 0.2; // mm
    const auto iteration = 100;
    const auto N = scannerPosition.size();

    if (N < 1)
    {
        return {};
    }

    std::vector<double> dist(N * N);

    for (std::size_t j = 0; j < N; ++j)
    {
        for (std::size_t i = 0; i < N; ++i)
        {
            const auto ix = scannerPosition[i].first;
            const auto iy = scannerPosition[i].second;
            const auto jx = scannerPosition[j].first;
            const auto jy = scannerPosition[j].second;
            const auto dij = std::sqrt((ix - jx) * (ix - jx) + (iy - jy) * (iy - jy));
            dist[j * N + i] = dij;
            dist[i * N + j] = dij;
        }
    }

    std::vector<std::size_t> index(N);
    std::iota(index.begin(), index.end(), 0);
    std::sort(std::begin(index), std::end(index),
            [&](auto lhs, auto rhs)
            {
                const auto lhsx = scannerPosition[lhs].first;
                const auto lhsy = scannerPosition[lhs].second;
                const auto rhsx = scannerPosition[rhs].first;
                const auto rhsy = scannerPosition[rhs].second;
                return lhsx * lhsx + lhsy * lhsy < rhsx * rhsx + rhsy * rhsy;
            });

    // find the square of interest for each image, starting with the center image. The square of interest of the center image is the square closest to TCP0. The square of interest of other images is the square that is nearest to the square of interest of its neighbor images.
    std::vector<std::size_t> center(N);
    for (std::size_t i = 0; i < N; ++i)
    {
        const auto indexI = index[i];

        // search the closest index previous to current index
        std::size_t closestIndex = index[0];
        double TCPx = TCP0x;
        double TCPy = TCP0y;
        for (std::size_t j = 0; j < i; ++j)
        {
            const auto indexJ = index[j];

            if (dist[indexI * N + indexJ] < dist[indexI * N + closestIndex])
            {
                closestIndex = indexJ;
                TCPx = data[closestIndex][center[closestIndex]][0].first;
                TCPy = data[closestIndex][center[closestIndex]][0].second;
            }
        }

        center[indexI] = std::distance(data[indexI].begin(),
                            std::min_element(data[indexI].begin(), data[indexI].end(),
                            [&](const auto& lhs, const auto& rhs)
                            {
                                const auto lhsx = lhs[0].first - TCPx;
                                const auto lhsy = lhs[0].second - TCPy;
                                const auto rhsx = rhs[0].first - TCPx;
                                const auto rhsy = rhs[0].second - TCPy;
                                return lhsx * lhsx + lhsy * lhsy < rhsx * rhsx + rhsy * rhsy;
                            }));
    }

    std::vector<std::pair<double, double>> offsetMm;

    for (std::size_t i = 0; i < N; ++i)
    {
        const int spotCount = (data[i][center[i]].size() - 1) / 2;

        std::vector<double> x;
        std::vector<double> y;
        std::vector<double> dx;
        std::vector<double> dy;

        for (const auto& square : data[i])
        {
            for (std::size_t i = 2; i < square.size(); i = i + 2)
            {
                x.emplace_back(square[0].first);
                y.emplace_back(square[0].second);
                dx.emplace_back(square[i].first);
                dy.emplace_back(square[i].second);
            }
        }

        const auto modelDx = linearFit2D(x, y, dx, tolerance, iteration);
        const auto modelDy = linearFit2D(x, y, dy, tolerance, iteration);

        const auto modelIsValid = modelDx.size() == 3 && modelDy.size() == 3;

        const auto dxModel = modelIsValid ? modelDx[0] + modelDx[1] * data[i][center[i]][0].first + modelDx[2] * data[i][center[i]][0].second : 0;
        const auto dyModel = modelIsValid ? modelDy[0] + modelDy[1] * data[i][center[i]][0].first + modelDy[2] * data[i][center[i]][0].second : 0;

        if (spotCount == 0)
        {
            offsetMm.emplace_back(dxModel, dyModel);
        }
        else
        {
            double dxReal = data[i][center[i]][2].first;
            double dyReal = data[i][center[i]][2].second;


            for (std::size_t j = 2; j < data[i][center[i]].size(); j = j + 2)
            {
                if (std::abs(data[i][center[i]][j].first - dxModel) < std::abs(dxReal - dxModel))
                {
                    dxReal = data[i][center[i]][j].first;
                }
                if (std::abs(data[i][center[i]][j].second - dyModel) < std::abs(dyReal - dyModel))
                {
                    dxReal = data[i][center[i]][j].second;
                }
            }

            // if model is valid but the spot lies to far away from the model, the spot is not plausible
            if (modelIsValid && (std::abs(dxReal - dxModel) > tolerance || std::abs(dyReal - dyModel) > tolerance))
            {
                offsetMm.emplace_back(dxModel, dyModel);
            }
            else // in other cases just use the measured offset
            {
                offsetMm.emplace_back(dxReal, dyReal);
            }
        }
    }

    return offsetMm;
}

std::vector<double> generalizedRansac(const std::vector<double>& A, const std::vector<double>& B, double threshold, int iteration)
{
    const int N = B.size();
    const int M = A.size() / N;

    if (N < M)
    {
        return std::vector<double>(M, 0.0);
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

    std::vector<double> subA(M * M);
    std::vector<double> subB(M);

    std::vector<uint8_t> maxValid(N, 255);
    int maxValidCount = 0;

    const cv::Mat cvA(N, M, CV_64FC1, (void *)A.data());
    const cv::Mat cvB(N, 1, CV_64FC1, (void *)B.data());
    const cv::Mat cvSubA(M, M, CV_64FC1, (void *)subA.data());
    const cv::Mat cvSubB(M, 1, CV_64FC1, (void *)subB.data());
    cv::Mat cvValid(N, 1, CV_8UC1);

    for (int i = 0; i < iteration; ++i)
    {
        // prepare submatrix by copying random rows from A to subA
        for (int j = 0; j < M; ++j)
        {
            const auto index = randomIndices[i * M + j];
            std::memcpy(&subA[j * M], &A[index * M], sizeof(*A.data()) * M);
            subB[j] = B[index];
        }

        cvValid = cv::abs(cvA * cvSubA.inv() * cvSubB - cvB) <= threshold;
        const auto validCount = cv::countNonZero(cvValid);

        if (validCount > maxValidCount)
        {
            maxValidCount = validCount;
            cv::Mat cvMaxValid{N, 1, CV_8UC1, (void *)maxValid.data()};
            cvValid.copyTo(cvMaxValid);
        }
    }

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

    cv::Mat cvInverseSubAValid;
    cv::invert(cvSubAValid, cvInverseSubAValid, cv::DECOMP_SVD);

    std::vector<double> model(M);
    cv::Mat cvModel(M, 1, CV_64FC1, (void *)model.data());
    cvModel = cvInverseSubAValid * cvSubBValid;

    return model;
}

PostprocessResult postprocess(const std::vector<std::pair<double, double>>& scannerPosition, const std::vector<std::vector<Corner>>& scanfieldResult, double period, double TCP0x, double TCP0y, int sgnX, int sgnY)
{
    const double squareSizeMm = 5;
    const auto N = scannerPosition.size();

    // TODO: check input size?

    std::vector<double> Sx(N);
    std::vector<double> Sy(N);
    std::transform(scannerPosition.begin(), scannerPosition.end(), Sx.begin(), [](const auto& pair){return pair.first;});
    std::transform(scannerPosition.begin(), scannerPosition.end(), Sy.begin(), [](const auto& pair){return pair.second;});

    std::vector<double> dist(N * N);

    for (std::size_t j = 0; j < N; ++j)
    {
        for (std::size_t i = 0; i < N; ++i)
        {
            const auto ix = Sx[i];
            const auto iy = Sy[i];
            const auto jx = Sx[j];
            const auto jy = Sy[j];
            const auto dij = std::sqrt((ix - jx) * (ix - jx) + (iy - jy) * (iy - jy));
            dist[j * N + i] = dij;
            dist[i * N + j] = dij;
        }
    }

    std::vector<std::size_t> index(N);
    std::iota(index.begin(), index.end(), 0);
    std::sort(std::begin(index), std::end(index),
            [&](auto lhs, auto rhs)
            {
                const auto lhsx = Sx[lhs];
                const auto lhsy = Sy[lhs];
                const auto rhsx = Sx[rhs];
                const auto rhsy = Sy[rhs];
                return lhsx * lhsx + lhsy * lhsy < rhsx * rhsx + rhsy * rhsy;
            });

    // determine anker corner for scanner zero position. Anker corner is the top left corner of TCP0
    std::vector<std::size_t> anker(N, 0);
    auto dMin = std::numeric_limits<double>::infinity();

    for (std::size_t i = 0; i < scanfieldResult[index[0]].size(); ++i)
    {
        const auto& corner = scanfieldResult[index[0]][i];
        if (corner.cornerType == false && corner.x < TCP0x && corner.y < TCP0y)
        {
            const auto dx = corner.x - TCP0x;
            const auto dy = corner.y - TCP0y;
            const auto d = dx * dx + dy * dy;
            if (d < dMin)
            {
                dMin = d;
                anker[index[0]] = i;
            }
        }
    }

    // determine anker corner for the rest of images
    for (std::size_t i = 1; i < N; ++i)
    {
        const auto indexI = index[i];

        // search the closest index previous to current index
        std::size_t closestIndex = index[0];
        for (std::size_t j = 0; j < i; ++j)
        {
            const auto indexJ = index[j];

            if (dist[indexI * N + indexJ] < dist[indexI * N + closestIndex])
            {
                closestIndex = indexJ;
            }
        }

        const auto closestX = scanfieldResult[closestIndex][anker[closestIndex]].x;
        const auto closestY = scanfieldResult[closestIndex][anker[closestIndex]].y;

        anker[indexI] = std::distance(scanfieldResult[indexI].begin(),
                    std::min_element(scanfieldResult[indexI].begin(), scanfieldResult[indexI].end(),
                    [&](const auto& lhs, const auto& rhs)
                    {
                        const auto lhsx = lhs.x - closestX;
                        const auto lhsy = lhs.y - closestY;
                        const auto rhsx = rhs.x - closestX;
                        const auto rhsy = rhs.y - closestY;
                        return lhsx * lhsx + lhsy * lhsy < rhsx * rhsx + rhsy * rhsy;
                    }));
    }

    // determine square center tcpx,tcpy
    std::vector<double> tcpx(N, 0);
    std::vector<double> tcpy(N, 0);
    for (std::size_t i = 0; i < N; ++i)
    {
        const auto& corners = scanfieldResult[i];
        const auto ankerI = anker[i];
        const auto ankerX = corners[ankerI].x;
        const auto ankerY = corners[ankerI].y;

        const auto left = ankerX - period * 0.25;
        const auto right = ankerX + period * 0.75;
        const auto top = ankerY - period * 0.25;
        const auto bottom = ankerY + period * 0.75;

        std::vector<Corner> neighbor;
        std::copy_if(corners.begin(), corners.end(), std::back_inserter(neighbor), [&](auto c)
        {
            return c.x >= left && c.x <= right && c.y >= top && c.y <= bottom;
        });

        for (const auto& c : neighbor)
        {
            tcpx[i] += c.x;
            tcpy[i] += c.y;
        }

        tcpx[i] /= neighbor.size();
        tcpy[i] /= neighbor.size();
    }

    // combine all data into I, X, Y, U, V;
    std::vector<double> I;
    std::vector<double> X;
    std::vector<double> Y;
    std::vector<double> U;
    std::vector<double> V;

    for (std::size_t i = 0; i < N; ++i)
    {
        const auto& corners = scanfieldResult[i];
        const auto ankerI = anker[i];
        const auto n = corners.size();

        std::vector<double> x(n);
        std::vector<double> y(n);

        for (std::size_t j = 0; j < n; ++j)
        {
            x[j] = corners[j].x - corners[ankerI].x;
            y[j] = corners[j].y - corners[ankerI].y;
        }

        //assign grid coordinates
        std::vector<double> u(n, 0);
        std::vector<double> v(n, 0);

        std::vector<std::size_t> cornerIndex(n);
        std::iota(cornerIndex.begin(), cornerIndex.end(), 0);
        std::sort(std::begin(cornerIndex), std::end(cornerIndex),
                [&](auto lhs, auto rhs)
                {
                    return x[lhs] * x[lhs] + y[lhs] * y[lhs] < x[rhs] * x[rhs] + y[rhs] * y[rhs];
                });

        for (std::size_t j = 1; j < n; ++j)
        {
            const auto cornerIndexJ = cornerIndex[j];
            auto closestIndex = cornerIndex[0];

            for (std::size_t k = 1; k < j; ++k)
            {
                const auto cornerIndexK = cornerIndex[k];
                const auto dx = x[cornerIndexJ] - x[cornerIndexK];
                const auto dy = y[cornerIndexJ] - y[cornerIndexK];
                const auto dxMin = x[cornerIndexJ] - x[closestIndex];
                const auto dyMin = y[cornerIndexJ] - y[closestIndex];

                if (dx * dx + dy * dy < dxMin * dxMin + dyMin * dyMin)
                {
                    closestIndex = cornerIndexK;
                }
            }

            const auto dx = x[cornerIndexJ] - x[closestIndex];
            const auto dy = y[cornerIndexJ] - y[closestIndex];

            u[cornerIndexJ] = u[closestIndex];
            v[cornerIndexJ] = v[closestIndex];

            if (std::abs(dx) > std::abs(dy))
            {
                u[cornerIndexJ] = dx > 0 ? u[cornerIndexJ] + squareSizeMm : u[cornerIndexJ] - squareSizeMm;
            }
            else
            {
                v[cornerIndexJ] = dy > 0 ? v[cornerIndexJ] + squareSizeMm : v[cornerIndexJ] - squareSizeMm;
            }
        }

        for (std::size_t j = 0; j < n; ++j)
        {
            I.emplace_back(i);
            X.emplace_back(corners[j].x - tcpx[i]);
            Y.emplace_back(corners[j].y - tcpy[i]);
            U.emplace_back(sgnX * (u[j] - squareSizeMm * 0.5));
            V.emplace_back(sgnY * (v[j] - squareSizeMm * 0.5));
        }
    }

    const int M = 8;
    std::vector<double> Ax(X.size() * M);
    std::vector<double> Ay(X.size() * M);

    for (std::size_t i = 0; i < X.size(); ++i)
    {
        const auto sx = Sx[I[i]];
        const auto sy = Sy[I[i]];
        const auto s2 = sx * sx + sy * sy;

        const auto u = U[i];
        const auto v = V[i];
        const auto w2 = u * u + v * v;

        Ax[M * i + 0] = u;
        Ax[M * i + 1] = s2 * u;
        Ax[M * i + 2] = v;
        Ax[M * i + 3] = sx * sy * v;
        Ax[M * i + 4] = sx * u * u;
        Ax[M * i + 5] = sx * v * v;
        Ax[M * i + 6] = sy * u * v;
        Ax[M * i + 7] = w2 * u;

        Ay[M * i + 0] = v;
        Ay[M * i + 1] = s2 * v;
        Ay[M * i + 2] = u;
        Ay[M * i + 3] = sx * sy * u;
        Ay[M * i + 4] = sy * v * v;
        Ay[M * i + 5] = sy * u * u;
        Ay[M * i + 6] = sx * u * v;
        Ay[M * i + 7] = w2 * v;
    }

    const double threshold = 120e-3 * period / (squareSizeMm * 2); // 120 um
    const int iteration = 3000;
    const auto modelX = generalizedRansac(Ax, X, threshold, iteration);
    const auto modelY = generalizedRansac(Ay, Y, threshold, iteration);

    // Calculate TCP model values
    const auto polyOrder = 5;
    const auto thresholdTCPFit = period / 8;
    const auto iterationTCPFit = 20000;

    const auto tcpxModel = polyFit2D(Sx, Sy, tcpx, polyOrder, thresholdTCPFit, iterationTCPFit);
    const auto tcpyModel = polyFit2D(Sx, Sy, tcpy, polyOrder, thresholdTCPFit, iterationTCPFit);

    return {tcpxModel[index[0]], tcpyModel[index[0]], tcpxModel, tcpyModel, modelX, modelY};
}
