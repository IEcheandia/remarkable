#include "math/mathCommon.h"
#include "math/2D/avgAndRegression.h" //for orthogonalLinearRegression2D
#include "math/2D/LineEquation.h"

#include <limits>
#include <iostream>
#include <cassert>



namespace precitec {
namespace math {


LineEquation::LineEquation()
    :LineEquation(0, 0, 0)
    {};

LineEquation::LineEquation(double oA, double oB, double oC) 
    : m_a(oA), m_b(oB), m_c(oC)
    {
        completeInitialization();
    };

LineEquation::LineEquation(double oM, double oQ) 
    : LineEquation(oM, -1, oQ)
    {};

LineEquation::LineEquation(double x, double y, double beta, angleUnit p_angleUnit, bool angleToHorizontalAxis)
{
    double betaRad = (p_angleUnit == angleUnit::eDegrees) ? degreesToRadians(beta) : beta;
    if ( !angleToHorizontalAxis )
    {
        betaRad = math::RIGHT_ANGLE_RAD - betaRad;
    }
    m_a = std::sin(betaRad);
    m_b = -std::cos(betaRad);
    m_c = -m_a*x - m_b *y;
    completeInitialization();

#ifndef NDEBUG
    //check that the computed line passes through x,y and the angle to the x axis is beta
    double betaDeg = (p_angleUnit == angleUnit::eDegrees) ? beta : beta * 180 / math::pi;
    if ( !angleToHorizontalAxis )
    {
        betaDeg = math::RIGHT_ANGLE_DEG - betaDeg;
    }
    if ( !isClose(std::abs(betaDeg), 90.0) )
    {
        assert(isClose(y , getY(x)));
    }
    if ( !isClose(betaDeg, 0.0) && !isClose(betaDeg, 180.0) )
    {
        assert(isClose(x, getX(y)));
    }
    assert(isClose( constrainAngle(betaDeg, angleUnit::eDegrees, true), getInclinationDegrees()));
#endif
    
};


LineEquation::LineEquation(const std::vector<double> & p_pXcoords, const std::vector<double> & p_pYcoords, bool pHorizontal)
{
    unsigned int oNumPoints = p_pXcoords.size();
    if ( oNumPoints < 2 || p_pYcoords.size() != oNumPoints )
    {
        m_a = 0;
        m_b = 0;
        m_c = 0;
        completeInitialization();
        assert(!isValid());
        return;
    }
    bool ok(true);
    if ( pHorizontal )
    {
        ok = orthogonalLinearRegression2D(m_a, m_b, m_c, oNumPoints, p_pXcoords.data(), p_pYcoords.data());
    }
    else
    {
        //swap x,y and m_a,m_b in order to have better results for vertical lines
        ok = orthogonalLinearRegression2D(m_b, m_a, m_c, oNumPoints, p_pYcoords.data(), p_pXcoords.data());
    }
    if (!ok )
    {
        m_a = 0;
        m_b = 0;
        m_c = 0;
        completeInitialization();
        assert(!isValid());
        return;
    }
    completeInitialization();
    assert(isValid());
}

void LineEquation::completeInitialization()
{
    m_mag2 = m_a*m_a + m_b*m_b;
    m_mag = std::sqrt(m_mag2);
    if ( m_mag > 0 )
    {
        //as a convention, we use the vector going "rightwards"
        double oSign = m_b > 0 ? 1.0 : -1.0;
        m_u = oSign * m_b / m_mag;
        m_v = oSign * -m_a / m_mag;
    }
    else
    {
        m_u = 0;
        m_v = 0;
    }
    assert( (m_mag > 0) == isValid());
}


bool LineEquation::isValid() const
{
    bool oValid =  m_a != 0 || m_b != 0;

#ifndef NDEBUG
    //the following assertions ensure that the internal variables are consistent 
    // i.e, completeInitialization() was called in the constructor
    if (oValid)
    {
        assert(m_mag > 0);
        assert(isClose(m_u*m_u + m_v*m_v, 1.0));
        assert(m_u >= 0);
        //check vector representation: 
        double x0 = 1;
        double y0 = 1;
        double t = -0.0001;
        if ( m_b != 0 )
        {
            y0 = getY(x0);
            
        }
        else
        {
            x0 = getX(y0);
        }
        double x1 = x0 + t*m_u;
        double y1 = y0 + t*m_v;
        assert(isClose(m_a*x1 + m_b *y1 + m_c, 0.0));
    }
    else
    {
        assert(m_mag == 0);
        assert(m_u == 0);
        assert(m_v == 0);
    }
#endif
    return oValid;
}

void LineEquation::getCoefficients(double &a, double &b, double &c, bool normalized) const
{
    a = m_a;
    b = m_b;
    c = m_c;
    if (normalized)
    {
        double oDiv = m_b > 0 ? m_mag : -m_mag;
        if (m_b == 0)
        {
            oDiv = m_a > 0 ? m_mag : -m_mag;
        }
        a /= oDiv;
        b /= oDiv;
        c /= oDiv;
    }
}

std::array<double, 3> LineEquation::getCoefficients(bool normalized) const
{
    double a, b, c;
    getCoefficients(a, b, c, normalized);
    return {a, b, c};
}

void LineEquation::getVector(double & r_u, double & r_v, bool p_PointingRight) const
{
    if ( p_PointingRight )
    {
        //by construction, the internal variables point right
        r_u = m_u;
        r_v = m_v;
        assert(r_u >= 0);
    }
    else
    {
        r_u = -m_u;
        r_v = -m_v;
        assert(r_u <= 0);
    }
}

void LineEquation::get2Points(double & x1, double & y1, double & x2, double & y2, const double increment) const
{
    if ( !isValid() )
    {
        x1 = 0;
        y1 = 0;
        x2 = 0;
        y2 = 0;
        return;
    }
    if ( isClose(m_b, 0.0) )
    {
        y1 = 0;
        x1 = getX(y1);
        y2 = y1 + increment;
        x2 = getX(y2);
    }
    else
    {
        x1 = 0;
        y1 = getY(x1);
        x2 = x1 + increment;
        y2 = getY(x2);
    }
    assert(isClose(distance(x1, y1), 0.0));
    assert(isClose(distance(x2, y2), 0.0));
}


std::vector<std::array<double, 2>> LineEquation::intersectRectangle(double p_xMin, double p_xMax, double p_yMin, double p_yMax) const
{
    std::vector<std::array<double, 2>> oResult;

    if ( !isValid() )
    {
        return oResult;
    }
    if ( p_xMin > p_xMax )
    {
        std::swap(p_xMin, p_xMax);
    }
    if ( p_yMin > p_yMax )
    {
        std::swap(p_yMin, p_yMax);
    }

    //there are 0,1,2 or infinite intersections between a line and a rectangle
    if ( math::isClose(m_a, 0.0) )
    {
        //horizontal line
        double y = -m_c / m_b;
        if ( y >= p_yMin && y <= p_yMax )
        {
            return{std::array<double, 2>{{p_xMin, y}},
                std::array<double, 2>{{p_xMax, y}}
        };
        }
        else
        {
            return oResult;
        }
    }
    if ( math::isClose(m_b, 0.0) )
    {
        //vertical line
        double x = -m_c / m_a;
        if ( x >= p_xMin && x <= p_xMax )
        {
            return {std::array<double, 2>{{x, p_yMin}},
                std::array<double, 2>{{x, p_yMax}}
        };
        }
        else
        {
            return oResult;
        }
    }

    
    for ( const double & x : {p_xMin, p_xMax} )
    {
        double y = getY(x);
        if ( !std::isnan(y) && y >= p_yMin && y <= p_yMax )
        {
            oResult.push_back({{x, y}});
        }
    }

    //in most cases, this is enough
    if ( oResult.size() == 2 )
    {
        return oResult;
    }

    for ( const double & y : {p_yMin, p_yMax} )
    {
        double x = getX(y);
        if ( !std::isnan(x) && x >= p_xMin && x <= p_xMax ) 
        {
            oResult.push_back({{x, y}});
        }
    }

    assert(oResult.size() <= 2);
    return oResult;

}

void LineEquation::getPointOnLine(double & xEnd, double & yEnd, const double & xStart, const double & yStart, const double & signedLength)
{
    if ( !isValid() )
    {
        xEnd = 0;
        yEnd = 0;
        return;
    }
    double xp, yp;
    project(xp, yp, xStart, yStart);
    xEnd = xp + signedLength*m_u;
    yEnd = yp + signedLength*m_v;
    assert(!isValid() || isClose(distance(xEnd, yEnd), 0.0));
}
void LineEquation::applyTranslation(double dx, double dy)
{
    if ( !isValid() )
    {
        return;
    }
#ifndef NDEBUG
    double oldInclination = getInclinationDegrees();
    double x0, y0, x1, y1;
    get2Points(x0, y0, x1, y1, 1);
#endif

    m_c = m_c - (m_a * dx + m_b * dy);
    assert(isValid());
    assert(getInclinationDegrees() == oldInclination);
    assert(isClose(distance(x0 + dx, y0 + dy), 0.0));
    assert(isClose(distance(x1 + dx, y1 + dy), 0.0));

}

double LineEquation::distance(double x, double y) const
{
    return isValid()? std::abs(m_a * x + m_b * y + m_c) / m_mag : 0;
}

void LineEquation::project(double & xp, double & yp, const double x0, const double y0) const
{
    xp = (m_b * (m_b*x0 - m_a*y0) - m_a*m_c) / m_mag2;
    yp = (m_a *(-m_b*x0 + m_a*y0) - m_b*m_c) / m_mag2;
    assert(isClose(distance(xp, yp),0.0, 1e-6));
    assert(isClose(distance(x0, y0),
            std::sqrt( (xp - x0) * (xp - x0) + (yp - y0)*(yp - y0))));
}

bool LineEquation::intersect(double & x, double &y, const LineEquation & pLine) const
{
    if ( !isValid() || !pLine.isValid() )
    {
        return false;
    }
    //solve linear system
    // a x + b y + c = 0  //a,b,c : this line
    // d x + e y + f = 0  //d,e,f : intersecting lines
    //return false if lines parallel or coincident 
    const double & o_d = pLine.m_a;
    const double & o_e = pLine.m_b;
    const double & o_f = pLine.m_c;

    double oDeterminant = m_a*o_e - m_b*o_d;
    if ( std::abs(oDeterminant) < 1e-6 )
    {
        return false;
    }
    x = (-o_e*m_c + m_b*o_f) / oDeterminant;
    y = (o_d*m_c - m_a*o_f ) / oDeterminant;

    assert(isClose(distance(x, y), 0.0) && "intersection point doesn't belong to this line");
    assert(isClose(pLine.distance(x, y), 0.0) && "intersection point doesn't belong to intersecting line");
    return true;
}


double LineEquation::computeRadiansBetweenUnitVectors(const double & p_u0, const double & p_v0, const double & p_u1, const double & p_v1)
{
    double oDotProduct = p_u0 * p_u1 + p_v0 * p_v1;
    if ( math::isClose(oDotProduct, 1.0))
    {
        oDotProduct = 1.0;
    }
    if ( math::isClose(oDotProduct, -1.0) )
    {
        oDotProduct = -1.0;
    }
    assert(std::abs(oDotProduct) <= 1 && "input vectors are not normalized or bad floating point representation ");
    double oAngle = std::acos(oDotProduct);

    assert(oAngle >= 0.0 && oAngle < math::STRAIGHT_ANGLE_RAD);
    
    //if dotProduct is positive, the 2 vectors point at the same direction (acute angle)
    assert( (oAngle <= (math::RIGHT_ANGLE_RAD + 1e-10))  == ( oDotProduct >= (0 - 1e-10)));

    //from vector 0  to vector 1, should we go counterclockwise (positive angle) or clockwise?
    if (  p_v1 < p_v0)
    {
        //clockwise
        oAngle = -oAngle;  
    }
    
    return oAngle;
}

double LineEquation::computeAngleToLine(const LineEquation & pOtherLine) const
{
    return computeRadiansBetweenUnitVectors(m_u, m_v, pOtherLine.m_u, pOtherLine.m_v);
}


double LineEquation::getY(const double & X) const
{
    if ( m_b != 0 )
    {
        return -m_a / m_b * X - m_c / m_b;
    }
    if ( X == -m_c / m_a )
    {
        std::cout << "Warning, vertical line \n";
        return 0;
    }
    else
    {
        std::cout << "Warning, vertical line with x = " << -m_c/m_a << " but x = " << X << " requested \n";
        return std::nan("");
    }
}

double LineEquation::getX(const double & Y) const
{
    if ( m_a != 0 )
    {
        return -m_b / m_a * Y - m_c / m_a;
    }
    if ( Y == -m_c / m_b )
    {
        std::cout << "Warning, horizontal line \n";
        return 0;
    }
    else
    {
        std::cout << "Warning, horizontal line with y = " << -m_c / m_b << " but y = " << Y << " requested \n";
        return std::nan("");
    }
}

double LineEquation::getInclinationDegrees() const
{
    if ( !isValid() )
    {
        return 0;
    }
    double oSign = m_b < 0 ? -1 : 1;
    double oInclinationRadians =std::atan2(-oSign* m_a, +oSign* m_b);

#ifndef NDEBUG
    if ( isClose(m_b,0.0) )
    {
        assert(isClose(std::abs(oInclinationRadians), math::RIGHT_ANGLE_RAD));
    }
    else
    {
        double y0 = getY(0);
        double y1 = getY(1);
        double angle = std::atan2(y1-y0, 1.0);
        assert(isClose(oInclinationRadians, angle));

        //angle from x axis to this line
        LineEquation oXAxis(0, 0);//x axis is represented by y = 0*x+0
        double oAngleRespectAxis = oXAxis.computeAngleToLine(*this);
        assert(isClose(oInclinationRadians, oAngleRespectAxis, 1e-3));
    }
#endif
    return math::radiansToDegrees(constrainAngle(oInclinationRadians, angleUnit::eRadians, true));
}



std::ostream& operator<<(std::ostream& os, const LineEquation& obj)
{
    os << "Line (" << obj.m_a << "," << obj.m_b << "," << obj.m_c << ") ";
    if ( obj.m_b == 0)
    {
        os << "Line x = " << -obj.m_c / obj.m_a;
    }
    else
    {
        os << "m: " << - obj.m_a / obj.m_b << " q: " << -obj.m_c/obj.m_b;
    }
    return os;
}



LineEquation LineEquation::getParallelLinePassingThroughPoint(const LineEquation & line, double x, double y)
{
    double newC = -line.m_a*x - line.m_b *y;
    return {line.m_a, line.m_b, newC};
}

LineEquation LineEquation::getPerpendicularLinePassingThroughPoint(const LineEquation & line, double x, double y)
{
    double a1 = line.m_b;
    double b1 = -line.m_a;
    double c1 = -a1 * x - b1*y;
    return {a1, b1, c1};


}



} //namespaces
}
