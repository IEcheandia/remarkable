/*
 * laserlineTracker1.h
 *
 *  Created on: 10.09.2010
 *      Author: KiH, HS
 */

#ifndef LASERLINETRACKER1_H_
#define LASERLINETRACKER1_H_

const int LINETRACKER1_Y_INVALID =  -1234;
const int LINETRACKER1_I_INVALID =  -5678;
 
#include "souvisSourceExportedTypes.h"
#include "laserlineResults.h"
#include "image/image.h"
#include "common/frame.h"
#include "common/defines.h"

using namespace precitec;
using namespace image;


//**********************************************************************************
// Hilfsstrukturen fuer Berechnung

class linePosition
{
public:
	int		x;
	int		y;
	int		iGrey;
	linePosition();

};


class LaserlineTracker1ParametersT
{
public:
	LaserlineTracker1ParametersT();
	int debuglevel;
	int iTrackStart;
	int iSuchSchwelle;
	bool doubleTracking;
	int upperLower;		//0=UPPER_LASERLINE   1=LOWER_LASERLINE,

    int upperMaxLineDiff;
    int lowerMaxLineDiff;
    bool lapJoin;

	int iMittelungX; //iPixelX;
	int iPixelY; ///< Search area y, by default upper and lower. Only upper search area if iPixelYLower > 0
	int iPixelYLower; ///< Lower search area y

	int iMittelungY;	//Mittelung Y
	int iAufloesungX;
	int iAufloesungY;
	int iMaxBreiteUnterbruch;
	int iMaxAnzahlUnterbrueche;
	int iMaxLinienSprungY;  //iMaxLinienspringY;

	int startAreaX;
	int startAreaY;
};


/**
* @brief haelt die tracking methoden und tracking ergebnisse
* in der LaserLineResult1T Klasse
*
*/
class LaserlineTracker1T
{
public:


	class geo2dRange
	{
		public:
		int start_; 				///< untere Intervallgrenze
		int end_;					///< obere Intervallgrenze

	};

	LaserlineTracker1T();
	~LaserlineTracker1T();

	LaserlineResultT result;			///< tracking ergebnisse Y,I,first u. last valid
	LaserlineTracker1ParametersT m_par;	///< tracking parameter

	int alloc(int npixx,int npixy);
	int process(const interface::ImageFrame *p_pFrame);

	//void drawresult();
	void printresult();

private:

	/**
	* @brief Lininesuche
	* ruft im Wesentlichen searchLinesUpperLower auf
	* \return   
	*
	*/
	int searchLine();
	
	/**
	* @brief Sucht die Startpunkte zum Tracken der Laserlinie
	* Entlang der aeussersten Spalten in ROI wird das MAximum gesucht
	* und mit 70% der Suchschwelle verglichen.
	* Es wird ueber mehrere Spalten nach innen gehend gesucht - bis 30% ROI Breite
	*
	* \param    xPos - x Position der Spalte in der gesucht wird
	* \param    rgSearchingAreaY - Suchbereich
	* \return   
	*
	*/
	int searchLinesUpperLower(int threshold);
	
	/**
	* @brief Start des trackings
	* Ruft im Wesentlichen trackingtracklines auf
	* \param    xPos - x Position der Spalte in der gesucht wird
	* \param    rgSearchingAreaY - Suchbereich
	* \return   
	*
	*/
	bool trackLines();
	
	/**
	* @brief Tracken der Laserlinie
	* Entlanglaufen auf der Laserlinie von dern Startpunkten aus.
	* Es wird immer hellste Punkt unter Beruecksichtigung der Parameter genommen
	* \param    trackDir Laufrichtung
	* \param    startPos Startpositionen 
	* \return   bool
	*
	*/
	bool trackingtrackLines(int trackDir, const linePosition &startPos, const linePosition &p_rEndPos);
	
	void TRangeSchnittmenge(geo2dRange & dest, geo2dRange & source1, geo2dRange & source2);
	void mittel1D(int *fkt, int iStartX, int iEndeX, int iFilterGroesse);
	void median1D(int* histo, int iStartX, int iEndeX);


	/**
	* @brief Maximumsuche ueber den Suchbereich
	*
	* \param    xPos - x Position der Spalte in der gesucht wird
	* \param    rgSearchingAreaY - Suchbereich
	* \return   
	*
	*/
	int findMaxInColumn(unsigned int xPos,  geo2dRange & rgSearchingAreaY);
	
	
	/**
	* @brief Berechnet das Maximum an der Stelle 
	* laeuft ueber y Mittelung und x Mittelung
	*
	* \param    xPos - x Position der Spalte in der gesucht wird
	* \param    yPosAlt
	* \param    yPosNeu1
	* \param    yPosNeu2
	* \param    max
	* \return   boolr
	*
	*/
	bool findMax(int xPos, int yPosAlt, int &yPosNeu1,int &yPosNeu2, int &max);

	
	/**
	* @brief Ueberprueft der gefundenen neuen Punkt des tarckings 
	*
	* \param    yTemp
	* \param    x
	* \param    laserline
	* \param    iDelta
	* \param    countGapPoints
	* \param    countGaps
	* \return   bool
	*
	*/
	bool checkTrackPoint(int& yTemp, int&	x, int* laserLine, int iDelta, int& countGapPoints,int& countGaps);

	void computeIntegralImage();
	int computeSum(int x1, int x2, int y1,int y2) const;

	int m_iTrackStopXLeft;             ///< tracking stop positionen
	int m_iTrackStopXRight;
	int m_iTrackError;					///< tracking Abbruch
	linePosition		endMw[2]; 		///< naht rand positionen
    linePosition     	vLinePos[2];	///< tracking start positionen
	SSF_SF_InputStruct internimage;
	int tempLaserLine[MAX_CAMERA_WIDTH];

	int rcroi_xstart;					///< tracking roi
	int rcroi_ystart;
	int rcroi_xend;
	int rcroi_yend;
    bool m_IntegralImageComputed;
    TLineImage<int> m_IntegralImage;
    std::vector<int> m_oBorderLine; // intermediate sum, for searchLinesUpperLower

    int m_lineStartY; ///< first y value of the laserLine

};



#endif /* LASERLINETRACKER1_H_ */
