/*
 * laserlineTracker1.cpp
 *
 *  Created on: 10.09.2010
 *      Author: KiH, HS
 */
 
#include "laserlineTracker1.h"

#include <stdlib.h>
#include <string>
#include <cstdio>

#include "laserlineResults.h"
#include "module/moduleLogger.h"

#define UPPER_LASERLINE  0
#define LOWER_LASERLINE  1

#define  VON_LINKS  0
#define  VON_RECHTS 1

#define  INT_L  0
#define  INT_R  1

using namespace precitec;
using namespace image;


#ifndef NDEBUG
//#define COMPAREOLDIMPLEMENTATION
#endif

linePosition::linePosition()
{
	x=0;
	y=0;
	iGrey=0;
}


LaserlineTracker1ParametersT::LaserlineTracker1ParametersT()
{

}

//****************************************************************************************
LaserlineTracker1T::LaserlineTracker1T()
{

	m_par.upperLower=0;  //0=UPPER_LASERLINE   1=LOWER_LASERLINE,
	m_par.doubleTracking=true;
	m_par.iTrackStart=2;
	m_par.iSuchSchwelle=50;
	m_par.iMaxBreiteUnterbruch=5;
	m_par.iMaxAnzahlUnterbrueche=2;
	m_par.iMaxLinienSprungY=5; //  iMaxLinienspringY=50;
	m_par.iAufloesungX=1;
	m_par.iMittelungX=2; //    par.iPixelX=2;
	m_par.iPixelY=5;
    m_par.iPixelYLower = -1;
	m_par.iAufloesungY=1;
	m_par.iMittelungY=1;
    m_par.lapJoin = false;
}

LaserlineTracker1T::~LaserlineTracker1T()
{
	//std::cout<<"DTOR vor free LaserLineTracker1T "<<std::endl;
	result.freeLaserLine();//wichtig
	//std::cout<<"DTOR durch..."<<std::endl;
}


int LaserlineTracker1T::alloc(int npixx,int npixy)
{
	int status = 0;
	status = result.allocLaserLine(npixx);

	return status;
}

/*
void LaserlineTracker1T::drawresult()
{

int x,y;
for(x=0;x<internimage.npixx;++x)
	{
		y=result.Y[x];
		if(y>internimage.npixy || y<0) continue;
		*(internimage.img+x+y*internimage.pitch)=0;
	}

}
*/


void LaserlineTracker1T::printresult() {

	int x, y;

//	wmLog(eDebug, "firstValidIndex=%d   lastValidIndex=%d \n", result.firstValidIndex,
//			result.lastValidIndex);
	for (x = result.firstValidIndex; x < result.lastValidIndex; ++x) {
		y = result.Y[x];
		if (y > internimage.npixy || y < 0)
			continue;
		//wmLog(eDebug, "x=%d y=%d   I=%d\n", x, y, result.I[x]);
	}

}




int LaserlineTracker1T::process(const interface::ImageFrame *p_pFrame)
{

	int ret;
	m_IntegralImageComputed = false;
	result.bIsValid=false;
	
	if (p_pFrame == nullptr)
	{
		return -1;
	}

	internimage.img = p_pFrame->data().begin();
	internimage.npixx =p_pFrame->data().width();
	internimage.npixy=p_pFrame->data().height();
	if ( internimage.npixy < 2 )
	{
		return -1;
	}

	internimage.pitch=p_pFrame->data().rowBegin(1) - p_pFrame->data().rowBegin(0); //TODO:Bug!!!!????
	internimage.roistart=internimage.img;
	internimage.roix0=0;
	internimage.roiy0=0;
	internimage.roidx=internimage.npixx;
	internimage.roidy=internimage.npixy;
        
    //internimage has been changed, the corresponding cache will be reset (if needed) in track lines

	int dummyCtr = result.getAllocated();

	if (internimage.npixx > dummyCtr)
		result.allocLaserLine(internimage.npixx);

	dummyCtr = result.getAllocated();

	for(int i=0;i<dummyCtr;++i)
	{
		result.Y[i]=LINETRACKER1_Y_INVALID;
		result.I[i]=LINETRACKER1_I_INVALID;
	}


	if (m_par.iAufloesungX == 1 && m_par.iAufloesungY == 1) 
	{
		computeIntegralImage();
	}


	result.firstValidIndex=-1;
	result.lastValidIndex=-1;

	/// Startpunkte der Linie suchen
	ret = searchLine();
	if(ret!=0) // Keinen gueltigen Bilddaten ( NULLptr)
		return ret;

	//std::cout<<"vLinePos[INT_L] "<<vLinePos[INT_L].x<<" "<<vLinePos[INT_L].y<<" "<<vLinePos[INT_L].iGrey<<std::endl;
	//std::cout<<"vLinePos[INT_R] "<<vLinePos[INT_R].x<<" "<<vLinePos[INT_R].y<<" "<<vLinePos[INT_R].iGrey<<std::endl;

	if( (vLinePos[INT_L].x >= 0) && (vLinePos[INT_L].y >=0))
	{
		result.laserLineStartPos.x =vLinePos[INT_L].x;
		result.laserLineStartPos.y =vLinePos[INT_L].y;
	}
	if( (vLinePos[INT_R].x >= 0) && (vLinePos[INT_R].y >=0 ))
	{
		result.laserLineEndPos.x =vLinePos[INT_R].x;
		result.laserLineEndPos.y =vLinePos[INT_R].y;
	}
	
	
	// Track the line
	ret = trackLines();

	//std::cout<<"LaserlineTracker1T::process nach trackLines... "<<std::endl;

	if (ret != 0)
		return ret;

	result.bIsValid=true;
	return 0;
}

//***********************************************************************************************************
//***********************************************************************************************************
// Hilfsroutinen fuer die Berechnung

void LaserlineTracker1T::computeIntegralImage()
{
    assert(!m_IntegralImageComputed && "Computing integral image wihout having invalidated it first");
    const unsigned int BORDER = 1; //integral image has a left and upper border filled with 0

    m_IntegralImage.resize(geo2d::Size(internimage.roidx + BORDER, internimage.roidy + BORDER));

    //make sure first row is filled with 0
    std::fill_n(m_IntegralImage.rowBegin(0), m_IntegralImage.width(), 0);

    for (unsigned int y_input = internimage.roiy0, y_int = 0, last_y_input = internimage.roiy0 + internimage.roidy; 
            y_input < last_y_input; 
          ++y_input, ++y_int )
    {

        assert(y_int == (y_input - internimage.roiy0 ));
        auto rIntegralImageCurrentRow = m_IntegralImage.rowBegin(BORDER + y_int);
        auto rIntegralImagePreviousRow = m_IntegralImage.rowBegin(BORDER + y_int - 1);
        auto rImageInCurrentRow = internimage.roistart + y_input*internimage.pitch;

        rIntegralImageCurrentRow[0] = 0; //make sure first column is filled with 0
        
        auto pIntegralImageAbove = rIntegralImagePreviousRow; //BORDER+0-1
        auto pIntegralImage = rIntegralImageCurrentRow;
        auto pInputImage = rImageInCurrentRow+internimage.roix0;
        
        for ( unsigned int x_input = internimage.roix0, x_int = 0, last_x_input = internimage.roix0 + internimage.roidx ; 
                x_input < last_x_input; 
                ++x_input,++x_int )
        {
            assert(x_int == (x_input - internimage.roix0));
            
            int val = (*pInputImage) - (*pIntegralImageAbove) ;
            val += (*(++pIntegralImageAbove)) + *pIntegralImage;
            ++pIntegralImage;
            (*pIntegralImage) = val;
            
            assert(rIntegralImageCurrentRow[BORDER+x_int] == 
                    rImageInCurrentRow[x_input] - rIntegralImagePreviousRow[BORDER + x_int - 1] + rIntegralImagePreviousRow[BORDER + x_int] + rIntegralImageCurrentRow[BORDER + x_int - 1]);
            ++pInputImage;

        }
    }
    m_IntegralImageComputed = true;
}
    

int LaserlineTracker1T::computeSum(const  int xMinInput, const int xMaxInput, const int yMinInput, const int yMaxInput) const
{
    assert(m_IntegralImageComputed);
    assert(m_par.iAufloesungX == 1 && m_par.iAufloesungY == 1);
    const int xMinIntegral = (xMinInput - internimage.roix0);
    const int xMaxIntegral = (xMaxInput - internimage.roix0);
    const int yMinIntegral = (yMinInput - internimage.roiy0);
    const int yMaxIntegral = (yMaxInput - internimage.roiy0);
    
#ifndef NDEBUG
    assert(xMinIntegral< xMaxIntegral && yMinIntegral < yMaxIntegral);
    
    for (auto && value : {xMinIntegral, xMaxIntegral})
    {
        assert(value >= 0);
        assert(value < m_IntegralImage.width());
    }
    for (auto && value : { yMinIntegral, yMaxIntegral})
    {
        assert(value >= 0);
        assert(value < m_IntegralImage.height());
    }
#endif
    
    auto * firstRow = m_IntegralImage.rowBegin(yMinIntegral);
    int A =*(firstRow+xMinIntegral);
    int B =*(firstRow+xMaxIntegral);

    auto * lastRow = m_IntegralImage.rowBegin(yMaxIntegral);
    int C = *(lastRow+xMinIntegral);
    int D = *(lastRow+xMaxIntegral);

    return D - B - C + A;
    
}


/************************************************************************
* Description: Tracking der Laserlinien                                 *
*              In den jeweiligen Tracking-Methoden soll folgendes in    *
*              presults gespeichert werden:                             *
*              aLaserLineY     --> Laserlinie-Koordinaten               *
*              aLaserLineGrey  --> Laserlinie-Grauwerte                 *
*              Darueberhinaus soll ein moeglicher TrackError bzw.       *
*              Linienunterbruch ermittelt werden                        *
*                                                                       *
* Parameter  : ppar:        Pointer to the internal parameter structure *
*              presults:    Pointer to the internal result structure    *
*              upperLower:  = 0: fuer obere Linie                       *
*                           = 1: fuer untere Linie                      *
************************************************************************/
bool LaserlineTracker1T::trackLines()
{
	int x,y;
	bool bTrackError = true;
	const auto & par =m_par; // const reference as a compile check that the parameters are not being modified  (that would invalidate the sum intensity cache)
	
	rcroi_xstart=internimage.roix0+par.iMittelungX; //PixelX;		 //tracking roi um Mittelung x nach innen
	rcroi_ystart=internimage.roiy0+par.iMittelungY;  //tracking roi um mittelung y nach innen

	int dx=internimage.roidx-2*par.iMittelungX; //  iPixelX;          // hoehe und breite anpassen
	int dy=internimage.roidy-2*par.iMittelungY;

	if(dx<0)  dx=0;
	if(dy<0)  dy=0;
	rcroi_xend=rcroi_xstart+dx;  
	rcroi_yend=rcroi_ystart+dy;
    
	// tracking von links und von rechts
	if (par.doubleTracking)
	{
		// Tracking Reserve als Souvis1 oder Souvis2 Unterscheidung ???
		int xstopLinks=0;
		vLinePos[INT_L].y+= internimage.roiy0;
		vLinePos[INT_R].y+= internimage.roiy0;

		// mit ROI Werten vorinitialisieren, falls tracking durchlaeuft
		endMw[INT_L].x = rcroi_xend;
		endMw[INT_R].x = rcroi_xstart;
		
		bTrackError = trackingtrackLines(VON_LINKS, vLinePos[INT_L], vLinePos[INT_R]);
		if(bTrackError)
		{
			
			xstopLinks=m_iTrackStopXRight; // Tracking von links
			endMw[INT_L].x = xstopLinks;
		}

		//laserlinie kopieren
		//laserlinie kopieren
		int nbytes= result.getAllocated() * sizeof(int); 
		if( static_cast<unsigned int>(nbytes) > sizeof(tempLaserLine))
		{
			wmLog(eDebug, "problems when copying to tempLaserLine\n");
		}
		memcpy(tempLaserLine,result.Y, nbytes);

		//debug
		//for(int z=0;z<xstopLinks;z++)
		//  std::cout<<z<<" "<<tempLaserLine[z]<<" "<<result.Y[z]<<std::endl;

		//std::cout<<" "<<std::endl;
		
		int xstopRechts = 0;
		bTrackError = trackingtrackLines(VON_RECHTS, vLinePos[INT_R], vLinePos[INT_L]);
		if(bTrackError)
		{
			
			xstopRechts=m_iTrackStopXLeft;
			endMw[INT_R].x= xstopRechts;

		}
		
		bTrackError = 0;

		//debug
		//for(int z=endMw[INT_R].x;z<endMw[INT_L].x;z++)
		//  std::cout<<z<<" "<<tempLaserLine[z]<<" "<<result.Y[z]<<std::endl;

		//for(int z=result.firstValidIndex;z<result.lastValidIndex  ;z++)
		//  std::cout<<z<<" "<<tempLaserLine[z]<<" "<<result.Y[z]<<std::endl;

		//std::cout<<" "<<std::endl;
		// Vergleich der Laserlinien links rechts beim Durchtracken
		// eventuell pruefen ob schon Werte im Array liegen , dann geeignete strategie waehlen:
		// hellere Punkte oder die geometrisch tieferen Werte
		// in welcher Richtung ist die Reflexion ??


		//Zwischen dem Stop der Auswerung von Links und dem Stop der Auswertung von Rechts: bestes aus beiden Auswertungen suchen
		// endMw[INT_L].x : Stop der Laserlinie von links am rechten Ende
		// endMw[INT_R].x : Stop der Laserlinie von rechts am linken Ende

	

		//Max BReite Unterbruch beruecksichtigen...
		int linkeSeite = endMw[INT_R].x- par.iMaxBreiteUnterbruch -1;
		int rechteSeite= endMw[INT_L].x+par.iMaxBreiteUnterbruch + 1;
		
		if( linkeSeite < result.firstValidIndex)
			linkeSeite =  result.firstValidIndex;

        if (rechteSeite > result.lastValidIndex)
			rechteSeite = result.lastValidIndex;

		for(int x=linkeSeite; x<rechteSeite;x++)
		{
			// Abhaengig von der Laserrichtung
			// default: Obere Linie - Laser von unten im Bild
			// Reflexion ueber der Linie im Bild
			if( par.upperLower == UPPER_LASERLINE)
			{
				if(   tempLaserLine[x]>0 && tempLaserLine[x] > result.Y[x])
					result.Y[x] = tempLaserLine[x];
			}
			else if(par.upperLower == LOWER_LASERLINE)// hier ist die Reflexion immmer unten im Bild
			{
				//std::cout<<x<<" "<<tempLaserLine[x]<<" "<<result.Y[x]<<std::endl;
				if(   tempLaserLine[x]>0 && tempLaserLine[x] < result.Y[x] )
				{
					result.Y[x] = tempLaserLine[x];
					//std::cout<<result.Y[x]<<std::endl;
				}
			}
		}

		//debug
		//for(int z=endMw[INT_R].x;z<endMw[INT_L].x;z++)
		//  std::cout<<z<<" "<<result.Y[z]<<std::endl;

	}
	else
	{
		int trackDir = VON_LINKS;
		// we start from the brighter side assuming we have a lower chance of getting really lost
		linePosition startPos;
		linePosition endPos;

		if(par.iTrackStart == 0)
		{
			//std::cout<<"von links"<<endLine;
			trackDir = VON_LINKS;
			startPos  = vLinePos[INT_L];
			endPos = vLinePos[INT_R];
		}
		else if(par.iTrackStart == 1)
		{
			//std::cout<<"von rechts"<<endLine;
			trackDir = VON_RECHTS;
			startPos  = vLinePos[INT_R];
			endPos = vLinePos[INT_L];
		}
		else if(par.iTrackStart == 2)
		{
			//std::cout<<"Automatik"<<endLine;
			if (vLinePos[INT_L].iGrey > vLinePos[INT_R].iGrey)
			{
				trackDir = VON_LINKS;
				startPos  = vLinePos[INT_L];
				endPos = vLinePos[INT_R];
			}
			else
			{
				trackDir = VON_RECHTS;
				startPos  = vLinePos[INT_R];
				endPos = vLinePos[INT_L];
			}
		}


		startPos.y += internimage.roiy0;
		endPos.y += internimage.roiy0;    // Der y-Wert wird wohl nicht benoetigt, insofern ist es egal, aber wir setzen das trotzdem.

		//std::cout<<"startpositionen: "<<startPos.x<<" ,"<<startPos.y<<std::endl;

		bTrackError = trackingtrackLines( trackDir, startPos, endPos);
	}

	//Helligkeitswerte erst hier eingetragen
	const int num_elements = result.getAllocated();
	for(x=result.firstValidIndex;x<=result.lastValidIndex;++x)
	{
		if ((0 <= x) && (x < num_elements))   // Extra-Abfrage um ganz sicher zu gehen.
		{
			y = result.Y[x];
			//std::cout<<x<<" "<<result.Y[x]<<std::endl;

			if (y>internimage.npixy || y < 0)
				result.I[x] = -1; //Achtung bei Umstellung auf char !!!!!!!!!
			else
				result.I[x] = *(internimage.img + x + y*internimage.pitch);
		}
	}

    //std::cout<<"trackLines beendet... "<<std::endl;
	return bTrackError;
} // trackLines


/*
// Schnittmenge aus zwei Tranges
Liefert den Schnitt aus zwei TRanges, sofern er existiert.
Existiert kein Schnitt, liefert die Funktion den Range zurueck in dem source1 liegen sollte.
*/

void LaserlineTracker1T::TRangeSchnittmenge(geo2dRange & dest, geo2dRange & source1, geo2dRange & source2)
{
	dest.start_= (source1.start_>source2.start_) ? source1.start_ : source2.start_ ;
	dest.end_= (source1.end_<source2.end_) ? source1.end_ : source2.end_ ;

}



//***********************************************************************************************

/************************************************************************
* Description: Bestimmt den naechsten Tracking-Punkt                    *
*              in der naechsten Spalte                                  *
*                                                                       *
* Parameter  : xPos:   entsprechende Spalte                             *
*              interval: Suchbereich in der entsprechenden Spalte       *
************************************************************************/

int LaserlineTracker1T::findMaxInColumn(unsigned int xPos,  geo2dRange & rgSearchingAreaY)
{
    const auto & par =m_par; // const reference as a compile check that the parameters are not being modified  (that would invalidate the sum intensity cache)

	int max=0;
	int yPosNeu1=0;
	int yPosNeu2=0;
	int endSearchingAreaY=rgSearchingAreaY.end_;

	
	// Schleife ueber das Spaltenintervall von oben nach unten  BAUSTELLE XAX
	max = 0;
	yPosNeu1 = LINETRACKER1_Y_INVALID;
	yPosNeu2 = LINETRACKER1_Y_INVALID;

	//maximum suche im Suchbereich
	for(int yPosAlt = rgSearchingAreaY.start_; yPosAlt < endSearchingAreaY; yPosAlt += par.iAufloesungY)
	{
		//Berechnet Maximum an der Stelle x,yPosAlt uber MittelungY Pixel
		findMax(xPos, yPosAlt, yPosNeu1, yPosNeu2, max);
	} // for Spalte
	// Rueckgabe des Mittelwerts der beiden Suchmethoden
	//wmLog(eDebug, "%d %d %d ******************************\n", yPosNeu1,yPosNeu2, (yPosNeu1 + yPosNeu2)/2);
	if(yPosNeu1==LINETRACKER1_Y_INVALID) return yPosNeu2;
	if(yPosNeu2==LINETRACKER1_Y_INVALID) return yPosNeu1;

	return (yPosNeu1 + yPosNeu2) / 2;
/**/
}


/************************************************************************
* Description: Ermittelt den naechsten Tracking-Punkt                   *
*              in der naechsten Spalte                                  *
*                                                                       *
* Parameter  : xPos:   entsprechende Spalte                             *
*              yPosAlt: y-Position des bisherigen Punktes               *
*              yPosNeu: y-Position des naechsten Punktes                *
*              max: Hilfsvar. zur Bestimmung des naechsten Track-Pktes  *
************************************************************************/

bool LaserlineTracker1T::findMax(int xPos, int yPosAlt, int &yPosNeu1,int &yPosNeu2, int &max)
{
    const auto & par = m_par; // const reference as a compile check that the parameters are not being modified  (that would invalidate the sum intensity cache)

    int startMittelungX = xPos - par.iMittelungX;
    int endMittelungX = xPos + par.iMittelungX;
    int endMittelungY = yPosAlt + par.iMittelungY;
    
    bool useIntegralImage = par.iAufloesungX == 1 && par.iAufloesungY == 1 &&  m_IntegralImageComputed;
    
    #ifdef COMPAREOLDIMPLEMENTATION
    useIntegralImage = false;
    #endif
    
    int sum;
    
    if (useIntegralImage)
    {    
        sum = computeSum(startMittelungX, endMittelungX+1,yPosAlt-par.iMittelungY, yPosAlt+par.iMittelungY+1);
    }
    else
    {
        sum = 0;

        // aufsummieren ueber y suchbereich
    
        const unsigned char * p_y = internimage.img + (yPosAlt-par.iMittelungY) * internimage.pitch;
        
        for (int i = yPosAlt-par.iMittelungY; i <= endMittelungY; ++i, p_y+=internimage.pitch )
        {      
            // Grauwerte in x-Richtung aufsummieren //mittelung X
            const unsigned char * p_x = p_y + startMittelungX;
            for (int x = startMittelungX; x <= endMittelungX; x += par.iAufloesungX, p_x += par.iAufloesungX)
            {
                sum += (*p_x);
            }
        }
        
        #ifdef COMPAREOLDIMPLEMENTATION
        if (m_IntegralImageComputed)
        {    
            assert( sum == computeSum(startMittelungX, endMittelungX+1,yPosAlt-par.iMittelungY, yPosAlt+par.iMittelungY+1));
        }
        #endif
                        

    }
    

	if( sum > max )
	{
		max = sum;
		yPosNeu1 = yPosAlt;
		yPosNeu2= yPosAlt;
	}
	else if( sum == max )
	{
		yPosNeu2= yPosAlt;
	}
	return true;
}


//***********************************************************************************************

bool LaserlineTracker1T::trackingtrackLines(int trackDir, const linePosition &startPos, const linePosition &p_rEndPos)
{

    const auto & par =m_par; // const reference as a compile check that the parameters are not being modified  (that would invalidate the sum intensity cache)

	int  uStart, uEnd;            // Start und Ende des ROI
	int  iDelta;                  // Trackrichtung
	int  x, yTemp;                // Aktuelle Position
	int  countGapPoints  =   0;   // Zaehlvariable fuer die Unterbruchlaenge
	int  countGaps       =   0;   // Zaehlvariable fuer die Anzahl der Unterbrueche
	bool bTrackError   = false;

	//std::cout<<"trackingtrackLines start "<<std::endl;


	m_iTrackError = 0;
	if (trackDir == VON_RECHTS)
	{

	
		uStart = startPos.x;
		uEnd   = rcroi_xstart - 1;  // uEnd wird bei der Suche nicht mehr eingeschlossen.

		// test
		if( uStart > (rcroi_xend - 1))
		{
			//std::cout<<"Tracking start ausserhalb ROI: uStart-rcroi_xend "<<uStart<<"-"<<rcroi_xend<<std::endl;
			uStart = rcroi_xend -1;  // wichtig, ohne der -1 haben wir Speicherzugriffsfehler bekommen.

		}
		if (uEnd < (p_rEndPos.x-1))  // wenn uEnd < p_rEndPos.x, dann suchen wir zu weit
		{
			// Wenn wir zu weit suchen wollen, dann setze das entsprechend:
			uEnd = p_rEndPos.x - 1;
		}
		iDelta = -1;
	}
	else
	{

		uStart = startPos.x;
		uEnd   = rcroi_xend;

		if(uStart < rcroi_xstart)
		{
			//std::cout<<"Tracking start ausserhalb ROI: uStart-rcroi_xstart"<<uStart<<"-"<<rcroi_xstart<<std::endl;
			uStart = rcroi_xstart;

		}
		if (uEnd > (p_rEndPos.x + 1))  // wenn uEnd >= p_rEndPos.x, dann suchen wir zu weit
		{
			uEnd = p_rEndPos.x + 1;
		}
		iDelta = 1;
	}

	// y-Startwert fuer das Tracking am linken Rand bestimmen

	//uebernehme startwert von searchLines
	//laserlinie kunstlich verbreitet wegen der rcRoi_-Erodierung
	//for(int i=0;i<=par.iPixelX;i++)
	for(int i=0;i<=par.iMittelungX;i++)
	{
		int oTemp = uStart - i * iDelta;
		if ((0 <= oTemp) && (oTemp < result.getAllocated()))  // um ganz sicher zugehen, ob das hier valide Indizes sind, fragen wir nochmal extra ab. Wir haben degenerierte Situationen gesehen, in denen hier ein Speicherzugriffsfehler passiert (z.B. uStart =1 und uEnd=1)
		{
			result.Y[oTemp] = startPos.y; // laserLine[uStart-i*iDelta] = startPos.y;
		}
	}

	geo2dRange rgRoiTmpY;  //Y Bereich Tracking ROI
	rgRoiTmpY.start_= rcroi_ystart;
	rgRoiTmpY.end_= rcroi_yend;

    m_lineStartY = LINETRACKER1_Y_INVALID;
    const auto actualPixelYLower = (par.iPixelYLower < 0) ? par.iPixelY : par.iPixelYLower;

	// ueber die komplette Breite des LINE_ROI
	for(x = uStart + iDelta; ((x < uEnd) && (trackDir == VON_LINKS)) || ((x > uEnd) && (trackDir == VON_RECHTS)); x += iDelta)
	{

		geo2dRange rgSearchingAreaY; //resultierender Suchbereich nach Ueberpruefung 


        geo2dRange source2; //Suchbereich Y
        source2.start_ = result.Y[x-iDelta] - par.iPixelY;
        source2.end_ = result.Y[x-iDelta] + actualPixelYLower + 1;


		//liegt der Suchbereich y im ROI ?
		TRangeSchnittmenge(rgSearchingAreaY, rgRoiTmpY,source2);

		//Suche nach dem Helligkeitsmaximum im Suchbereich
		yTemp = findMaxInColumn (x, rgSearchingAreaY);

		// TrackError pruefen
		// maxUnterbruch usw. werden hier geprueft...
		if (!checkTrackPoint(yTemp, x, result.Y, iDelta, countGapPoints, countGaps))
		{
			bTrackError = true;
			break;
		}

		//Wert war unterhalb Schwelle :  gerade weiterlaufen, letzten Y wert nehmen
		if(m_iTrackError == 228)
		{
			yTemp = result.Y[x - iDelta];
			m_iTrackError = 0; //zuruecksetzen
		}

		// Shared-Memory-Variablen fuellen
		result.Y[x]      = yTemp;
		//result.I[x] = internimage.img[x+result.Y[x]*internimage.pitch]; //Bugfix zu Souvis: das erst spaeter eingetragen
	}

	if(!bTrackError)
	{
		//laserlinie kunstlich verbreitet wegen der rcRoi_-Erodierung
		//for(int i=0;i<par.iPixelX;i++)
		for(int i=0;i<par.iMittelungX;i++)
		{
			int oIndex1 = uEnd + i * iDelta;
			int oIndex2 = uEnd - iDelta;
			bool oIndex1Valid = (0 <= oIndex1) && (oIndex1 < result.getAllocated());
			bool oIndex2Valid = (0 <= oIndex2) && (oIndex2 < result.getAllocated());
			if (oIndex1Valid && oIndex2Valid)
			{
				// Extra-ueberpruefung: in degenerierten Faellen (z.B. uStart = 1, uEnd = 1) kann es vorkommen, dass wir hier einen Speicherzugriffsfehler verursachen.
				result.Y[oIndex1] = result.Y[oIndex2];
			}
		}
	}

	// zu klaeren...
	int uScanEnd   = x - iDelta;
	if (!bTrackError)
	{
		uScanEnd = uEnd + (par.iMittelungX - 1) * iDelta;
	}
	int uScanStart;
	if (trackDir == VON_RECHTS)
	{
		// Linienverfolgung von rechts
		if (uStart == (rcroi_xend - 1))
		{
			// Die Spalte, in der wir mit der Verfolgung angefangen haben, hat sich aus dem reduzierten ROI ergeben - also machen wir das mit dem reduzierten ROI jetzt rueckgaengig.
			uScanStart = uStart - par.iMittelungX * iDelta;  // Diese Aenderung des Startpunktes reflektiert die Tatsache, dass wir den Rand, welcher durch die Mittelung entsteht, mit Daten aufgefuellt haben. Das muss sich auch in dieser Variable wiederspiegeln, sonst wird der Bereich der gueltigen Daten nicht entsprechend angepasst.
		}
		else
		{
			// Die Spalte, in der wir mit der Verfolgung angefangen haben, war ein Ergebnis von searchLines(). In diesem Fall muss nichts rueckgaengig gemacht werden. Diese Vorgehensweise hat zur Folge, dass die Phantasiewerte, mit denen wir den Rand der Mittelung auffuellen wuerden, keine Gueltigkeit erhalten - und das ist das, was wir in dieser Situation wollen.
			uScanStart = uStart;
		}
	}
	else 
	{
		if (uStart == rcroi_xstart)
		{
			// Die Spalte, in der wir mit der Verfolgung angefangen haben, hat sich aus dem reduzierten ROI ergeben - also machen wir das mit dem reduzierten ROI jetzt rueckgaengig.
			uScanStart = uStart - par.iMittelungX * iDelta;  // Diese Aenderung des Startpunktes reflektiert die Tatsache, dass wir den Rand, welcher durch die Mittelung entsteht, mit Daten aufgefuellt haben. Das muss sich auch in dieser Variable wiederspiegeln, sonst wird der Bereich der gueltigen Daten nicht entsprechend angepasst.
		}
		else{
			// Die Spalte, in der wir mit der Verfolgung angefangen haben, war ein Ergebnis von searchLines(). In diesem Fall muss nichts rueckgaengig gemacht werden. Diese Vorgehensweise hat zur Folge, dass die Phantasiewerte, mit denen wir den Rand der Mittelung auffuellen wuerden, keine Gueltigkeit erhalten - und das ist das, was wir in dieser Situation wollen.
			uScanStart = uStart;
		}
	}
	if (uScanStart>uScanEnd)
	{
		unsigned int heli;
		heli=uScanStart; uScanStart=uScanEnd; uScanEnd=heli;
	}
	
	if(result.firstValidIndex<0 || uScanStart<result.firstValidIndex)
	{
		result.firstValidIndex=uScanStart;
	}


	if(result.lastValidIndex<0 || uScanEnd>result.lastValidIndex)
	{
		result.lastValidIndex=uScanEnd;
	}
	// 1D-Filter anwenden
//	median1D(result.Y, uScanStart, uScanEnd);
//	mittel1D(result.Y, uScanStart, uScanEnd, 5);

	m_iTrackStopXLeft  = uScanStart;        // Erweiterung EMAG S1
	m_iTrackStopXRight = uScanEnd;          // Erweiterung EMAG S1


	//std::cout<<"trackingtrackLines beendet "<<std::endl;

	return bTrackError;
}


//***********************************************************************************************


inline int absInt(int i) { return ( (int)((i>=0)?(i):(-i)) ); }

bool  LaserlineTracker1T::checkTrackPoint(
	int&	yTemp,
	int&	x,
	int*	laserLine,
	int		iDelta,
	int&	countGapPoints,
	int&	countGaps)
{

    const auto & par =m_par; // const reference as a compile check that the parameters are not being modified  (that would invalidate the sum intensity cache)

	//unsigned char dummy = internimage.img[x + yTemp * internimage.pitch];

	// Ist Grauwert zu niedrig ?
	if( internimage.img[x + yTemp * internimage.pitch] < par.iSuchSchwelle )
	{
		// Unterbruchlaenge ermitteln
		countGapPoints++;

		
		// Unterbruch zu lange !!!!!!!!!!
		if( countGapPoints > par.iMaxBreiteUnterbruch )
		{
			// Zaehler fuer Unterbruchlaenge zuruecksetzen
			countGapPoints = 0;

			//std::cout<<" x vor zuruecksetzen: "<<x<<std::endl;


			// gehe wieder zurueck um Unterbruchlaenge
			x -= (iDelta * par.iMaxBreiteUnterbruch);
			x -= iDelta;
			 // Unterbruch zu lange
			//std::cout<<" x nach zuruecksetzen: "<<x<<std::endl;


			m_iTrackError = 225;
			return false;
		}
		
	
		// naechsten Trackingpunkt in gesamter ROI Hoehe ermitteln
		/*
			geo2dRange rgRoiTmpY;
			rgRoiTmpY.start_= rcroi_ystart;
			rgRoiTmpY.end_= rcroi_yend;
			yTemp = findMaxInColumn (x, rgRoiTmpY);
		*/

			m_iTrackError = 228; // Wert war unter Schwelle
		
	} //Grauwert zu niedrig
	else
	{
		// Luecke war vorhanden, also GapZaehler erhoehen
		if(countGapPoints>0)
			countGaps++;// Unterbrueche zaehlen - wie oft ueber die komplette Linie unter Suchschwelle ?

		// zu viele Unterbrueche ?
		if(countGaps > par.iMaxAnzahlUnterbrueche)
		{
			// statusMsg(STATUS_MSG,"TRACKERROR: zu viele Unterbrueche");
			m_iTrackError = 226;
			return false;
		}

		// Zaehler fuer Unterbruchlaenge zuruecksetzen
		countGapPoints = 0;

	}//Punkt o.k.

	// Sprung zu gross ?
	//if ( static_cast<unsigned int>( absInt( (int)yTemp - laserLine[x - iDelta] ) ) > (unsigned int)(par.iMaxLinienspringY))
	if ( static_cast<unsigned int>( absInt( (int)yTemp - laserLine[x - iDelta] ) ) > (unsigned int)(par.iMaxLinienSprungY))
	{
	// statusMsg(STATUS_MSG,"TRACKERROR: Sprung zu gross");
		m_iTrackError = 227;
		return false;
	}

    if (m_par.lapJoin && m_iTrackError != 228)
    {
        if (m_lineStartY == LINETRACKER1_Y_INVALID)
        {
            m_lineStartY = yTemp;
        }

        int curDiff = yTemp - m_lineStartY; ///< Difference between the actual tracked y and a horizontal at the level of the first tracked y-value

        if (yTemp < 0)
        {
            curDiff = -curDiff;
        }

        if (curDiff < 0 && std::abs(curDiff) > m_par.upperMaxLineDiff)
        {
            m_iTrackError = 229;
            return false;
        }
        if (curDiff > 0 && curDiff > m_par.lowerMaxLineDiff)
        {
            m_iTrackError = 229;
            return false;
        }
    }

	return true;
}

//***********************************************************************************************




#define lltlocalmin(A,B)  (((A)<(B)) ? (A) : (B)  )
#define lltlocalmax(A,B)  (((A)>(B)) ? (A) : (B)  )

void LaserlineTracker1T::median1D(int* histo, int iStartX, int iEndeX)
{
	int a, b, c;
	int i;

	// 1. Element
	a = histo[iStartX];
	b = histo[iStartX+1];
	c = histo[iStartX+2];
	histo[iStartX] = lltlocalmin( lltlocalmin( lltlocalmax(a,b) , lltlocalmax(b,c) ) ,lltlocalmax(a,c) );

	for (i = iStartX + 1; i <= (iEndeX - 1); i++)
	{
		a = histo[i-1];
		b = histo[i];
		c = histo[i+1];
		histo[i] = lltlocalmin( lltlocalmin( lltlocalmax(a,b), lltlocalmax(b,c) ) ,lltlocalmax(a,c) );
	}

	// letztes Element
	a = histo[iEndeX-2];
	b = histo[iEndeX-1];
	c = histo[iEndeX];
	histo[iEndeX] = lltlocalmin( lltlocalmin( lltlocalmax(a,b), lltlocalmax(b,c) ) ,lltlocalmax(a,c) );
}



#undef lltlocalmin
#undef lltlocalmax



//***********************************************************************************************

// Mittelwertfilter
void LaserlineTracker1T::mittel1D(int *fkt, int iStartX, int iEndeX, int iFilterGroesse)
{
	const int MAX_IMAGE_PIXELS = 3000;
	int fktTemp[MAX_IMAGE_PIXELS];
	int sum, x;

	for(x = (iStartX + iFilterGroesse); x <= (iEndeX - iFilterGroesse); x++)
	{
		sum = 0;
		for(int i= -iFilterGroesse; i <= iFilterGroesse; i++)
		{
			sum += fkt[x+i];
		}
		fktTemp[x] = sum / (2 * iFilterGroesse + 1);
	}

	for(x = (iStartX + iFilterGroesse); x <= (iEndeX - iFilterGroesse); x++)
	{
		fkt[x] = fktTemp[x];
	}
}


//******************************************************************************************************************
// Sucht den Startpunkt der Linie mit 70% der Suchschwelle
//******************************************************************************************************************

int LaserlineTracker1T::searchLine()
{
    const auto & par =m_par; // const reference as a compile check that the parameters are not being modified  (that would invalidate the sum intensity cache)

	int ret;
	ret = searchLinesUpperLower(par.iSuchSchwelle);
	if(ret!=0) return ret;

	/*
	clamping mit starty: das koennen wir immer noch machen
	ret = isInFirstLastImageRange(image, starty, direction, imageInfo);
	if(ret!=0) return ret;
	*/

	return 0;
}


//*************************************************************************************************************

int LaserlineTracker1T::searchLinesUpperLower(int threshold)
{
    const auto & par =m_par; // const reference as a compile check that the parameters are not being modified  (that would invalidate the sum intensity cache)

	const int IMinBorder = 10;
	const int INT_SIDE=2; /* Number of seam sides left-right*/
	int    rows  = internimage.roidy;
	int    cols  = internimage.roidx;
	int    pitch = internimage.pitch; //needed only for old implentation
	int    side, posx;
	int    value;
	int    y;
	int    maxIntensity[INT_SIDE];
	int    min[INT_SIDE];
	bool   bSetLinePos;
	const int INT_NOT_VALID = -1000;

	if ( (internimage.img == nullptr) || (internimage.pitch < 0) )
	{	
		return -1;
	}

	int actualStartAreaX = par.startAreaX < cols  ? par.startAreaX : cols - 1;
	int actualStartAreaY = par.startAreaY < rows ? par.startAreaY : rows -1;
	if (actualStartAreaX <= 0 || actualStartAreaY <=0)
	{
		return -1;
	}

    
	int startPointSize = actualStartAreaX * actualStartAreaY;
	
	const int searchwidth = 50; // die suche nach einem startpunkt soll bis 50% in den ROI gehen. 
	
	//needed only for old implentation
	const unsigned char * startp=internimage.img+internimage.roix0+internimage.roiy0*internimage.pitch;

	//30% ROI Breite => neu 50!!
	int posdx = int((double(cols) / 100)*searchwidth);

	// da immer x*y Pixel zusammengefasst werden
	threshold *= startPointSize;

	// Check if lines are visible on the vertical line given by the position of the first line
	// on the left and right side. Both lines have to be visible that the image is valid!

    bool useIntegralImage = m_IntegralImageComputed;
    
    #ifdef COMPAREOLDIMPLEMENTATION
    useIntegralImage = false;
    #endif
    
	try
	{
		for (side = 0; side < INT_SIDE; side++) //todo unroll loop
		{
			int VZ = 0;
			// start innerhalb ROI festlegen
			if (side == INT_L)
			{
				posx = 0;
				VZ = +1;
			}
			else
			{
				posx = cols - 1;
				VZ = -1;
			}
			
			const int xSumStartOffset=internimage.roix0 +((side == INT_L) ?                     0 : - actualStartAreaX+1);
			const int xSumEndOffset  =internimage.roix0 +((side == INT_L)? actualStartAreaX :                             1);
			const int ySumStart = internimage.roiy0;
			const int ySumEnd = internimage.roiy0 + actualStartAreaY;            

			// Init
			vLinePos[side].x = 0;
			bSetLinePos = false;
            

			while (!bSetLinePos && (vLinePos[side].x != INT_NOT_VALID))
			{

				// bei posx fuer jede Zeile Pixel in der geforderten Breite zusammenfassen
				m_oBorderLine.clear();
				m_oBorderLine.reserve(actualStartAreaY);
                
                if (!useIntegralImage)
                {
                    for (int rowCounter = 0; rowCounter < rows; rowCounter++)
                    {
                        int sumInLine = 0;
                        for (int x = 0; x < actualStartAreaX; x++)
                        {
                            sumInLine += *(startp + (posx + VZ*x) + rowCounter * pitch);
                        }
                        m_oBorderLine.push_back(sumInLine);
#ifdef COMPAREOLDIMPLEMENTATION
                        assert(sumInLine == computeSum(xSumStartOffset + posx, xSumEndOffset + posx,
                                                        internimage.roiy0 +  rowCounter,  internimage.roiy0 + rowCounter+1));
#endif
                    }

                    // Search smooth -> use 3 Pixels together
                    //value = *(startp + posx + 3 * pitch) + *(startp + posx + 4 * pitch) + *(startp + posx + 5 * pitch);
                    value = 0;

                    //int startIndex = -par.startAreaY / 2;
                    //int endIndex = (par.startAreaY+1) / 2;

                    //for (int lineIndex = startIndex; lineIndex < endIndex; lineIndex++)
                    //{		

                    //}


                    for (int lineIndex = 0 ; lineIndex < actualStartAreaY; lineIndex++)
                    {
                        value += m_oBorderLine.at(lineIndex);
                    }
#ifdef COMPAREOLDIMPLEMENTATION
                    assert(value == computeSum(xSumStartOffset + posx, xSumEndOffset + posx,
                                                        internimage.roiy0,  internimage.roiy0 + actualStartAreaY));
#endif
                }
                else
                {

                    for (int rowCounter = 0; rowCounter < rows; rowCounter++)
                    {
                        m_oBorderLine.push_back(computeSum(xSumStartOffset + posx, xSumEndOffset + posx,
                                                    internimage.roix0 + rowCounter, internimage.roix0 +  rowCounter+1));
                    }
                    value = computeSum(xSumStartOffset + posx, xSumEndOffset + posx,
                                                       ySumStart, ySumEnd);

                }

				maxIntensity[side] = 0;  /* To search position of max. intensity */
				min[side] = startPointSize * 260;

				int ySum = 0;
				int yCount = 0;

				// Test vertical values thru 'posx' with the highest GreyValue
				for (y = actualStartAreaY / 2 + 1; y < (rows - actualStartAreaY / 2 - 2); y++)
				{
					int minusIndex = y - actualStartAreaY / 2 - 1;
					value -= m_oBorderLine.at(minusIndex);
					int plusIndex = y + (actualStartAreaY + 1) / 2 - 1;
					value += m_oBorderLine.at(plusIndex);

					if (value < min[side])
					{
						min[side] = value;
					}
					if (value > threshold)
					{
						if (value > maxIntensity[side]) // jetzt neuer Zwischenschritt, erspart die Koordinatenberechnung weiter unten...
						{ // ein neues Max ist gefunden => Container fuer summierte Y's nullen
							if ((y > IMinBorder) && (y < internimage.roidy - IMinBorder))
							{
								ySum = 0;
								yCount = 0;
							}
						}

						if (value >= maxIntensity[side])
						{
							if ((y > IMinBorder) && (y < internimage.roidy - IMinBorder))
							{
								// --> Punkt gefunden!!!
								vLinePos[side].x = posx;
								vLinePos[side].y = y;
								ySum += y;
								yCount++;
								maxIntensity[side] = value;

								bSetLinePos = true;
							} // y > ...

						} // value> maxIntensity
					}  // value > threshold
				} // for y

				if (yCount > 0) // es wurden Y's summiert, also den Mittelwert nehmen => y-Berechnung von unten entfaellt! => auskommentiert
				{
					vLinePos[side].y = ySum / yCount;
				}

				// keine Linie gefunden???
				if (!bSetLinePos)
				{
					if (side == INT_L)
					{
						posx++;
						if (posx >= posdx)
							vLinePos[side].x = vLinePos[side].y = INT_NOT_VALID; // NO LINE FOR SIDE
					}
					else
					{
						posx--;
						if (posx <= ( cols - posdx) )
							vLinePos[side].x = vLinePos[side].y = INT_NOT_VALID; // NO LINE FOR SIDE
					}

					//vLinePos[side].x = vLinePos[side].y = INT_NOT_VALID; // NO LINE FOR SIDE

				} // if !bSet

				//else
				//{   // LaserLinie gefunden, nochmalige Suche nach der hoechsten Intensitaet in der Spalte posx
				//	// hier ist die y-Berechnung, die durch Mittelwertbildung gemacht wird => entfaellt! (wird bereits oben mitgemacht, spart viel Zeit)

				//	unsigned long int oNumerator = 0;
				//	unsigned long int oDenominator   = 0;
				//	bool bCalcSize = false;

				//	value = *(startp + posx + 3 * pitch) + *(startp + posx + 4 * pitch) + *(startp + posx + 5 * pitch);

				//	// Get the position with the biggest continous max value
				//for (y = 5 ; y < (rows - 5) ; y++)
				//{
				//	value -= *(startp + posx + (y - 2) * pitch );
				//	value += *(startp + posx + (y + 1) * pitch );

				//	if(value >= maxIntensity[side])  // diese if-Abfrage darf nicht selber von einer if-Abfrage bezueglich bCalcSize gekapselt sein, weil wir alle relevanten Pixel mitzaehlen muessen!
				//	{
				//		oNumerator = oNumerator + y;
				//		oDenominator++;
				//		bCalcSize = true;  // Berechnung ausloesen
				//	}
				//	if(value < maxIntensity[side])
				//	{
				//		// nur falls die Berechnung ausgeloest wurde, dann soll sie jetzt zu Ende gebracht werden:
				//		if (bCalcSize)
				//		{
				//			vLinePos[side].y = oNumerator / oDenominator;
				//			bCalcSize = false;
				//			oNumerator = 0;
				//			oDenominator = 0;   //  neu initialisieren.
				//		}
				//	}
				//}// end of for y
				//}// end of else if !bset


			} // while
		} // for side

		/*
		//Beide gueltig , rechts vor Links gefunden
		if (vLinePos[INT_L].x > vLinePos[INT_R].x && (vLinePos[INT_L].x != INT_NOT_VALID) && (vLinePos[INT_R].x != INT_NOT_VALID))
		{
		vLinePos[INT_L].x = INT_NOT_VALID;
		vLinePos[INT_R].x = INT_NOT_VALID;
		}

		// Es muessen beide Seiten gefunden werden!!!!!!
		if (   (vLinePos[INT_L].x == INT_NOT_VALID)
		|| (vLinePos[INT_R].x == INT_NOT_VALID) )
		{
		vLinePos[INT_L].x = vLinePos[INT_R].x = INT_NOT_VALID;
		vLinePos[INT_L].iGrey = 0;
		vLinePos[INT_R].iGrey = 0;
		return -1;
		}
		*/


		//************************
		if (par.doubleTracking)
		{
			// double tracking: need both sides to track
			if ((vLinePos[INT_L].x == INT_NOT_VALID)
				|| (vLinePos[INT_R].x == INT_NOT_VALID))
			{
				vLinePos[INT_L].x = vLinePos[INT_R].x = INT_NOT_VALID; vLinePos[INT_L].iGrey = 0; vLinePos[INT_R].iGrey = 0;
				return -1;
			}
			if (vLinePos[INT_L].x > vLinePos[INT_R].x)
			{
				vLinePos[INT_L].x = vLinePos[INT_R].x = INT_NOT_VALID; vLinePos[INT_L].iGrey = 0; vLinePos[INT_R].iGrey = 0;
				return -1;
			}
		}
		else
		{
			//No double tracking

			switch (par.iTrackStart)
			{
			case 0:
				// tracking von links
				if (vLinePos[INT_L].x == INT_NOT_VALID)
				{
					vLinePos[INT_L].x = vLinePos[INT_R].x = INT_NOT_VALID; vLinePos[INT_L].iGrey = 0; vLinePos[INT_R].iGrey = 0;
					return -1;
				}
				break;

			case 1:
				// tracking von rechts
				if (vLinePos[INT_R].x == INT_NOT_VALID)
				{
					vLinePos[INT_L].x = vLinePos[INT_R].x = INT_NOT_VALID; vLinePos[INT_L].iGrey = 0; vLinePos[INT_R].iGrey = 0;
					return -1;
				}
				break;

			case 2:
				// tracking von links oder rechts je nach Helligkeit
				if ((vLinePos[INT_L].x == INT_NOT_VALID)
					|| (vLinePos[INT_R].x == INT_NOT_VALID))
				{
					vLinePos[INT_L].x = vLinePos[INT_R].x = INT_NOT_VALID; vLinePos[INT_L].iGrey = 0; vLinePos[INT_R].iGrey = 0;
					return -1;
				}
				if (vLinePos[INT_L].x > vLinePos[INT_R].x)
				{
					vLinePos[INT_L].x = vLinePos[INT_R].x = INT_NOT_VALID; vLinePos[INT_L].iGrey = 0; vLinePos[INT_R].iGrey = 0;
					return -1;
				}
				break;

			default:
				vLinePos[INT_L].x = vLinePos[INT_R].x = INT_NOT_VALID; vLinePos[INT_L].iGrey = 0; vLinePos[INT_R].iGrey = 0;
				return -1;
			}
		}
	

//************************

		// Intensitaet setzen, Vorsicht: wegen x*y Pixel x*y-fache Intensitaet!!!
		vLinePos[INT_L].iGrey = maxIntensity[INT_L] / startPointSize;
		vLinePos[INT_R].iGrey = maxIntensity[INT_R] / startPointSize;

		/*
		if (diag_g & CALC_DBG_LINE_POS)
		{
		showCalcLnUpperLower(startp, linePos);
		}
		*/
		return 0;
	}

	catch (...)
	{
		assert(false && "The known OutOfRangeException in searchLinesUpperLower should be not occur with actualStartAreaY");
		vLinePos[INT_L].x = vLinePos[INT_R].x = INT_NOT_VALID; vLinePos[INT_L].iGrey = 0; vLinePos[INT_R].iGrey = 0;
		return -1;

	}

} // searchLinesUpperLower



