#pragma once

#include <utility>
#include <vector>

#include <string>

namespace precitec
{
namespace coordinates
{

/**
 * Given the IDM measurements of the scan field, determine the model parameters
 * for the system. IDM measurement data are stored as x, y, and z vectors of
 * equal sizes. z has N vectors of size equal to x and y, where N is the total
 * number of reference arms to be calibrated.
 *
 * Example data set for 4 reference arms:
 * x: {-5, 0, 5, -5, 0, 5, -5, 0, 5}
 * y: {-5, -5, -5, 0, 0, 0, 5, 5, 5}
 * z: {
 *       {1, 2, 3, 4, 5, 6, 7, 8, 9},
 *       {1, 2, 3, 4, 5, 6, 7, 8, 9},
 *       {1, 2, 3, 4, 5, 6, 7, 8, 9},
 *       {1, 2, 3, 4, 5, 6, 7, 8, 9}
 *    }
 *
 * The algorithm uses RANSAC to filter out invalid data. The threshold and
 * iteration parameters are RANSAC specific parameters.
 *
 * The output of idmModel function is a pair of vectors. The first vector
 * stores the calibrated offset of each reference arm. So if there are 4
 * reference arms, the size of the first vector would be 4. The second vector
 * stores system parameters common for all reference arms, i.e. the curvature
 * and tilt parameters.
**/
std::pair<std::vector<double>, std::vector<double>> idmModel(const std::vector<float>& x, const std::vector<float>& y, const std::vector<std::vector<float>>& z, float threshold, int iteration);

/**
 * Given the model parameters determined by the idmModel function, determine
 * the optimal reference arm to measure the object at a given scanner position.
 * Optimal means the difference between the  measurement range above the focus
 * plane and below the focus plane is minimal.
**/
int fiberSwitchSelect(const std::pair<std::vector<double>, std::vector<double>>& model, double idmHalfRange, double sx, double sy);

/**
 * Saves the raw IDM measurement data in a file for debug.
**/
void saveIdmRawData(const std::vector<float>& x, const std::vector<float>& y, const std::vector<std::vector<float>>& z, const std::string& fileName);

}
}
