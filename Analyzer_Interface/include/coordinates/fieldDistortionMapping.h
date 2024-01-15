#pragma once

#include <cstddef>
#include <cstdint>
#include <utility>
#include <vector>

struct Corner
{
    double x;
    double y;
    bool cornerType;
};

struct PostprocessResult
{
    double tcp0x;
    double tcp0y;
    std::vector<double> tcpx;
    std::vector<double> tcpy;
    std::vector<double> modelx;
    std::vector<double> modely;
};

/**
Finds all grid intersections in an 8-bit image. Intersections with a score lower
than minScore will be filtered out. If the distance between two intersections
is less than minPixelDistance, the one with the lower score will be filtered out
 **/
std::vector<std::pair<int, int>> detectGridPosition(const uint8_t* image,
std::size_t height, std::size_t width, std::ptrdiff_t stride, double minScore,
int minPixelDistance, std::size_t templateSize);

/**
Finds the point in a gridPosition vector which is closest to (x,y)
**/
std::pair<int, int> findClosest(const std::vector<std::pair<int, int>>&
gridPosition, double x, double y);

/**
Given a list of pixel points and the center (centerX, cetnerY), subtract all
points with the center
 **/
std::vector<std::pair<int, int>> subtractGridCenter(const
std::vector<std::pair<int, int>>& gridPosition, double centerX, double centerY);

/**
Given a list of grid positions, return their corresponding world coordinates
assuming neighbor grid points are "gridLength" units apart from each other
 **/
std::vector<std::pair<double, double>> assignGridCoordinate(const
std::vector<std::pair<int, int>>& gridPosition, double gridLength,
bool invertedX, bool invertedY);

/**
Given pixel coordinates and their corresponding world coordinate, determine the
optimal parameters for the mapping equation
 **/
std::vector<double> optimizeDistortionCoefficient(const std::vector<std::pair<int, int>>&
gridPixelPosition, const std::vector<std::pair<double, double>>& gridWorldPosition);

/**
Removes abnormal data points before performing optimizeDistortionCoefficient
**/
std::vector<double> optimizeDistortionCoefficient(const std::vector<std::pair<int, int>>&
gridPixelPosition, const std::vector<std::pair<double, double>>& gridWorldPosition, double threshold, int iteration);

/**
Finds the average value of vector y
 **/
double constantRegression(const std::vector<double>& y);

/**
Regression for y = a*x
 **/
double linearRegression(const std::vector<double>& x, const std::vector<double>& y);

/**
Regression for a = ka1 + ka2 * (sx^2 + sy^2)
 **/
std::pair<double, double> ka(const std::vector<double>& scannerPositionX,
const std::vector<double>& scannerPositionY, const std::vector<double>& a);

/**
Regression for b = kb1 + kb2 * sx * sy
 **/
std::pair<double, double> kb(const std::vector<double>& scannerPositionX,
const std::vector<double>& scannerPositionY, const std::vector<double>& b);

/**
Given the scanner position and calibration parameters, estimate the distortion
coefficients
 **/
std::vector<double> scannerPositionToDistortionCoefficient(double sx, double sy,
const std::vector<double>& k);

/**
Given the position of object relative to laser focus point in world coordinate,
calculate the pixel position of the object relative to TCP
 **/
std::pair<double, double> worldToPixel(double u, double v, const std::vector<double>& distortionCoefficient);

/**
Given the pixel position of object relative to TCP, calculate the object position
relative to laser focus point in world coordinate
 **/
std::pair<double, double> pixelToWorld(double x, double y, const std::vector<double>& distortionCoefficient);

std::pair<double, double> imageShift(const uint8_t *imageA, const uint8_t *imageB, int height, int width, std::ptrdiff_t strideA, std::ptrdiff_t strideB);

int horizontalPeriod(uint8_t *image, int height, int width, std::ptrdiff_t stride);

std::vector<Corner> findChessBoardCorners(uint8_t *image, int height, int width, std::ptrdiff_t stride, int period);

std::vector<std::vector<std::pair<double, double>>> findAnalyseSquares(uint8_t *image, int height, int width, std::ptrdiff_t stride, int period);

std::vector<std::vector<std::pair<double, double>>> findAnalyseSquaresLoG(uint8_t *image, int height, int width, std::ptrdiff_t stride, int period);

std::vector<std::pair<double, double>> postprocessSquareResult(const std::vector<std::vector<std::vector<std::pair<double, double>>>>& data, const std::vector<std::pair<double, double>>& scannerPosition, double TCP0x, double TCP0y);

std::vector<double> polyFit2D(const std::vector<double>& x, const std::vector<double>& y, const std::vector<double>& z, int order, double threshold, int iteration);

std::vector<double> generalizedRansac(const std::vector<double>& A, const std::vector<double>& B, double threshold, int iteration);

PostprocessResult postprocess(const std::vector<std::pair<double, double>>& scannerPosition, const std::vector<std::vector<Corner>>& scanfieldResult, double period, double TCP0x, double TCP0y, int sgnX = 1, int sgnY = 1);
