/**************************************************************************************
 This software is developed by NEWSON NV
 Redistribution and use, with or without modification, are permitted
 
 THIS SOFTWARE IS PROVIDED BY NEWSON AS IS.
 ANY EXPRESS OR IMPLIED WARRANTIES ARE DISCLAIMED.
 IN NO EVENT SHALL NEWSON BE LIABLE FOR ANY DAMAGES ARISING
 OUT OF THE USE OF THIS SOFTWARE.
 **************************************************************************************/

//#include "stdafx.h"
#include "All.h"

ClfPoint::ClfPoint(double X, double Y)
{
	x=X;
	y=Y;
}

ClfPoint ClfPoint::operator*(const double& d) 
{
  return ClfPoint(x*d,y*d);
} 

ClfPoint ClfPoint::operator/(const double& d)
{
  return ClfPoint(x/d,y/d);
} 

ClfPoint ClfPoint::operator+(const ClfPoint& p)
{
  return ClfPoint(x+p.x,y+p.y);
} 

ClfPoint ClfPoint::operator-(const ClfPoint& p)
{
  return ClfPoint(x-p.x,y-p.y);
} 

ClfPoint ClfPoint::operator-()
{
  return ClfPoint(-x,-y);
} 

bool ClfPoint::operator>(const ClfPoint& p)
{ 
  return (x>p.x && y>p.y);
}

bool ClfPoint::operator<(const ClfPoint& p)
{ 
  return (x<p.x && y<p.y);
}

double ClfPoint::Length()
{
  return sqrt(x*x+y*y);
}

double ClfPoint::ArcTan()
{
  return atan2(y,x);
}

bool ClfPoint::operator!=(const ClfPoint& p)
{
  return (x!=p.x || y!=p.y);
} 

bool ClfPoint::operator==(const ClfPoint& p)
{ 
  return (x==p.x && y==p.y);
} 
