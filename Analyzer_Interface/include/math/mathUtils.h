/*
 * mathUtils.h
 *
 *  Created on: Sep 17, 2013
 *      Author: abeschorner
 */

#include<vector>
#include<tuple>
#include<array>
#include <iostream>
#include <sstream>
#include <string>

#ifndef MATHUTILS_H_
#define MATHUTILS_H_

namespace precitec {
namespace math {

typedef std::vector<double> tVecDouble;

// string to T. Example: if ( from_String(target, string, std::dec) ) {}
template <class T>
bool from_string(T& t, const std::string& s, std::ios_base& (*f)(std::ios_base&)) {
  std::istringstream iss(s);
  return !(iss >> f >> t).fail();
}

bool isPowerOfTwo(unsigned long long x);
bool lengthRatio(double &p_rRes, const double p_oSlope, const double p_oLength, const double p_oLengthToCompareTo, tVecDouble &p_rX);

} /* namespace math */
} /* namespace precitec */
#endif /* MATHUTILS_H_ */
