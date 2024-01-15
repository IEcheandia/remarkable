/**
 *  @file
 *  @copyright	Precitec Vision GmbH & Co. KG
 *  @author		LB
 *  @date		2018
 *  @brief		Utility for handling line equations, computing distances, intersections, projections
 */

#ifndef LINEEQUATION_H_
#define LINEEQUATION_H_

#include <cmath>
#include <vector>
#include <array>
#include <limits>
#include <ostream>
#include "math/mathCommon.h"

#include "Analyzer_Interface.h"

namespace precitec {
namespace math {

class ANALYZER_INTERFACE_API LineEquation
{
    friend ANALYZER_INTERFACE_API std::ostream& operator<<(std::ostream&, const LineEquation&);

public:

    //default CTOR, create invalid line
    LineEquation();
    //constructor with implicit line equation ax + by + c = 0
    LineEquation(double A, double B, double C); 
    //constructor with explicit line equation y = mx + q
    LineEquation(double M, double Q);
    //constructor from point coordinates and angle (see output of lineEstimator filter, but angle has other direction)
    LineEquation(double x, double y, double beta, angleUnit p_angleUnit, bool angleToHorizontalAxis = true);
    //constructor with points to fit
    LineEquation(const std::vector<double> & p_pXcoords, const std::vector<double> & p_pYcoords, bool pHorizontal = true); //fit points (tls)
    
    bool isValid() const;
    void getCoefficients(double &a, double &b, double &c, bool normalized=false) const;
    std::array<double,3> getCoefficients(bool normalized) const;
    void getVector(double & r_u, double & r_v, bool p_PointingRight= true) const; ///< Normalized vector, parallel to the line
    void get2Points(double & x1, double & y1, double & x2, double & y2, const double increment = 1.0) const; //get 2 points lying on the line (the first one is always the intersection to the axis)
    std::vector<std::array<double,2>> intersectRectangle(double p_xMin, double p_xMax, double p_yMin, double p_yMax) const;

    void getPointOnLine(double & xEnd, double & yEnd, const double & xStart, const double & yStart, const double & signedLength);
    
    //apply a translation to the line
    void applyTranslation(double dx, double dy);

    //distance between point and line
    double distance(double x, double y) const;
    //project point (x0,y0) on line
    void project(double & xp, double & yp, const double x0, const double y0 ) const;
    //find intersection point (x,y) to line pLine. returns true if intersection found, false
    //if invalid lines, parallels or coincident
    bool intersect(double & x, double &y, const LineEquation &pLine) const;
    //compute angle to pOtherLine.
    double computeAngleToLine(const LineEquation & pOtherLine) const;
    //getY corresponding to X on the line
    double getY(const double & X) const;
    //getX corresponding to Y on the line
    double getX(const double & Y) const;
    //get inclination angles to axis, expressed between ( -90, 90)
    double getInclinationDegrees() const;

    static double computeRadiansBetweenUnitVectors(const double & p_u0, const double & p_v0, const double & p_u1, const double & p_v1);
    static LineEquation getParallelLinePassingThroughPoint(const LineEquation & line, double x, double y);
    static LineEquation getPerpendicularLinePassingThroughPoint(const LineEquation & line, double x, double y);

private:
    void completeInitialization();
    double m_a;
    double m_b;
    double m_c;
    double m_mag2;
    double m_mag;
    double m_u;
    double m_v;

};
ANALYZER_INTERFACE_API std::ostream& operator<<(std::ostream&, const LineEquation&);



} // namespaces
}

#endif 

