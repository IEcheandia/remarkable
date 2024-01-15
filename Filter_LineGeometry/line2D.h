/**
 * 	@file
 * 	@copyright	Precitec Vision GmbH & Co. KG
 * 	@author		OS
 * 	@date		2015
 * 	@brief 		This class offers the functionality for calculation needs on lines and points.
 */

#ifndef LINE2D_H_
#define LINE2D_H_

class Line2D
{
public:
	Line2D(double slope, double yIntercept);
	Line2D(double x, double y, double slope);
	Line2D(double x1, double y1, double x2, double y2);

	double getSlope();
	double getYIntercept();
	double getY(double x);
	double getIsValid();
	double getIsVertical();

	double getOrthoSlope();
	double calcDistance(double x, double y);
	double calcDistance(double x, double y, double & outXonLine);

	double getIntersectionX(Line2D otherLine);

	double m_oInterceptX;
	double m_oInterceptY;

private:
	double m_slope;
	double m_yIntercept;
	bool m_isVertical;
	bool m_isValid;
};

#endif // LINE2D_H_
