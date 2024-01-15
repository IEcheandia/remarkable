#pragma once

#include <vector>

namespace precitec {
namespace filter {

/**
Given a single line, apply a low pass filter to smooth the line.
'fmax' parameter is the maximum frequency allowed to pass through
the low pass filter. A frequency of '1' means a sine wave
that spans the line exactly one time. A frequency of '2' means a
sine wave that spans the line exactly two times and so on.
 **/
std::vector<double> smoothLine(const std::vector<double>& line, int fmax);

/**
Given a single line, apply a low pass filter to smooth the line.
'fmax' parameter is the maximum frequency allowed to pass through
the low pass filter.

The 'iteration' and 'threshold' parameters are used to filter out
outliers in the line. Outliers are filtered iteratively by selecting
a few samples at a time and fit the samples against a sum of
sinusoidal waves.

The 'threshold' parameter is the maximum
difference between a data point of the line and the fitted line, in
order to be considered valid (not an outlier). If either the
'iteration' or 'threshold' parameter is 0, no outlier detection
will be performed.
 **/
std::vector<double> smoothLine(const std::vector<double> &line, int fmax, int iteration, double threshold);

}
}
