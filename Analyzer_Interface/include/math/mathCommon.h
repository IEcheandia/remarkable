/**
 * 	@file
 * 	@copyright	Precitec Vision GmbH & Co. KG
 * 	@author		  Andreas Beschorner (BA)
 * 	@date		    03/2013
 *  @brief      Simple and/or specific math functions.
 */

#ifndef MATHCOMMON_H
#define MATHCOMMON_H

#include <cmath>
#include <cassert>
#include <utility> //swap
/// @brief: namespace precitec
namespace precitec {
/// @brief: namespace precitec::math
namespace math {

/// Constant value for numerical accuracy tests.
static const double eps = 1e-7;
static const double pi = std::acos(double(-1.0)); 

static const double RIGHT_ANGLE_DEG = 90.0;
static const double RIGHT_ANGLE_RAD = math::pi/2;
static const double STRAIGHT_ANGLE_DEG = 180.0;
static const double STRAIGHT_ANGLE_RAD = math::pi;
static const double TURN_ANGLE_DEG = 360.0;
static const double TURN_ANGLE_RAD = 2 * math::pi;

enum class angleUnit
{
    eDegrees,
    eRadians
};


inline double degreesToRadians(const double & p_Angle)
{
    return p_Angle * pi / 180.0;
}

inline double radiansToDegrees(const double & p_Angle)
{
    return p_Angle * 180.0 / pi;
}

    

/// Signature of a signed type T. Given p_oValue=\f$p\f$ returns \f$\begin{cases}1 & p>0\\0 & p =0\\-1 & p<0\end{cases}.\f$
template<typename T>
int sgn(const T p_oValue)
{
	if (p_oValue > 0)
	{
		return 1;
	} else if (p_oValue < 0)
	{
		return -1;
	} else
	{
		return 0;
	}
}

/** Tests whether a comparable value falls into an interval of the same type. Given a value \f$p\f$=p_oVal and left/right boundaris \f$l\f$=p_oLeft,
 * \f$r\f$=p_oRight, the functions returns true for \f$ l \le p \le r\f$, false otherwise.
 */
template<typename T>
bool isInRange(T p_oVal, T p_oLeft, T p_oRight)
{
	if (p_oLeft > p_oRight)
	{
		std::swap(p_oLeft, p_oRight);
	}
	return ( (p_oLeft <= p_oVal) && (p_oRight >= p_oVal) );
}

/** Tests whether a comparable value falls into an interval of type long. Given a value \f$p\f$=p_oVal and long left/right boundaris \f$l\f$=p_oLeft,
 * \f$r\f$=p_oRight, the functions returns true for \f$ l \le p \le r\f$, false otherwise.
 */
template<typename T>
bool isInRange(T p_oVal, long p_oLeft, long p_oRight)
{
	if (p_oLeft > p_oRight)
	{
		long pTmp = p_oLeft;
		p_oLeft = p_oRight;
		p_oRight = pTmp;
	}
	return ( (p_oLeft <= p_oVal) && (p_oRight >= p_oVal) );
}

//Compare two numbers with a tolerance
template<typename T>
bool isClose(T p_val1, T p_val2, T tol = 1e-10)
{
    return std::abs(p_val1 - p_val2) < tol;
}

//convert an angle in the range (-90,90) or (0,180)
inline double constrainAngle(double p_Angle, angleUnit p_angleUnit, bool p_useNegativeValues = false)
{
    const bool isRadians = p_angleUnit == angleUnit::eRadians;

#ifndef NDEBUG
    const double oInputAngleRad = p_angleUnit == angleUnit::eRadians? p_Angle : degreesToRadians(p_Angle);
#endif
    const double & r_TurnAngle = isRadians ? TURN_ANGLE_RAD : TURN_ANGLE_DEG;
    const double & r_StraightAngle = isRadians ? STRAIGHT_ANGLE_RAD : STRAIGHT_ANGLE_DEG;
    const double & r_RightAngle = isRadians ? RIGHT_ANGLE_RAD : RIGHT_ANGLE_DEG;

    //normalize angle between [0,360)
    p_Angle = std::fmod(p_Angle, r_TurnAngle);
    if ( p_Angle < 0 )
    {
        p_Angle += r_TurnAngle;
    }

    //normalize angle between [0, 180)
    p_Angle = std::fmod(p_Angle, r_StraightAngle);
    assert(p_Angle >= 0 && p_Angle <= r_StraightAngle);

    //if necessary,  normalize angle between (-90, 90]
    if ( p_useNegativeValues && (p_Angle > r_RightAngle) )
    {
        p_Angle -= r_StraightAngle;
    }
#ifndef NDEBUG
    if ( p_useNegativeValues )
    {
        assert(p_Angle > -r_RightAngle && p_Angle <= r_RightAngle);
    }

    if ( !isClose(std::abs(oInputAngleRad), RIGHT_ANGLE_RAD, 1e-2) )
    {
        double oTgInputAngle = std::tan(oInputAngleRad);
        double oTgOutputAngle = std::tan(isRadians ? p_Angle : degreesToRadians(p_Angle));
        assert(isClose(oTgInputAngle, oTgOutputAngle));
    }
#endif
    return p_Angle;
}


//assumes p_oMin < p_oMax
template<typename T>
T clip(T p_oVal, const T & p_oMin, const T & p_oMax)
{
	assert(p_oMin <= p_oMax);
	T result = p_oVal < p_oMin ? p_oMin : p_oVal;
	result = result > p_oMax ? p_oMax : result;
	return result;
}

} // namespaces
}

#endif

