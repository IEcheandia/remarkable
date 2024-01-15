/**************************************************************************************
 This software is developed by NEWSON NV
 Redistribution and use, with or without modification, are permitted
 
 THIS SOFTWARE IS PROVIDED BY NEWSON AS IS.
 ANY EXPRESS OR IMPLIED WARRANTIES ARE DISCLAIMED.
 IN NO EVENT SHALL NEWSON BE LIABLE FOR ANY DAMAGES ARISING
 OUT OF THE USE OF THIS SOFTWARE.
 **************************************************************************************/

class ClfPoint
{
public:
//' properties
  double x;
  double y;
  
//' constructors, destructors
  ClfPoint(double X=0, double Y=0);

//' operators
  bool operator!=(const ClfPoint& p);
  bool operator==(const ClfPoint& p);
  ClfPoint operator*(const double& d);
  ClfPoint operator/(const double& d);
  ClfPoint operator+(const ClfPoint& p);
  ClfPoint operator-(const ClfPoint& p);
  ClfPoint operator-();
  bool operator>(const ClfPoint& p);
  bool operator<(const ClfPoint& p);
  
//' methods
//  CPoint CPoint();
  double Length();
  double ArcTan();
};
