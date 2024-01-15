/*
 * 	@file
 * 	@copyright	Precitec Vision GmbH & Co. KG
 * 	@author		Andreas Beschorner (BA)
 * 	@date		2012 - 2013
 */


#include "math/2D/avgAndRegression.h"
#include <config-weldmaster.h>

#include <limits>
#if HAVE_SSE4
#include <smmintrin.h>
#endif
#include <iostream>
#include <cstdio>
#include <cassert>

namespace precitec {
namespace math {

bool arithmeticAvg (double &p_rAvg, const unsigned int p_oNumPoints, const double* p_pValues, double &p_rSum)
{
    if (p_oNumPoints == 0)
    {
        p_rAvg = std::numeric_limits<double>::quiet_NaN();
        return false;
    }

#if HAVE_SSE4
    unsigned int i;
    align16(double oTmp[2]);
    __m128d res = _mm_setzero_pd();
    __m128d r1, r2;

    const unsigned int oLooplength = (p_oNumPoints / 4);
    const unsigned int oLoopCnt = (oLooplength*4);

    for (i=0; i < oLoopCnt; i += 4)
    {
        r1 = _mm_loadu_pd(p_pValues+i);
        r2 = _mm_loadu_pd(p_pValues+i+2);
        res = _mm_add_pd(res, r1);
        res = _mm_add_pd(res, r2);
    }
    _mm_store_pd(oTmp, res);
    p_rAvg = oTmp[0] + oTmp[1];

    while (i < p_oNumPoints )
    {
        p_rAvg += p_pValues[i];
        ++i;
    }

    p_rSum = p_rAvg;
    p_rAvg /= p_oNumPoints;
    return true;
#else
    p_rAvg = std::numeric_limits<double>::quiet_NaN();
    return false;
#endif
}

bool evaluateRegression(double &p_rSlope, double &p_rIntercept, double &p_rRegCoeff,
                               double p_oAvgX, double p_oAvgY, double* p_pX, double* p_pY, double *p_pXY)
{
    double Sx=0.0; double Sy=0.0; double Sxy=0.0;

    Sx  = p_pX[0] + p_pX[1];
    Sy  = p_pY[0] + p_pY[1];
    Sxy = p_pXY[0] + p_pXY[1];

    // numerical checks

    if (Sx < eps) // quasi vertical line: no funktion in y, our equation is x = const. = p_oAvgX
    {
        p_rSlope = 0.0;
        p_rIntercept = p_oAvgX; // y Intercept!!!
        return false;
    }

    if (Sy < eps)
    {
        if (Sx >= eps)  // horizontal line, y = const = p_oAvgY
        {
            p_rSlope = 0.0;
            p_rRegCoeff = 1.0;
            p_rIntercept = p_oAvgY;
            return true;
        }
    }

    p_rSlope = Sxy / Sx;
    p_rIntercept = p_oAvgY - (p_rSlope*p_oAvgX);
    p_rRegCoeff = Sxy/std::sqrt(Sx*Sy);

    return true;
}

/**
 * \section labelRegression Mean square error linear regression: Main
 * Given a set of 2d-points (x, y), this function computes\n
 *
 * \f$ oX = \sum_i(x_i-\overline{x})^2,\quad
 *     oY = \sum_i(y_i-\overline{y})^2, \quad
 *     oXY = \sum_i( (x_i-\overline{x})(y_i-\overline{y}) ) \f$\n\n
 *
 * where \f$ 1 \le i \le \#\f$points and \f$ \overline{x} \f$, \f$ \overline{y} \f$ are arithmetic means of \f$ x \f$ and \f$ y \f$ coordinates respectively.\n
 * Both, a windows and a g++ version are assembler-optimized using intrinsincs. For the intrinsic variant to be really optimized w.r.t. sse registers,
 * pipelining etc. one MUST compile with O2 optimization!
 */

#if HAVE_SSE4
template<typename T>
void linearRegression2DAsm(double *oX, double *oY, double *oXY,
                           unsigned int p_oNumPoints, T *p_pXcoords, T *p_pYcoords, double p_oAvgX, double p_oAvgY)
{
    // we must not modify ESI, EDI! Pipe optimized
	register __m128d resSx __asm__("xmm3") = _mm_setzero_pd();
	register __m128d resSy asm("xmm4")  = _mm_setzero_pd();
	register __m128d resSxy asm("xmm5") = _mm_setzero_pd();
	register const __m128d avgX asm("xmm6") = _mm_set_pd(p_oAvgX, p_oAvgX);
	register const __m128d avgY asm("xmm7") = _mm_set_pd(p_oAvgY, p_oAvgY);

    //asm("movupd xmm6, xmmword ptr oAvgX" : : "g"(oAvgX));

    // the loop is pipe optimized!
    unsigned int i;
    __m128d rX, rY, rXY;
    for (i=0; i < (p_oNumPoints-1); i += 2)
    {
        rX  = _mm_loadu_pd(p_pXcoords+i);  // x[i+1], x[i]
        rY  = _mm_loadu_pd(p_pYcoords+i);  // y[i+1], y[i]

        rX  = _mm_sub_pd(rX, avgX);         // (x_i - AvgX)
        rY  = _mm_sub_pd(rY, avgY);
        rXY = _mm_mul_pd(rX, rY);           // (x_i - AvgX)(y_i - AvgY)*

        rX  = _mm_mul_pd(rX, rX);           // (x_i - AvgX)^2
        rY  = _mm_mul_pd(rY, rY);

        resSxy = _mm_add_pd(resSxy, rXY);   // Sxy = sum_i[(x_i - AvgX)(y_i - AvgY)]
        resSx  = _mm_add_pd(resSx, rX);     // Sx  = sum_i[(x_i - AvgX)^2]
        resSy  = _mm_add_pd(resSy, rY);
    }

    // optimized for pipes!
    if (i < p_oNumPoints)
    {
        rX = _mm_loadh_pd(avgX, p_pXcoords+i);
        rY = _mm_loadh_pd(avgY, p_pYcoords+i);

        rX  = _mm_sub_pd(rX, avgX);
        rY  = _mm_sub_pd(rY, avgY);

        rXY = _mm_mul_pd(rX, rY);
        rX  = _mm_mul_pd(rX, rX);
        rY  = _mm_mul_pd(rY, rY);

        resSx  = _mm_add_pd(resSx, rX);
        resSy  = _mm_add_pd(resSy, rY);
        resSxy = _mm_add_pd(resSxy, rXY);
    }

    _mm_storeu_pd(oX, resSx);
    _mm_storeu_pd(oY, resSy);
    _mm_storeu_pd(oXY, resSxy);

}
#endif

bool linearRegression2D(double &p_rSlope, double &p_rIntercept, double &p_rRegCoeff,
                        const unsigned int p_oNumPoints, double *p_pXcoords, double *p_pYcoords)
{
    double oX[2], oY[2], oXY[2];
    p_rSlope = 0.0; p_rIntercept = 0.0;
    double oAvgX = 0.0; double oAvgY = 0.0;
    double oAvgSum = 0.0; // non needed here, usefull for cumulative, successive computations
    p_rRegCoeff=0.0;

    if (p_oNumPoints < 2)
    {
        return false;
    }

    arithmeticAvg(oAvgX, p_oNumPoints, p_pXcoords, oAvgSum);
    arithmeticAvg(oAvgY, p_oNumPoints, p_pYcoords, oAvgSum);
#if HAVE_SSE4
    linearRegression2DAsm(oX, oY, oXY, p_oNumPoints, p_pXcoords, p_pYcoords, oAvgX, oAvgY);
#else
        return false;
#endif

    return evaluateRegression(p_rSlope, p_rIntercept, p_rRegCoeff, oAvgX, oAvgY, oX, oY, oXY);
}

bool linearRegression(double &p_rSlope, double &p_rIntercept,
        const double p_oX1, const double p_oY1, const double p_oX2, const double p_oY2)
{
    if (std::abs(p_oX2 - p_oX1) < math::eps)   // nearly same x coords -> function is const. x
    {
        p_rSlope = 0.0;
        p_rIntercept = p_oX1;
        return false;
    }
    p_rSlope = (p_oY2 -p_oY1) / (p_oX2 - p_oX1);
    p_rIntercept = p_oY1 - (p_rSlope * p_oX1);
    return true;
}


//compare with doLineFit in computeAngle.h (input is not necessarily laser line, return line coefficients instead of y1,y2)
bool orthogonalLinearRegression2D(double &oA, double &oB, double &oC, const unsigned int p_oNumPoints, const double * const p_pXcoords, const double * const p_pYcoords)
{
    oA = 0;
    oB = 0;
    oC = 0;

    if ( p_oNumPoints < 2 )
    {
        return false;
    }

    double oXMean = 0.0f;
    double oYMean = 0.0f;
    for (unsigned int oIndex = 0; oIndex < p_oNumPoints; ++oIndex )   
    {
        
        oXMean += p_pXcoords[oIndex];
        oYMean += p_pYcoords[oIndex];
    }

    oXMean = oXMean / static_cast<double>(p_oNumPoints);
    oYMean = oYMean / static_cast<double>(p_oNumPoints);

    double oSXX = 0.0f;
    double oSYY = 0.0f;
    double oSXY = 0.0f;
    for ( unsigned int oIndex = 0; oIndex < p_oNumPoints; ++oIndex )  
    {
        double oXTemp = p_pXcoords[oIndex] - oXMean;
        double oYTemp = p_pYcoords[oIndex] - oYMean;

        oSXX = oSXX + oXTemp * oXTemp;
        oSYY = oSYY + oYTemp * oYTemp;
        oSXY = oSXY + oXTemp * oYTemp;

}

    // Eine exakt senkrechte Gerade zeichnet sich aus durch oSXX = 0.
    // Allerdings sollte eine exakt senkrechte Gerade hier gar nicht vorkommen.
    // Die Eingangs-X-Koordinaten sind die Koordinaten zweier verschiedener Extrema
    // der Kruemmung. D.h. es liegen mindestens zwei Punkte vor, mit verschiedenen X-Koordinaten. 
    // Tatsaechlich ist es sogar realistisch, dass wir eine exakt waagerechte Gerade vorliegen haben.
    // In dem Fall ist oSYY = 0.
    // Teste auf degenerierte Eingangsdaten und teste dabei sowohl X als auch Y:
    bool oDegenerate = (oSXX < 0.1f) && (oSYY < 0.1f);     // UND-verknuepft, nicht ODER-verknuepft ...
    if ( oDegenerate )
    {
        // Diese Daten ergeben ganz einfach kein Liniensegment
        return false;
    }

    double oQ = 0.5f * (oSYY - oSXX);
    double oW = sqrt(oQ * oQ + oSXY * oSXY);
    double oQMinusW = oQ - oW;
    double oDenominator = sqrt(oQMinusW * oQMinusW + oSXY * oSXY);



    if ( oDenominator == 0.0 )
    {
        // Fuer den Fall, dass die Eingabepunkte aus 2 Punkten bestehen, welche exakt auf einer waagerechten oder senkrechten
        // Linie liegen, kann eine Loesung zwar bestimmt werden, aber dieser Loesungsweg geht dann nicht, weil der Denominator
        // zu null wird, und dann wird durch null geteilt und das ist bekanntlich nicht gut.

        // Falls Denominator gleich null und oSXX > 0, dann liegt eine waagerechte Gerade vor.
        // Der Normalenvektor zur Gerade ist also aufwaerts gerichtet.
        // Das Vorzeichen des Normalenvektors wird in der Folge noch korrigiert:
        if ( oSXX > 0.1 )   // Wert kann nicht negativ sein
        {
            oA = 0.0;
            oB = 1.0;
        }

        if ( oSYY > 0.1 )   // Wert kann nicht negativ sein
        {
            oA = 1.0;
            oB = 0.0;
        }
    }
    else
    {
        oA = oSXY / oDenominator;
        oB = oQMinusW / oDenominator;
    }

    // fA ist die X-Komponente des Normalenvektors, fB ist die Y-Komponente des Normalenvektors.
    // Es wird festgelegt, dass der Abstand der Gerade zum Ursprung in Richtung des Normalenvektors
    // stets positiv sein soll. D.h. der Normalenvektor ist entsprechend zu orientieren.
    double oDistanceTest = oXMean * oA + oYMean * oB;
    if ( oDistanceTest < 0.0f )
    {
        oA = -oA;
        oB = -oB;
    }

    oC = -oA* oXMean - oB * oYMean;
    return true;
}



} //namespaces
}
