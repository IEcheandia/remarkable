/*
 * laserlineResults.h
 *
 *  Created on: 10.09.2010
 *      Author: KiH, HS
 */

#ifndef LASERLINERESULTS_H_
#define LASERLINERESULTS_H_

#include "Poco/SharedPtr.h"
#include "geo/point.h"
#include <cstdio>
#include <memory>




/**
* @brief LaserLine Ergebniss Struktur
*
* in der LaserLineResult1T Klasse liegen die tracking Resultate
* startpositionen,array mit den y Werten und den Intensitaeten 
*
*/
class LaserlineResultT
{
public:
	LaserlineResultT();
	~LaserlineResultT();

	int allocLaserLine(int nnelements);	///< Speicher allozieren
	void freeLaserLine(); 				///< allozierten Speicher freigeben: Achtung: ~LaserlineResultT macht das nicht !!!!
	void swap(LaserlineResultT &rhs);
	int getAllocated();

	unsigned long nalloced;             ///< Anzahl der allokierten Punkte fuer die Linie (i.d.R. ROI Breite)
	int *Y;								///< pointer auf Y Position
	int *I;								///< pointer auf intensitaet

	int firstValidIndex;				/// < erster gueltiger Wert
	int lastValidIndex;					/// < letzter gueltiger Wert

	bool bIsValid;

	precitec::geo2d::TPoint<int>  laserLineStartPos;  ///< startpunkt der Laserlinie (im Tracking ROI)
	precitec::geo2d::TPoint<int>  laserLineEndPos;    ///< Endpunkt der Laserlinie


private:
	unsigned long m_oNumberAllocated;		///< Anzahl allokierter Werte
	unsigned long m_oNumberCalled;          
	unsigned long m_oNumberFreed;
	std::unique_ptr<int[]> m_pY;			///< smart pointer zur Verwaltung der Laserlinien Y Werte
	std::unique_ptr<int[]> m_pI;			///< smart pointer zur Verwaltung der Laserlinien I Werte
	
};



#endif /* LASERLINERESULTS_H_ */
