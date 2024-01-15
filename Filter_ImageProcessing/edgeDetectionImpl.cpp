/**
 *  @copyright:		Precitec Vision GmbH & Co. KG
 *  @author			JS
 *  @date			2014
 *  @file
 *  @brief			Performs edge detection operations sobel and kirsch
 */

#if defined __QNX__ || defined __linux__
#else
#pragma warning( disable : 4996 )   // see http://stackoverflow.com/questions/3317536/visual-studio-warning-c4996
#endif // #if defined __QNX__ || defined __linux__
// local includes
#include "edgeDetectionImpl.h"

#include "image/image.h"				///< BImage


// system includes
#include <new>
#include <cmath>
// std lib includes
#include <limits>

namespace precitec {
	using namespace image;
	namespace filter {



/**
* @brief					performs roberts edge detection
* @param p_rSource	        input image
* @param p_rDestin		    result image
* @param pMode              decides filtering of the detected edges
*/
int roberts(const BImage& p_rSource, BImage& p_rDestin, int pMode)
{

	int height = p_rSource.size().height;
	int width = p_rSource.size().width;
	//int pitch = width;
	int i = 0;
	int j = 0;

	for ( i = 0; i<height - 1; i++)        //y Rand 1
	{
		for ( j = 0; j<width - 1; j++)     //x Rand 1
		{
			p_rDestin[i][j] = static_cast<byte> (  std::abs((p_rSource[i][j] - p_rSource[i][j + 1])
								                    + (p_rSource[i][j] - p_rSource[i+1][j] )  ));

		}
		p_rDestin[i][width - 1] = 0;

	}

	std::memset(p_rDestin[height - 1], 0, width);

	return(0);



}//roberts





//Gx
//1 0 -1
//2 0 -2
//1 0 -1

//Gy
// 1  2  1
// 0  0  0
//-1 -2 -1

/**
* @brief					performs roberts edge detection with the filter masks above
* @param p_rSource	        input image
* @param p_rDestin		    result image
* @param pMode              decides filtering of the detected edges
*/
int sobel(const BImage& p_rSource, BImage& p_rDestin,int pMode)
{

	//byte const *sLine;
	//byte *dLine0, *dLine1, *dLine2;

	int height = p_rSource.size().height;
	int width  = p_rSource.size().width;
	int *pIntImage = NULL;
	int *pRemember = NULL;
	int max = -10000;
	int min = 10000;



	if (pMode > 0)
	{
		pIntImage = new int[height*width];
		pRemember = pIntImage;
	}

	//  Irgendwas initialisieren ?
	//	p_rDestin.fill(255);

	// Zeilen durchlaufen
	for (int y = 1; y < height - 1; y++)
	{
		//Zeilenzugriffe Destination
		const byte	*pLineInPre = p_rSource[y - 1];
		const byte	*pLineInCur = p_rSource[y];
		const byte	*pLineInPost = p_rSource[y + 1];

		byte		*pLineOut = p_rDestin[y];


		pLineOut[0] = 0; // fill boundary

		//Spalten durchlaufen
		for (int x = 1; x < width - 1; x++)
		{
			// Faltung mit Gx
			const int	oRawValueX = std::abs(
				pLineInPre[x - 1] - pLineInPre[x + 1]
				+ 2 * pLineInCur[x - 1] - 2 * pLineInCur[x + 1]
				+ pLineInPost[x - 1] - pLineInPost[x + 1]);

			// Faltung mit Gy
			const int	oRawValueY = std::abs(
				pLineInPre[x - 1] + 2 * pLineInPre[x] + pLineInPre[x + 1]
				- pLineInPost[x - 1] - 2 * pLineInPost[x] - pLineInPost[x + 1]);

			//int oRawValueG = static_cast<byte>(  sqrt(oRawValueX*oRawValueX + oRawValueY*oRawValueY));

			int oRawValueG = (  std::abs(oRawValueX) + std::abs(oRawValueY) );

			if (pMode > 0)
			{
				// min max zum Normieren:
				if (oRawValueG > max)
					max = oRawValueG;
				if (oRawValueG < min)
					min = oRawValueG;

				//zwischenspeichern
				*pIntImage++ = oRawValueG;


			}
			else
				pLineOut[x] = static_cast<byte>(std::min(oRawValueG, 255));	// prevent clipping
		} // for
		pLineOut[width - 1] = 0; // fill boundary
	} // for

	//Normieren
	if (pMode > 0)
	{
		pIntImage = pRemember;
		double diff = static_cast<double>(max - min);
		for (int y = 1; y < height - 1; y++)
		{
			byte	*pLineOut = p_rDestin[y];
			for (int x = 1; x < width - 1; x++)
			{
				pLineOut[x] = static_cast<byte>((255.0 / diff) *  static_cast<double> (*pRemember++ - min));
			}
		}
	}

	std::memset(p_rDestin[0], 0, width);
	std::memset(p_rDestin[height - 1], 0, width);

	if (pMode >0)
		delete[] pIntImage;

	return(0);

}//sobel



//G0
//-1 0 1
//-2 0 2
//-1 0 1

//G1
//-2  -1  0
//-1   0  1
// 0   1  2

//G2
//-1  -2  -1
// 0   0   0
// 1   2   1

//G3
//0  -1  -2
//1   0  -1
//2   1   0
/**
* @brief					performs kirsch edge detection with the filter masks above
* @param p_rSource	        input image
* @param p_rDestin		    result image
* @param pMode              decides filtering of the detected edges
*/

int kirsch(const BImage& p_rSource, BImage& p_rDestin, int pMode)
{

	//byte const *sLine;
	//byte *dLine0, *dLine1, *dLine2;

	int height = p_rSource.size().height;
	int width = p_rSource.size().width;
	int *pIntImage = NULL;
	int *pRemember = NULL;
	int max = -10000;
	int min = 10000;


	if (pMode > 0)
	{
		pIntImage = new int[height*width];
		pRemember = pIntImage;
	}

	//  Irgendwas initialisieren ?
	//	p_rDestin.fill(255);

	// Zeilen durchlaufen
	for (int y = 1; y < height - 1; y++)
	{
		//Zeilenzugriffe Destination
		const byte	*pLineInPre = p_rSource[y - 1];
		const byte	*pLineInCur = p_rSource[y];
		const byte	*pLineInPost = p_rSource[y + 1];

		byte		*pLineOut = p_rDestin[y];


		pLineOut[0] = 0; // fill boundary

		//Spalten durchlaufen
		for (int x = 1; x < width - 1; x++)
		{

			// Faltung mit G0
			//-1 0 1
			//-2 0 2
			//-1 0 1
			const int	oRawValue0 = std::abs(
				-1 * pLineInPre[x - 1]  + pLineInPre[x + 1]      +
				-2 * pLineInCur[x - 1]  +  2 * pLineInCur[x + 1] +
				-1 * pLineInPost[x - 1] + pLineInPost[x + 1]
				);

			// Faltung mit G1
			//-2  -1  0
			//-1   0  1
			// 0   1  2
			const int	oRawValue1 = std::abs(
				-2 * pLineInPre[x - 1] - pLineInPre[x]     +
				-1 * pLineInCur[x - 1] + pLineInCur[x + 1] +
				pLineInPost[x] + 2 * pLineInPost[x + 1]
				);

			//Faltung mit G2
			//-1  -2  -1
			// 0   0   0
			// 1   2   1
			const int	oRawValue2 = std::abs(
				-1 * pLineInPre[x - 1] - 2 * pLineInPre[x]  -pLineInPre[x + 1] +
				pLineInPost[x - 1] + 2 * pLineInPost[x] + pLineInPost[x + 1]
			);

			//Faltung mit G3
			//0  -1  -2
			//1   0  -1
			//2   1   0
			const int	oRawValue3 = std::abs(
				-1 * pLineInPre[x]  - 2 * pLineInPre[x + 1] +
				pLineInCur[x - 1] - pLineInCur[x + 1] +
				2* pLineInPost[x - 1] + pLineInPost[x]
				);

			//max der 4 Werte
			int oRawValueG = (std::max(oRawValue0, oRawValue1));
			oRawValueG     = (std::max(oRawValueG, oRawValue2));
			oRawValueG     = (std::max(oRawValueG, oRawValue3));


			if (pMode > 0)
			{
				// min max zum Normieren:
				if (oRawValueG > max)
					max = oRawValueG;
				if (oRawValueG < min)
					min = oRawValueG;

				//zwischenspeichern
				*pIntImage++ = oRawValueG;


			}
			else
				pLineOut[x] = static_cast<byte>(std::min(oRawValueG, 255));	// prevent clipping
		} // for
		pLineOut[width - 1] = 0; // fill boundary
	} // for

	//Normieren
	if (pMode > 0)
	{
		pIntImage = pRemember;
		double diff = static_cast<double>(max - min);
		for (int y = 1; y < height - 1; y++)
		{
			byte	*pLineOut = p_rDestin[y];
			for (int x = 1; x < width - 1; x++)
			{
				pLineOut[x] = static_cast<byte>((255.0 / diff) *  static_cast<double> (*pRemember++ - min));
			}
		}
	}

	std::memset(p_rDestin[0], 0, width);
	std::memset(p_rDestin[height - 1], 0, width);

	if (pMode >0)
		delete[] pIntImage;

	return(0);

}//kirsch


int canny(const BImage& p_rSource, BImage& p_rDestin, int pMode)
{

	return (0);
}



} // namespace filter
} // namespace precitec
