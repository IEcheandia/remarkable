/**
*  @copyright:		Precitec Vision GmbH & Co. KG
*  @author			Simon Hilsenbeck (HS)
*  @date			2013
*  @file			
*  @brief			Performs morphology operations on a binary image. There are speed-optimized versions, which work on bit-packed image structures.
*  @details			Border treatment: Borders are assumed to be zero. First and last out image line is always filled with zeros.
*					NB:	Byte-image algorithms are optimized for 'black'-dominated images. Automatic detection of dominating color can be added, algorithms exist.
*					NB:	Optimized and non-optimized alogorithms should provide exactly same results (checked with memcmp).
*/

// local includes
#include "filter/morphologyImpl.h"

// std lib includes
#include <limits>
#include <cstring>

namespace precitec {
	using namespace image;
	namespace filter {


/************************************************************************
* Description:  Fuehrt eine Dilataion mit einem 3x3 SE durch             *
*               White: optimiert auf ueberwiegend weisse Pixel im Bild  *
*               255 --> 255                                             *
*               0   --> 0, nur wenn das SE vollstaendig ins Objekt passt*
*                                                                       *
* Parameter:    p_rSource:          Eingangsbild                           *
*               p_rDestin:          Ergebnisbild                           *
************************************************************************/

void dilatationWhite(const BImage& p_rSource, BImage& p_rDestin)
{
	byte const *sLine0, *sLine1, *sLine2;
	byte *dLine;

	unsigned int sum;

	// 1.) Ergebnisbild mit 0 initialiserien
	p_rDestin.fill(0);

	for (int oRow=1; oRow < p_rSource.size().height-1; oRow++)
	{
		sLine0 = p_rSource[oRow-1];
		sLine1 = p_rSource[oRow];
		sLine2 = p_rSource[oRow+1];
		dLine  = p_rDestin[oRow];

		for (int oCol=1; oCol < p_rSource.size().width-1; oCol++)
		{
			// 255 --> 255
			if ( sLine1[oCol] == 255 )
			{
				dLine[oCol] = 255;
			}
			else
				// 0 --> 0, nur wenn das SE vollstaendig ins Objekt passt
			{
				sum = sLine0[oCol-1] + sLine0[oCol  ] + sLine0[oCol+1] +
					sLine1[oCol-1] /*+sLine1[oCol]*/+ sLine1[oCol+1] +
					sLine2[oCol-1] + sLine2[oCol  ] + sLine2[oCol+1];
				dLine[oCol] = ( sum > 0 )       ? 255 : 0;
			}
		} // for oCol
	} // for oRow
}

/************************************************************************
* Description:  Fuehrt eine Dilataion mit einem 3x3 SE durch             *
*               Black: optimiert auf ueberwiegend scharze Pixel im Bild *
*               1.) Ergebnisbild mit 0 initialiserien                   *
*               2.) Pixel mit 255 wird durch das SE ersetzt             *
*                                                                       *
* Parameter:    p_rSource:          Eingangsbild                           *
*               p_rDestin:          Ergebnisbild                           *
************************************************************************/

void dilatationBlack(const BImage& p_rSource, BImage& p_rDestin) {
	poco_assert_dbg(p_rSource.size() == p_rDestin.size());

	const int		oWidth		( p_rSource.size().width );
	const int		oHeight		( p_rSource.size().height );

	byte const*		sLine		( nullptr );
	byte*			dLine0		( nullptr );
	byte*			dLine1		( nullptr );
	byte*			dLine2		( nullptr );

	// 1.) Ergebnisbild mit 0 initialiserien
	p_rDestin.fill(0);

	sLine = p_rSource[0];
	dLine1 = p_rDestin[0+1];

	// special treatment for first pixel

	if (sLine[0]) {
		dLine1[0] = dLine1[1] = 255;
	} // if

	// special treatment for first line

	for (int oCol=1; oCol < oWidth-1; oCol++) {
		if (sLine[oCol] == 255) {
			dLine1[oCol-1] = dLine1[oCol] = dLine1[oCol+1] = 255;
		} // if
	} // for oCol


	for (int oRow=1; oRow < oHeight-1; oRow++) {
		sLine = p_rSource[oRow];
		dLine0 = p_rDestin[oRow-1];
		dLine1 = p_rDestin[oRow  ];
		dLine2 = p_rDestin[oRow+1];

		// special treatment for first column
		if ( sLine[0] == 255 ) {
				dLine0[1] = dLine1[1] = dLine2[1] =  255;
				dLine0[0] = dLine1[0] = dLine2[0] =  255; // set also border, to conform with dilate32
		} // if

		// main treatment
		for (int oCol=1; oCol < oWidth-1; oCol++) {
			// 2.) Pixel mit 255 wird durch das SE ersetzt
			if ( sLine[oCol] == 255 ) {
				dLine0[oCol-1] =  dLine0[oCol] =  dLine0[oCol+1] = 255;
				dLine1[oCol-1] =  dLine1[oCol] =  dLine1[oCol+1] = 255;
				dLine2[oCol-1] =  dLine2[oCol] =  dLine2[oCol+1] = 255;
			} // if
		} // for oCol

		// special treatment for last column
		if ( sLine[oWidth-1] == 255 ) {
			dLine0[oWidth-2] = dLine1[oWidth-2] = dLine2[oWidth-2] = 255;
			dLine0[oWidth-1] = dLine1[oWidth-1] = dLine2[oWidth-1] = 255; // set also border again, to conform with dilate32
		} // if

	} // for oRow

	// special treatment for last line

	sLine = p_rSource[oHeight-1];
	dLine1 = p_rDestin[oHeight-2];
	for (int oCol=1; oCol < oWidth-1; oCol++) {
		if ( sLine[oCol] == 255 ) {
			dLine1[oCol-1] =  dLine1[oCol] =  dLine1[oCol+1] = 255;
		} // if
	} // for oCol

	// special treatment for last pixel

	if (*(p_rSource.end() - 1)) {
		dLine1[oWidth-1] = dLine1[oWidth-2] = 255;
	} // if

	// as in erosionBlack, do not set first and last line, so fill again with zeros

	memset(p_rDestin.upperLeft(), 0, p_rDestin.size().width); 
	memset(p_rDestin.lowerLeft(), 0, p_rDestin.size().width); 
} // dilatationBlack

/************************************************************************
* Description:  Fuehrt eine Erosion mit einem 3x3 SE durch               *
*               Black: optimiert auf ueberwiegend schwarze Pixel im Bild*
*               0 --> 0                                                 *
*               255 --> 255, nur wenn das SE vollstaendig ins Objekt passt*
*                                                                       *
* Parameter:    p_rSource:          Eingangsbild                           *
*               p_rDestin:          Ergebnisbild                           *
************************************************************************/

void erosionBlack(const BImage& p_rSource, BImage& p_rDestin)
{
	byte const *sLine0, *sLine1, *sLine2;
	byte *dLine;

	unsigned int sum;

	// 1.) Ergebnisbild mit 0 initialiserien
	p_rDestin.fill(0);

	for (int oRow=1; oRow < p_rSource.size().height-1; oRow++)
	{
		sLine0 = p_rSource[oRow-1];
		sLine1 = p_rSource[oRow];
		sLine2 = p_rSource[oRow+1];
		dLine  = p_rDestin[oRow];

		for (int oCol=1; oCol < p_rSource.size().width-1; oCol++)
		{
			if ( sLine1[oCol] == 0 )
			{
				dLine[oCol] = 0;
			}
			else
			{
				sum = sLine0[oCol-1] + sLine0[oCol  ] + sLine0[oCol+1] +
					sLine1[oCol-1] /*+sLine1[oCol]*/+ sLine1[oCol+1] +
					sLine2[oCol-1] + sLine2[oCol  ] + sLine2[oCol+1];
				dLine[oCol] = ( sum < (/*9*/8*255) ) ? 0 : 255;
			}
		} // for oCol
	} // for oRow
}

/************************************************************************
* Description:  Fuehrt eine Erosion mit einem 3x3 SE durch               *
*               White: optimiert auf ueberwiegend weisse Pixel im Bild  *
*               1.) Ergebnisbild mit 255 initialiserien                 *
*               2.) Pixel mit 0 wird durch das SE ersetzt               *
*                                                                       *
* Parameter:    p_rSource:          Eingangsbild                           *
*               p_rDestin:          Ergebnisbild                           *
************************************************************************/

void erosionWhite(const BImage& p_rSource, BImage& p_rDestin)
{
	byte const *sLine;
	byte *dLine0, *dLine1, *dLine2;


	//  1.) Ergebnisbild mit 255 initialiserien
	p_rDestin.fill(255);

	for (int oRow=1; oRow < p_rSource.size().height-1; oRow++)
	{
		dLine0 = p_rDestin[oRow-1];
		dLine1 = p_rDestin[oRow  ];
		dLine2 = p_rDestin[oRow+1];
		sLine = p_rSource[oRow];

		for (int oCol=1; oCol < p_rSource.size().width-1; oCol++)
		{
			// 2.) Pixel mit 0 wird durch das SE ersetzt
			if ( sLine[oCol] == 0 )
			{
				dLine0[oCol-1] =  dLine0[oCol] =  dLine0[oCol+1] =
					dLine1[oCol-1] =  dLine1[oCol] =  dLine1[oCol+1] =
					//dLine2[oCol-1] =  dLine1[oCol] =  dLine1[oCol+1] =
					dLine2[oCol-1] =  dLine2[oCol] =  dLine2[oCol+1] =
					0;
			}
		} // for oCol
	} // for oRow
}


/************************************************************************
* Description:  Fuehrt "Iteration" morphologische Operationen (Erosion / *
*               Dilatation) durch                                       *
*                                                                       *
* Parameter:    p_rSource:          Eingangsbild                           *
*               p_rDestin:          Ergebnisbild                           *
*               pMorphFun:       "Funktionszeiger", gibt an, welche     *
*                                morphologische Operation ausgefuehrt   *
*                                wird                                   *
*               p_oNbIterations:      Anzahl der Iterationen                 *
************************************************************************/

void morph(const BImage& p_rSource, BImage& p_rDestin, PMorphFun pMorphFun, byte p_oNbIterations)
{

	//p_rDestin.roi()  = p_rSource.roi();
	switch (p_oNbIterations)
	{
	case 0:
		{
			p_rDestin = p_rSource;
			break;
		}
	case 1:
		{
			pMorphFun(p_rSource, p_rDestin);
			break;
		}
	case 2:
		{
			BImage morph0(p_rSource.size());

			pMorphFun(p_rSource, morph0);
			pMorphFun(morph0, p_rDestin);

			break;
		}
	default:
		{
			BImage morph0(p_rSource.size());
			BImage morph1(p_rSource.size());

			pMorphFun(p_rSource, morph0);
			for (int i = 1; i < p_oNbIterations - 2; i++)
			{
				pMorphFun(morph0, morph1);
				i++;
				pMorphFun(morph1, morph0);
			}
			if (p_oNbIterations&1) /*ungerade*/
			{
				pMorphFun(morph0, morph1);
				pMorphFun(morph1, p_rDestin);
			}
			else
			{
				pMorphFun(morph0, p_rDestin);
			}
		} // default
	} // switch
}

/************************************************************************
* Description:  fuehrt eine Oeffnung durch                               *
*                                                                       *
* Parameter:    p_rSource:     Eingangsbild                                *
*               p_rDestin:     Ergebnisbild                                *
*               p_oNbIterations: Anzahl Oeffnungen                           *
************************************************************************/

void opening(const BImage& p_rSource, BImage& p_rDestin, int p_oNbIterations)
{
	BImage oDestinTmp(p_rSource.size());

	//if ( isWhiteDominant(p_rSource) )
	//{
		//morph(p_rSource,  oDestinTmp, erosionWhite,    p_oNbIterations);
		//morph(oDestinTmp,  p_rDestin, dilatationWhite, p_oNbIterations);
	//} else
	// TODO
	{
		morph(p_rSource,  oDestinTmp, erosionBlack,    p_oNbIterations);
		morph(oDestinTmp,  p_rDestin, dilatationBlack, p_oNbIterations);
	}

}

/************************************************************************
* Description:  fuehrt eine Schliessung durch                            *
*                                                                       *
* Parameter:    p_rSource:     Eingangsbild                                *
*               p_rDestin:     Ergebnisbild                                *
*               p_oNbIterations: Anzahl Schliessungen                        *
************************************************************************/

void closing(const BImage& p_rSource, BImage& p_rDestin, int p_oNbIterations)
{
	BImage oDestinTmp(p_rSource.size());
	
	// TODO
	//if ( isWhiteDominant(p_rSource) )
	//{
	//	morph(p_rSource,  oDestinTmp, dilatationWhite, p_oNbIterations);
	//	morph(oDestinTmp,  p_rDestin, erosionWhite,    p_oNbIterations);
	//} else
	{
		morph(p_rSource,  oDestinTmp, dilatationBlack, p_oNbIterations);
		morph(oDestinTmp,  p_rDestin, erosionBlack,    p_oNbIterations);
	}
}

/************************************************************************
* Description:  "berechnet" dass Maximum von n Zahlen                   *
*                                                                       *
* Parameter:    p_pData:      Feld von n Zahlen                             *
*               p_oSize:   Anzahl n                                      *
*                                                                       *
* Returns:      Maximum der n Zahlen                                    *
************************************************************************/

byte getMax(const byte *const p_pData, byte p_oSize)
{
	byte max = 0;
	for (unsigned int iMax = 0; iMax < p_oSize; iMax++)
	{
		if (p_pData[iMax] > max)
			max = p_pData[iMax];
	} // for iMax
	return max;
}

/************************************************************************
* Description:  "berechnet" dass Minimum von n Zahlen                   *
*                                                                       *
* Parameter:    p_pData:      Feld von n Zahlen                             *
*               p_oSize:   Anzahl n                                      *
*                                                                       *
* Returns:      Minimum der n Zahlen                                    *
************************************************************************/

byte getMin(const byte *const p_pData, byte p_oSize)
{
	byte min = 255;
	for (unsigned int iMin = 0; iMin < p_oSize; iMin++)
	{
		if (p_pData[iMin] < min)    min = p_pData[iMin];
	} // for iMin
	return min;
}

/************************************************************************
* Description:  "berechnet" dass Maximum von n Binaer-Zahlen             *
*                                                                       *
* Parameter:    p_pData:      Feld von n Zahlen                             *
*               p_oSize:   Anzahl n                                      *
*                                                                       *
* Returns:      Maximum der n Binaer-Zahlen                              *
************************************************************************/

byte getMaxBinaer(const byte *const p_pData, byte p_oSize)
{
	for (unsigned int iMax = 0; iMax < p_oSize; iMax++)
	{
		if (p_pData[iMax] == 255)
			return 255;
	} // for iMax
	return 0;
}

/************************************************************************
* Description:  "berechnet" dass Minimum von n Binaer-Zahlen             *
*                                                                       *
* Parameter:    p_pData:      Feld von n Zahlen                             *
*               p_oSize:   Anzahl n                                      *
*                                                                       *
* Returns:      Minimum der n Binaer-Zahlen                              *
************************************************************************/

byte getMinBinaer(const byte *const p_pData, byte p_oSize)
{
	for (unsigned int iMin = 0; iMin < p_oSize; iMin++)
	{
		if (p_pData[iMin] == 0)
			return 0;
	} // for iMin
	return 255;
}

/************************************************************************
* Description:  fuehrt die Erosion / Dilataion durch                     *
*                                                                       *
* Parameter:    p_rSource:          Eingangsbild                           *
*               p_rDestin:          Ergebnisbild                           *
*               p_oMorphOperation:  Dilation / Erosion                     *
************************************************************************/

void dilationErosionS2(const BImage& p_rSource, BImage& p_rDestin, MorphOperation p_oMorphOperation)
{
	//  SE:
	//      1
	//    1 1 1
	//  1 1 1 1 1
	//    1 1 1
	//      1

	//p_rDestin.roi()  = p_rSource.roi();

	byte p_pData[13];
	byte const *sLine0, *sLine1, *sLine2, *sLine3, *sLine4;
	byte *dLine;


	for (int oRow=2; oRow<p_rSource.size().width-2; oRow++)
	{
		sLine0 = p_rSource[oRow-2];
		sLine1 = p_rSource[oRow-1];
		sLine2 = p_rSource[oRow];
		sLine3 = p_rSource[oRow+1];
		sLine4 = p_rSource[oRow+2];
		dLine  = p_rDestin[oRow];
		for (int oCol=2; oCol<p_rSource.size().height-2; oCol++)
		{
			p_pData[0] = sLine0[oCol];
			p_pData[1] = sLine1[oCol-1];
			p_pData[2] = sLine1[oCol  ];
			p_pData[3] = sLine1[oCol+1];
			p_pData[4] = sLine2[oCol-2];
			p_pData[5] = sLine2[oCol-1];
			p_pData[6] = sLine2[oCol  ];
			p_pData[7] = sLine2[oCol+1];
			p_pData[8] = sLine2[oCol+2];
			p_pData[9] = sLine3[oCol-1];
			p_pData[10]= sLine3[oCol  ];
			p_pData[11]= sLine3[oCol+1];
			p_pData[12]= sLine4[oCol];

			dLine[oCol] = (p_oMorphOperation == Dilation) ? getMax(p_pData, 13) : getMin(p_pData, 13);
		} // for oCol
	} // for oRow
	/*
	// folgendes gilt NUR fuer ein 3*3SE!!!
	p_rDestin.roi().x  += 2;
	p_rDestin.roi().y  += 2;
	p_rDestin.size().height -= 4;
	p_rDestin.size().width -= 4;
	*/
}

/************************************************************************
* Description:  fuehrt die Erosion / Dilataion durch                     *
*                                                                       *
* Parameter:    p_rSource:          Eingangsbild                           *
*               p_rDestin:          Ergebnisbild                           *
*               p_oMorphOperation:  Dilation / Erosion                     *
************************************************************************/

void dilationErosionS3(const BImage& p_rSource, BImage& p_rDestin, MorphOperation p_oMorphOperation)
{
	//  SE:
	//    1
	//  1 1 1
	//    1

	//p_rDestin.roi()  = p_rSource.roi();
	byte p_pData[5];
	byte const *sLine0, *sLine1, *sLine2;
	byte *dLine;


	for (int oRow=1; oRow<p_rSource.size().height-1; oRow++)
	{
		sLine0 = p_rSource[oRow-1];
		sLine1 = p_rSource[oRow];
		sLine2 = p_rSource[oRow+1];
		dLine  = p_rDestin[oRow];
		for (int oCol=1; oCol<p_rSource.size().width-1; oCol++)
		{
			p_pData[0] = sLine0[oCol  ];
			p_pData[1] = sLine1[oCol-1];
			p_pData[2] = sLine1[oCol  ];
			p_pData[3] = sLine1[oCol+1];
			p_pData[4] = sLine2[oCol  ];

			dLine[oCol] = (p_oMorphOperation == Dilation) ? getMax(p_pData, 5) : getMin(p_pData, 5);
		} // for oCol
	} // for oRow
	/*
	// folgendes gilt NUR fuer ein 3*3SE!!!
	p_rDestin.roi().x++;
	p_rDestin.roi().y++;
	p_rDestin.size().height -= 2;
	p_rDestin.size().width -= 2;
	*/
}



//------------------------------------------------------------
//------------------------------------------------------------
//------32 bit Varianten von erosion und dilation ------------
//------------------------------------------------------------

/**
* zeilenweise Erosion auf 'packed bit' Doppelwort
*
* \param uLeft  32 bits im linken    Block (nur rechtestes wird benutzt)
* \param uX     32 bits im zentralen Block alle Bits werden neu berechnet
* \param uRight 32 bits im rechten   Block (nur linkestes bit wird benutzt)
*
* \result erodierte Bits im zentralen Block
*/
inline unsigned int erode(unsigned int uLeft, unsigned int uX, unsigned int uRight)
{
	return uX & ((uX>>1) | (uLeft<<31)) & ((uX<<1) | (uRight>>31));
}

/**
 * 3*3 Erosion auf 32-Bit 'packed bit'-Daten
 *
 * \param p_rSrcPacked Eingangsbild
 * \param p_rErode Ausgangsbild
 *
 */
void erode32(const U32Image& p_rSrcPacked, U32Image& p_rErode)
{
	const int			oDx			( p_rSrcPacked.size().width );
	const int			oDy			( p_rSrcPacked.size().height );
	const unsigned int*	vBinN		( nullptr );
	const unsigned int*	vBinC		( nullptr );
	const unsigned int*	vBinS		( nullptr );
	unsigned int*		vErode		( nullptr );
	unsigned int		uSet		( 0 );

	for (int oRow=1; oRow < oDy-1; oRow++)
	{
		vBinN  = p_rSrcPacked[oRow+1]; // muss unbedingt U32Image definieren
		vBinC  = p_rSrcPacked[oRow  ]; // muss unbedingt U32Image definieren
		vBinS  = p_rSrcPacked[oRow-1]; // muss unbedingt U32Image definieren
		vErode = p_rErode[oRow]; // muss unbedingt U32Image definieren

		// Spezialbehandlung fuer erstes DWord
		uSet  = erode(0, vBinN[0], vBinN[1]);
		uSet &= erode(0, vBinC[0], vBinC[1]);
		uSet &= erode(0, vBinS[0], vBinS[1]);
		vErode[0] = uSet;

		for (int oCol=1; oCol < oDx-1; oCol++)
		{
			uSet  = erode(vBinN[oCol-1], vBinN[oCol], vBinN[oCol+1]);
			uSet &= erode(vBinC[oCol-1], vBinC[oCol], vBinC[oCol+1]);
			uSet &= erode(vBinS[oCol-1], vBinS[oCol], vBinS[oCol+1]);
			vErode[oCol] = uSet;
		}

		// Spezialbehandlung fuer letztes DWord
		uSet  = erode(vBinN[oDx-2 >= 0 ? oDx-2 : 0], vBinN[oDx-1], 0);
		uSet &= erode(vBinC[oDx-2 >= 0 ? oDx-2 : 0], vBinC[oDx-1], 0);
		uSet &= erode(vBinS[oDx-2 >= 0 ? oDx-2 : 0], vBinS[oDx-1], 0);
		vErode[oDx-1] = uSet;

	} // for (int oRow=1; oRow<oDy-1; oRow++)

	// Spezialbehandlung fuer die erste und letzte Zeile
	//std::memcpy(p_rErode[0],     p_rErode[1],     (oDx)*sizeof(unsigned int));
	//std::memcpy(p_rErode[oDy-1], p_rErode[oDy-2], (oDx)*sizeof(unsigned int));
	memset(p_rErode[oDy-1], 0, (oDx)*sizeof(unsigned int)); // erosionBlack also leaves the line filled with zeros
	memset(p_rErode[0],     0, (oDx)*sizeof(unsigned int)); // erosionBlack also leaves the line filled with zeros
} // erode32


/**
 * zeilenweise Dilation auf 'packed bit' Doppelwort
 *
 * \param uLeft  32 bits im linken    Block (nur rechtestes wird benutzt)
 * \param uX     32 bits im zentralen Block alle Bits werden neu berechnet
 * \param uRight 32 bits im rechten   Block (nur linkestes bit wird benutzt)
 *
 * \result erodierte Bits im zentralen Block
 */
inline unsigned int dilate(unsigned int uLeft, unsigned int uX, unsigned int uRight)
{
	return uX | (uX>>1) | (uLeft<<31) | (uX<<1) | (uRight>>31);
}

/**
 * 3*3 Dilation auf 32-Bit 'packed bit'-Daten
 *
 * \param p_rSrcPacked Eingangsbild
 * \param p_rErode Ausgangsbild
 *
 */
void dilate32(const U32Image& p_rSrcPacked, U32Image& p_rDilate)
{
	int  oDx = p_rSrcPacked.size().width;
	int  oDy = p_rSrcPacked.size().height;
	const unsigned int *vBinN;
	const unsigned int *vBinC;
	const unsigned int *vBinS;
	unsigned int *vDilate;
	unsigned int  uSet;

	for (int oRow=1; oRow < oDy-1; oRow++) {
		vBinN  = p_rSrcPacked[oRow+1]; // muss unbedingt U32Image definieren
		vBinC  = p_rSrcPacked[oRow  ]; // muss unbedingt U32Image definieren
		vBinS  = p_rSrcPacked[oRow-1]; // muss unbedingt U32Image definieren
		vDilate = p_rDilate[oRow]; // muss unbedingt U32Image definieren

		// Spezialbehandlung fuer erste DWord
		uSet  = dilate(0, vBinN[0], vBinN[1]);
		uSet |= dilate(0, vBinC[0], vBinC[1]);
		uSet |= dilate(0, vBinS[0], vBinS[1]);
		vDilate[0] = uSet;

		for (int oCol=1; oCol < oDx-1; oCol++)
		{
			uSet  = dilate(vBinN[oCol-1], vBinN[oCol], vBinN[oCol+1]);
			uSet |= dilate(vBinC[oCol-1], vBinC[oCol], vBinC[oCol+1]);
			uSet |= dilate(vBinS[oCol-1], vBinS[oCol], vBinS[oCol+1]);
			vDilate[oCol] = uSet;
		} // for

		// Spezialbehandlung fuer letztes DWord
		uSet  = dilate(vBinN[oDx-2 >= 0 ? oDx-2 : 0], vBinN[oDx-1], 0);
		//[] (int oSet) { for (int oBit = 31; oBit >= 0; --oBit) std::cout << ((oSet >> oBit) & 1); } (vBinC[oDx-2 >= 0 ? oDx-2 : 0]); std::cout << "\n"; // debug
		//[] (int oSet) { for (int oBit = 31; oBit >= 0; --oBit) std::cout << ((oSet >> oBit) & 1); } (vBinC[oDx-1]); std::cout << "\n"; // debug
		uSet |= dilate(vBinC[oDx-2 >= 0 ? oDx-2 : 0], vBinC[oDx-1], 0);
		uSet |= dilate(vBinS[oDx-2 >= 0 ? oDx-2 : 0], vBinS[oDx-1], 0);
		vDilate[oDx-1] = uSet;
		//std::cout << "[" << oDx-1 << " " << oRow << "] set.\n"; // debug
		//[] (int oSet) { for (int oBit = 31; oBit >= 0; --oBit) std::cout << ((oSet >> oBit) & 1); } (uSet); std::cout << "\n"; // debug

	} // for

	// Spezialbehandlung fuer die erste und letzte Zeile
	//std::memcpy(p_rDilate[0],     p_rDilate[1],     (oDx)*sizeof(unsigned int));
	//std::memcpy(p_rDilate[oDy-1], p_rDilate[oDy-2], (oDx)*sizeof(unsigned int));
	std::memset(p_rDilate[oDy-1], 0, (oDx)*sizeof(unsigned int)); // dilationBlack also leaves the line filled with zeros
	std::memset(p_rDilate[0],     0, (oDx)*sizeof(unsigned int)); // dilationBlack also leaves the line filled with zeros

} // dilate


/**
 * Oeffnung auf 32-Bit 'packed bit'-Daten mit 3*3 Kern
 *
 * \param p_rSrcPacked Eingangsbild
 * \param p_rDstPacked Ausgangsbild
 * \param p_oNbIterations Anzahl Oeffnungen 1<=p_oNbIterations<=BigValue
 *
 */
void opening32(const U32Image& p_rSrcPacked, U32Image& p_rDstPacked, unsigned int p_oNbIterations)
{
  switch (p_oNbIterations)
	{
	case 0:
		{
			p_rDstPacked = p_rSrcPacked;
			break;
		}
  case 1:
		{
			U32Image p_rErode(p_rSrcPacked.size());
			erode32(p_rSrcPacked, p_rErode);
			dilate32(p_rErode, p_rDstPacked);
			break;
		}
	case 2:
		{
			U32Image oTemp0(p_rSrcPacked.size());
			U32Image oTemp1(p_rSrcPacked.size());
			erode32(p_rSrcPacked, oTemp0);
			erode32(oTemp0, oTemp1);
			dilate32(oTemp1, oTemp0);
			dilate32(oTemp0, p_rDstPacked);
			break;
		}
	default:
		{
			U32Image oTemp0(p_rSrcPacked.size());
			U32Image oTemp1(p_rSrcPacked.size());

			erode32(p_rSrcPacked, oTemp0);
			for (unsigned int i = 1; i < p_oNbIterations - 2; i+=2)
			{
				erode32(oTemp0, oTemp1);
				erode32(oTemp1, oTemp0);
			}
			if (p_oNbIterations&1) /*ungerade*/
			{
				erode32(oTemp0, oTemp1);
				erode32(oTemp1, p_rDstPacked);
			}
			else
			{
				erode32(oTemp0, p_rDstPacked);
			}

			dilate32(p_rDstPacked, oTemp0);
			for (unsigned int i = 1; i < p_oNbIterations - 2; i+=2)
			{
				dilate32(oTemp0, oTemp1);
				dilate32(oTemp1, oTemp0);
			}
			if (p_oNbIterations&1) /*ungerade*/
			{
				dilate32(oTemp0, oTemp1);
				dilate32(oTemp1, p_rDstPacked);
			}
			else
			{
				dilate32(oTemp0, p_rDstPacked);
			}
			break;
		} // default
	} // switch
} // opening32

/**
 * Schliessung auf 32-Bit 'packed bit'-Daten mit 3*3 Kern
 *
 * \param p_rSrcPacked Eingangsbild
 * \param p_rDstPacked Ausgangsbild
 * \param p_oNbIterations Anzahl Schliessungen 1<=p_oNbIterations<=BigValue
 *
 */
void closing32(const U32Image& p_rSrcPacked, U32Image& p_rDstPacked, unsigned int p_oNbIterations)
{
  switch (p_oNbIterations)
	{
	case 0:
		{
			p_rDstPacked = p_rSrcPacked;
			break;
		}
  case 1:
		{
			U32Image p_rErode(p_rSrcPacked.size());
			dilate32(p_rSrcPacked, p_rErode);
			erode32(p_rErode, p_rDstPacked);
			break;
		}
	case 2:
		{
			U32Image oTemp0(p_rSrcPacked.size());
			U32Image oTemp1(p_rSrcPacked.size());
			dilate32(p_rSrcPacked, oTemp0);
			dilate32(oTemp0, oTemp1);
			erode32(oTemp1, oTemp0);
			erode32(oTemp0, p_rDstPacked);
			break;
		}
	default:
		{
			U32Image oTemp0(p_rSrcPacked.size());
			U32Image oTemp1(p_rSrcPacked.size());

			dilate32(p_rSrcPacked, oTemp0);
			for (unsigned int i = 1; i < p_oNbIterations - 2; i+=2)
			{
				dilate32(oTemp0, oTemp1);
				dilate32(oTemp1, oTemp0);
			}
			if (p_oNbIterations&1) /*ungerade*/
			{
				dilate32(oTemp0, oTemp1);
				dilate32(oTemp1, p_rDstPacked);
			}
			else
			{
				dilate32(oTemp0, p_rDstPacked);
			}

			erode32(p_rDstPacked, oTemp0);
			for (unsigned int i = 1; i < p_oNbIterations - 2; i+=2)
			{
				erode32(oTemp0, oTemp1);
				erode32(oTemp1, oTemp0);
			}
			if (p_oNbIterations&1) /*ungerade*/
			{
				erode32(oTemp0, oTemp1);
				erode32(oTemp1, p_rDstPacked);
			}
			else
			{
				erode32(oTemp0, p_rDstPacked);
			}
			break;
		} // default
	} // switch
} // closing32


/**
 * Konversion von 1Pixel/byte Binaerdarstellung auf 32-Bit 'packed bit'-Daten
 *
 * \param p_rSrc Eingangsbild
 * \param p_rDstPacked Ausgangsbild
 *
 */
void bin2ToBin32(const image::BImage& p_rSrc, U32Image& p_rDstPacked)
{
	const unsigned int		oNbBitsPerByte		( std::numeric_limits<byte>::digits );
	const unsigned int		oNbBytesPerDWord	( sizeof(unsigned int) );
	const unsigned int		oNbBitsPerDWord		( oNbBytesPerDWord * oNbBitsPerByte );
	const unsigned int		oDxByte				( p_rSrc.size().width );
	const unsigned int		oDyByte				( p_rSrc.size().height );

	unsigned int			*pDWord				( p_rDstPacked.begin() );
	unsigned int			oDWordPos			( 0 );
	unsigned int			oNewWord			( 0 );

	for (unsigned int oRow=0; oRow < oDyByte; ++oRow) {
		const byte *const	pRow		( p_rSrc[oRow] ); // get line pointer
		int					oBitPos		( oNbBitsPerDWord - 1 );
		for (unsigned int oCol = 0; oCol < oDxByte; ++oCol) {
			oNewWord |=  (pRow[oCol] & 1) << oBitPos;
			--oBitPos;
			if (oCol && ((oCol + 1) % oNbBitsPerDWord == 0 || oCol == oDxByte - 1)) {
				pDWord[oDWordPos]	= oNewWord;

				//if (oRow % 2 == 0) {
				//	if (oDWordPos % 2 == 0)
				//		std::cout << "row <" << oRow << ": ";
				//	for (int oBitPos = oNbBitsPerDWord - 1; oBitPos >=  0; --oBitPos) {
				//		if (oBitPos % 1 == 0)
				//			std::cout << unsigned int((oNewWord >> oBitPos) & 1)  << ' ';
				//	}
				//	if ((oDWordPos + 1) % 2 == 0)
				//		std::cout << ">\p_oNbIterations";
				//}

				oBitPos				= oNbBitsPerDWord - 1;
				oNewWord			= 0;

				++oDWordPos;
			} // if
		} // for
	} // for
} // bin32ToBin2


/**
 * Konversion von 32-Bit 'packed bit'-Daten auf 1Pixel/byte Binaerdarstellung
 *
 * \param p_rSrcPacked Eingangsbild
 * \param p_rDst Ausgangsbild
 *
 */
void bin32ToBin2(const U32Image& p_rSrcPacked, BImage& p_rDst)
{
	//p_rDst.fill(0);

	const unsigned int	oNbBitsPerByte		( std::numeric_limits<byte>::digits );
	const unsigned int	oNbBitsPerDWord		( sizeof(unsigned int) * oNbBitsPerByte );
	const unsigned int	oDxDWord			( p_rSrcPacked.size().width );
	const unsigned int	oDyDWord			( p_rSrcPacked.size().height );
	const unsigned int	oDxByte				( p_rDst.size().width );

	poco_assert_dbg(oDyDWord == p_rDst.size().height);
	poco_assert_dbg(oDxByte >= oNbBitsPerDWord); // image width must be at least dword length

	unsigned int		oColB				( 0 );
	unsigned int		oDWord				( 0 );

	for (unsigned int oRow=0; oRow < oDyDWord; oRow++) {
		const unsigned int* pDWord	( p_rSrcPacked[oRow] );
		byte*				pByte	( p_rDst[oRow] );
		unsigned int		oCol	( 0 );
		for (oColB=0; oCol < oDxDWord; ++oCol) {
			oDWord	= pDWord[oCol];
			// shift the 32 bits into place
			for (int oBitPos = oNbBitsPerDWord - 1; oBitPos >= 0; oBitPos--) {
				pByte[oColB++] = -byte((oDWord >> oBitPos) & 1); // -byte(0/1) -> 0/255
				if (oColB == oDxByte) { // stop if the byte image col end is reached
					break;
				} // if
			}
			// the last bit neednt be shifted
			// (actually this case was put extra to make the aloop condition easy)
			//pByte[oColB++] = -byte(oDWord & 1);
		} // for (int oCol=0, oColB=0; oCol<oDxDWord; oCol++)

		//oDWord = pDWord[oCol];

		//// shift the last 32 bits into place
		//int uPos = oDyByte - oNbBitsPerDWord * oDxDWord;
		//for (int oBit=31; oBit > oNbBitsPerDWord - uPos; oBit--)
		//{
		//	pByte[oColB++] = -byte((oDWord>>oBit) & 1); // -byte(0/1) -> 0/255
		//}
		//// the last bit needn#t be shifted
		//// (actually this case was put extra to make the loop condition easy)
		//if (uPos > 0) {
		//	pByte[oColB++] = -byte(oDWord & 1); // -byte(0/1) -> 0/255
		//}

		//// Spezialbehandlung fuer die erste und letzte Spalte
		//pByte[0] = pByte[oDxByte-1] = 0;

	} // for (int oRow=0; oRow<oDyDWord; oRow++)

	// Spezialbehandlung fuer die erste und letzte Zeile
	//memset(p_rDst[0],     0, (oDxDWordByte)*sizeof(byte));
	//memset(p_rDst[oDyDWord-1], 0, (oDxDWordByte)*sizeof(byte));
} // bin32ToBin2


	} // namespace filter
} // namespace precitec




