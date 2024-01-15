
/**
 * 	@file
 * 	@copyright	Precitec Vision GmbH & Co. KG
 * 	@author		OS
 * 	@date		2015
 * 	@brief 		This class offers the functionality for calculating a line.
 */


#include "LineFitter.h"



LineFitter::LineFitter()
{
	reset();
}

void LineFitter::reset()
{
    iPointAnz=0;
    Sx=0;
    Sy=0;
    Sxy=0;
    Sxx=0;
}

void LineFitter::addPoint(double x, double y)
{
    iPointAnz++;
  //printf("Punkt Nr. %d geaddet!\n", iPointAnz);

    Sy  += y;
    Sxy += x*y;
    Sx  += x;
    Sxx += x*x;
}

void LineFitter::delPoint(double x, double y)
{
    if (iPointAnz<=0) return;

    iPointAnz--;

    Sy  -= y;
    Sxy -= x*y;
    Sx  -= x;
    Sxx -= x*x;
}

bool LineFitter::calcMB(double & m, double & b)
{
    m = b = 0.0;

    if (iPointAnz < 2)
        return false;

    if (Sxx == 0)
        return false;

    double Teiler = iPointAnz - (Sx * Sx / Sxx);
    //printf("Teiler = %f\n", Teiler);

    if (Teiler == 0)
        return false;

    double Konstante = Sy - (Sxy * Sx / Sxx);
    //printf("Konstante = %f\n", Konstante);
    b = (Konstante / Teiler);
    //printf("b = %f\n", b);

    m = (Sxy - Sx * b) / Sxx;

    return true;
}

bool LineFitter::calcMeanY(double & m, double & b)
{
    m = b = 0.0;

    if (iPointAnz < 2)
        return false;

    if (Sy == 0)
        return false;

    b = (Sy / iPointAnz);
    //printf("b = %f\n", b);

    return true;
}
