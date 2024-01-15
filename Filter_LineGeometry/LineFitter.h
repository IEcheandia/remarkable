/**
 * 	@file
 * 	@copyright	Precitec Vision GmbH & Co. KG
 * 	@author		OS
 * 	@date		2015
 * 	@brief 		This class offers the functionality for calculating a line.
 */

#ifndef LINEFITTER_H_
#define LINEFITTER_H_


class LineFitter
{
public:
	LineFitter();
	void reset();
    void addPoint(double x, double y);
    void delPoint(double x, double y);
    bool calcMB(double & m, double & b);
    bool calcMeanY(double & m, double & b);

private:
	double Sx, Sy, Sxx, Sxy; // Container-Variablen zur Berechnung der Geraden
    int iPointAnz;
};

#endif /* LINEFITTER_H_ */
