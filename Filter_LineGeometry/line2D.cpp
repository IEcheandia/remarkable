/**
 * 	@file
 * 	@copyright	Precitec Vision GmbH & Co. KG
 * 	@author		OS
 * 	@date		2015
 * 	@brief 		This class offers the functionality for calculation needs on lines and points.
 */

#include "line2D.h"
#include <stdlib.h>
#include <math.h>
#include <cmath>

#define SMALL_EPSILON 0.000001
#define BIG_SLOPE 1000000.0

/////////////////////////
// Class Line 2D
/////////////////////////

Line2D::Line2D(double slope, double yIntercept)
{
	m_slope = slope;
	m_yIntercept = yIntercept;;
	m_isVertical = false;
	m_isValid = true;
}

Line2D::Line2D(double x, double y, double slope)
{
	m_slope = slope;
	m_yIntercept = y - slope * x;
	m_isVertical = false;
	m_isValid = true;
}

Line2D::Line2D(double x1, double y1, double x2, double y2)
{
	if ( (std::abs(x2-x1) < SMALL_EPSILON) && (std::abs(y2-y1) < SMALL_EPSILON) )
	{ // Punkte identisch => keine Gerade moeglich
		m_slope = 0;
		m_yIntercept = 0;
		m_isValid = false;
		m_isVertical = true;

		return;
	}

	if ( std::abs(x2-x1) < SMALL_EPSILON)
	{ // Punkte verschieden, aber Gerade senkrecht
		m_slope = BIG_SLOPE;
		m_yIntercept = x1;
		m_isValid = true;
		m_isVertical = true;
		return;
	}

	// Punkte hier ok

	m_slope = (y2-y1) / (x2-x1);
	m_yIntercept = y1 - m_slope * x1;
	m_isValid = true;
	m_isVertical = false;
	m_oInterceptX = 0.0;
	m_oInterceptY = 0.0;
}

double Line2D::getSlope()
{
	return m_slope;
}

double Line2D::getYIntercept()
{
	return m_yIntercept;
}

double Line2D::getIsValid()
{
	return m_isValid;
}
double Line2D::getIsVertical()
{
	return m_isVertical;
}

double Line2D::getY(double x)
{
	if (!m_isValid || m_isVertical) return 0;
	return x * m_slope + m_yIntercept;
}

double Line2D::getOrthoSlope()
{
	if (!m_isValid) return 0;
	if (m_isVertical) return 0;
	if (m_slope == 0) return BIG_SLOPE;
	return (-1.0 / m_slope);
}

double Line2D::calcDistance(double x, double y, double & outXonLine)
{
	Line2D newLine(x, y, getOrthoSlope());

	double intersectionX = getIntersectionX(newLine);
	outXonLine = intersectionX;
	double intersectionY = getY(intersectionX);

	m_oInterceptX = intersectionX;
	m_oInterceptY = intersectionY;

	double diffX = intersectionX - x;
	double diffY = intersectionY - y;

	return sqrt(diffX * diffX + diffY * diffY);
}

double Line2D::calcDistance(double x, double y)
{
	double outX;
	return calcDistance(x, y, outX);
}

double Line2D::getIntersectionX(Line2D otherLine)
{
	if (!m_isValid) return 0;
	if (!otherLine.getIsValid()) return 0;

	if (m_isVertical) return m_yIntercept;
	if (otherLine.getIsVertical()) return otherLine.getYIntercept();

	if (std::abs(m_slope - otherLine.getSlope()) < SMALL_EPSILON) return 0; // Steigungen zu aehnlich => parallel => nix Schnittpunkt

	return (otherLine.getYIntercept() - m_yIntercept) / (m_slope - otherLine.getSlope());
}

