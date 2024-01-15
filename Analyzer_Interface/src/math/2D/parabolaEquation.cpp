#include "math/2D/parabolaEquation.h"

namespace precitec {
namespace math {


//////////////////////////////////////////////
/*********************************************
CLASS ParabelFit
*********************************************/
//////////////////////////////////////////////

ParabolaEquation::ParabolaEquation()
{
    Reset();
}

ParabolaEquation::~ParabolaEquation()
{}

void ParabolaEquation::Reset()
{
    iPointAnz = 0;
    S01 = 0;
    S11 = 0;
    S21 = 0;
    S10 = 0;
    S20 = 0;
    S30 = 0;
    S40 = 0;
}

void ParabolaEquation::AddPoint(double x, double y)
{
    iPointAnz++;

    S01 += y;
    double xy = x*y;
    S11 += xy;
    S21 += x*xy;

    S10 += x;
    double xx = x*x;
    S20 += xx;
    S30 += x*xx;
    S40 += xx*xx;
}

void ParabolaEquation::DelPoint(double x, double y)
{
    if ( iPointAnz <= 0 ) return;

    iPointAnz--;

    S01 -= y;
    S11 -= x*y;
    S21 -= x*x*y;

    S10 -= x;
    S20 -= x*x;
    S30 -= x*x*x;
    S40 -= x*x*x*x;
}

bool ParabolaEquation::CalcABC(double & a, double & b, double & c)
{
    a = b = c = 0.0;

    if ( iPointAnz < 3 ) return false;

    double A = iPointAnz*S30 - S10*S20;
    double B = iPointAnz*S20 - S10*S10;

    double C = iPointAnz*S40 - S20*S20;
    double D = iPointAnz*S30 - S10*S20;

    double dZaehler = B * (iPointAnz*S21 - S01*S20) - A * (iPointAnz*S11 - S01*S10);
    double dNenner = B * C - A * D;

    a = dZaehler / dNenner;

    b = (iPointAnz*S11 - S01*S10 - a*(iPointAnz*S30 - S10*S20)) / B;

    c = (S01 - a*S20 - b*S10) / iPointAnz;

    return true;
}

} // namespaces
}


