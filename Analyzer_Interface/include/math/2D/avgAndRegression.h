/**
 * 	@file
 * 	@copyright	Precitec Vision GmbH & Co. KG
 * 	@author		  Andreas Beschorner (AB)
 * 	@date		    2012
 *  @brief      Library implementing mathematic functions and methods.
 */

#ifndef MATHGEOMETRY_H_
#define MATHGEOMETRY_H_

#include <cmath>
#include <vector>
#include <limits>

#include "math/asmDefs.h"
#include "math/mathCommon.h"
#include "Analyzer_Interface.h"

/// @brief: namespace precitec
namespace precitec {
/// @brief: namespace precitec::math
namespace math {

ANALYZER_INTERFACE_API bool evaluateRegression(double &p_rSlope, double &p_rIntercept, double &p_rRegCoeff,
		double p_oAvgX, double p_oAvgY, double* p_pX, double* p_pY, double *p_pXY);

/**
 * \section labelAverage Arithmetic average of a 1d-sequence
 * Computes arithmetic average of a sequence of values. Optimized by using SSE-intrinsics and a simple loop unrolling.
 */
ANALYZER_INTERFACE_API bool arithmeticAvg (double &p_rAvg, const unsigned int p_oNumPoints, const double* p_pValues, double &p_rSum);

/**
 * \section labelRegressionCaller Mean square error linear regression: Callee
 * Caller function for SSE-optimized linear regression.
 * Computes slope \f$ s \f$, intercept \f$ c \f$ and the regression \f$ r \f$ coefficient:
 * \f$ s = \frac{oXY}{oX},\quad
 *     c = \overline{y}-(s\cdot\overline{x}),\quad
 *     r = \frac{oXY}{\sqrt{oX\cdot oXY}} \f$,\n
 * where \f$ oX, oY\f$ and \f$ oXY \f$ are computed in
 * \link precitec::math::linearRegression2DAsm Main square error linear regression: Main \endlink
 */
ANALYZER_INTERFACE_API bool linearRegression2D(double &p_rSlope, double &p_rIntercept, double &p_rRegCoeff,
	const unsigned int p_oNumPoints, double *p_pXcoords, double *p_pYcoords);

ANALYZER_INTERFACE_API bool linearRegression(double &p_rSlope, double &p_rIntercept,
		const double p_oX1, const double p_oY1, const double p_oX2, const double p_oY2);

// fit points to Ax + By + C = 0 
ANALYZER_INTERFACE_API bool orthogonalLinearRegression2D(double &p_rA, double &p_rB, double &p_rC,
		const unsigned int p_oNumPoints, const double * const p_pXcoords, const double * const p_pYcoords);

// -------------------------------------------------------


/**
 * \section labelAverage Arithmetic average of a 1d-sequence
 * Computes arithmetic average of a sequence of values. Optimized by using SSE-intrinsics and a simple loop unrolling.
 */
template<typename T>
void arithmeticAvgVec (double &p_rAvg, const std::vector<T> &p_rValues, double &p_rSum, int p_oStopAfter=0)
{
    if (p_rValues.size() == 0)
    {
        p_rAvg = std::numeric_limits<double>::quiet_NaN();
        return;
    }

    T *oValues = new T[p_rValues.size()];
    for (unsigned int i=0; i < p_rValues.size(); ++i)
    {
        oValues[i] = p_rValues[i];
    }

    if (p_oStopAfter < 1)
    {
        p_oStopAfter = p_rValues.size();
    }
    arithmeticAvg(p_rAvg, p_oStopAfter, oValues, p_rSum);
    delete [] oValues;
}



// ---------------------- Line segment passing throuhg two given points ----------------------------


template <typename T>
inline bool compute2dLine(double &p_rSlope, double &p_rIntercept,
                          const T p_oXLeft, const T p_oYLeft, const T p_oXRight, const T p_oYRight)
{
    if ( std::abs(p_oXRight - p_oXLeft) < math::eps )
    {
        return false;
    }
    p_rSlope = (p_oYRight - p_oYLeft)/(p_oXRight - p_oXLeft);
    p_rIntercept = ((p_oYLeft - p_rSlope*p_oXLeft) + (p_oYRight - p_rSlope*p_oXRight))*0.5;
    return true;
}


// --------------------------------- Linear Regression ---------------------------------------------






/**
 * \section labelRegressionUnoptimized Mean square error linear regression
 * Unoptimized version, \see \b{Mean square error linear regression} \b for details.
 */
template<typename T>
bool linearRegression2DNoSSE(double &p_rSlope, double &p_rIntercept, double &p_rRegCoeff,
                             const unsigned int p_oNumPoints, const T *p_pXcoords, const T *p_pYcoords)
{
    p_rRegCoeff=0.0;
    int i;

    // we calculate 2 times a 1D average instead of 1 time a 2D avg. due to vectorization and prevention of cash line misses when using 2 arrays
    double oAvgX = 0.0; double oAvgY = 0.0;
    p_rSlope = 0.0; p_rIntercept = 0.0;
    arithmeticAvg(oAvgX, p_oNumPoints, p_pXcoords);
    arithmeticAvg(oAvgY, p_oNumPoints, p_pYcoords);

    if (p_oNumPoints < 2)
    {
        return false;
    }

    // we do loop unrolling, simulating the SIMD implementation, sorta
    int oLoopsize = (int)p_oNumPoints / 4;
    unsigned int oIdx = 0;
    double Sx=0.0; double Sy=0.0; double Sxy=0.0;

    // wee need to access both arrays anyway, so no separate loops here
    for (i=0; i < oLoopsize; ++i)
    {
        oIdx = i << 2;
        Sx  += ( p_pXcoords[oIdx] - oAvgX )*( p_pXcoords[oIdx] - oAvgX ) + ( p_pXcoords[oIdx+1] - oAvgX )*( p_pXcoords[oIdx+1] - oAvgX ) +
                ( p_pXcoords[oIdx+2] - oAvgX )*( p_pXcoords[oIdx+2] - oAvgX ) + ( p_pXcoords[oIdx+3] - oAvgX )*( p_pXcoords[oIdx+3] - oAvgX );
        Sy  += ( p_pYcoords[oIdx] - oAvgY )*( p_pYcoords[oIdx] - oAvgY ) + ( p_pYcoords[oIdx+1] - oAvgY )*( p_pYcoords[oIdx+1] - oAvgY ) +
                ( p_pYcoords[oIdx+2] - oAvgY )*( p_pYcoords[oIdx+2] - oAvgY ) + ( p_pYcoords[oIdx+3] - oAvgY )*( p_pYcoords[oIdx+3] - oAvgY );
        Sxy += ( p_pXcoords[oIdx] - oAvgX )*( p_pYcoords[oIdx] - oAvgY ) + ( p_pXcoords[oIdx+1] - oAvgX )*( p_pYcoords[oIdx+1] - oAvgY ) +
                ( p_pXcoords[oIdx+2] - oAvgX )*( p_pYcoords[oIdx+2] - oAvgY ) + ( p_pXcoords[oIdx+3] - oAvgX )*( p_pYcoords[oIdx+3] - oAvgY );
    }

    oIdx = oLoopsize << 2;
    while (oIdx < p_oNumPoints)
    {
        Sx  += (p_pXcoords[oIdx] - oAvgX)*(p_pXcoords[oIdx] - oAvgX);
        Sy  += (p_pYcoords[oIdx] - oAvgY)*(p_pYcoords[oIdx] - oAvgY);
        Sxy += (p_pXcoords[oIdx] - oAvgX)*(p_pYcoords[oIdx] - oAvgY);
        ++oIdx;
    }


    // todo: evaluateRegression
    //return evaluateRegression(p_rSlope, p_rIntercept, p_rRegCoeff, oAvgX, oAvgY, oX, oY, oXY);
}

template<typename T>
double arithmeticAvgVector(std::vector< T > &p_rVec)
{
    if (p_rVec.size() < 1)
    {
        return 0.0;
    }

    double oRet=0.0;
    for (unsigned int i=0; i < p_rVec.size(); ++i)
    {
        oRet += p_rVec[i];
    }
    oRet /= p_rVec.size();
    return oRet;
}


// ---------------------------  //

/// linear regression using points and values. Returns the length
template<typename T>
bool pointRegression(double &p_rLength, double &m_oDirX, double &m_oDirY,
                     const std::vector<T> &p_rX, const std::vector<T> &p_rY, const std::vector<T> &p_rVal, const unsigned int p_oLen=0)
{
    if (p_oLen == 0)
    {
        if ( (p_rX.size() != p_rY.size()) || (p_rX.size() != p_rVal.size()) )
        {
            return false;
        }
        p_oLen = p_rX.size();
    }

    T oVal[6]={0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
    for (unsigned int i=0; i < p_oLen; ++i)
    {
        oVal[0] += (p_rVal[i] * p_rVal[i]);     /// sum val[i]^2
        oVal[1] += p_rVal[i];                   /// sum val[i]
        oVal[2] += (p_rX[i] * p_rVal[i]);       /// sum x[i]val[i]
        oVal[3] += p_rX[i];                     /// sum x[i]
        oVal[4] += (p_rY[i] * p_rVal[i]);       /// sum y[i]val[i]
        oVal[5] += p_rY[i];                     /// sum y[i]
    }
    double denom = (oVal[0] * p_oLen) - ( oVal[1] * oVal[1]); /// val[i]+#points - (sum val[i])^2

    if (std::abs(denom) < eps)
    {
        return false;
    }
    double nom1 = ((oVal[1] * oVal[3]) - (oVal[2]*p_oLen))/denom; double nom2 = ((oVal[1] * oVal[5]) - (oVal[4]*p_oLen))/denom;

    p_rLength = std::sqrt((nom1*nom1) + (nom2*nom2));// std::cout << " // Len " << p_rLength  << std::endl;

    if ((nom1*m_oDirX) < 0)
    {
        m_oDirX = -m_oDirX;
    }

    if ((nom2*m_oDirY) < 0)
    {
        m_oDirY = -m_oDirY;
    }

    return true;
}

} // namespaces
}

#endif /* MATHGEOMETRY_H_ */

