
/**
 *	@file
 *  @copyright		Precitec Vision GmbH & Co. KG
 *  @author			Simon Hilsenbeck (HS)
 *  @date			2013
 *  @brief			algorithmic interface for images
 */

#include "filter/algoImage.h"
#include "filter/algoStl.h"
#include "system/typeTraits.h"
#include "module/moduleLogger.h"		///< logger
#include "filter/samplingInformation.h"
#include <cmath>
#include <random>

namespace precitec {
	using namespace	image;
namespace filter {

math::Vec3D applyTrafo(const int p_oX, const int p_oY, const interface::Trafo &p_rTrafo)
{
	geo2d::Point oSrc(p_oX, p_oY);
	geo2d::Point oTr(p_rTrafo(oSrc));
	math::Vec3D oDest(oTr.x, oTr.y, 0.0);
	return oDest;
}

math::Vec3D applyTrafo(const math::Vec3D &p_oSrc, const interface::Trafo &p_rTrafo)
{
	geo2d::Point oSrc((int)(0.5+p_oSrc[0]), (int)(0.5+p_oSrc[1]));
	geo2d::Point oTr(p_rTrafo(oSrc));
	math::Vec3D oDest(oTr.x, oTr.y, 0.0);
	return oDest;
}

double calcVariance(const BImage& p_rImageIn) {
	if (p_rImageIn.isValid() == false) {
		return 0;
	} // if

	const unsigned int		oWidth			( p_rImageIn.width() );
	const unsigned int		oHeight			( p_rImageIn.height() );
	const unsigned int		oArea			( oWidth * oHeight );
	unsigned int			oSum			( 0 );
	unsigned int			oSumSqare		( 0 );

	for (unsigned int y = 0; y < oHeight; ++y) {
		const byte* pLine	( p_rImageIn[y] ); // get line pointer
		for (unsigned int x = 0; x < oWidth; ++x) {
			oSum		+= pLine[x];
			oSumSqare	+= pLine[x] * pLine[x];
		} // for
	} // for

	if (oArea - 1 == 0) {
		return 0;
	} // if

	const unsigned int		oMean			( oSum / oArea ); // neq zero asserted above
	const unsigned int		oVariance		( (oSumSqare - oMean*oSum) / (oArea - 1) ); // neq zero asserted above

	return oVariance;
} // calcVariance



byte calcMinMaxDistance(const BImage& p_rImageIn) {
	using std::numeric_limits;

	if (p_rImageIn.isValid() == false) {
		return 0;
	} // if

	const auto		oWidth = p_rImageIn.width();
	const auto		oHeight = p_rImageIn.height();

	auto			oMin = numeric_limits<byte>::max();
	auto			oMax = numeric_limits<byte>::min();

	for (auto oYIn = 0; oYIn < oHeight; ++oYIn) {
		const byte* pLineIn(p_rImageIn[oYIn]); // get line pointer
		for (auto oXIn = 0; oXIn < oWidth; ++oXIn) {
			oMin = pLineIn[oXIn] < oMin ? pLineIn[oXIn] : oMin;
			oMax = pLineIn[oXIn] > oMax ? pLineIn[oXIn] : oMax;
		} // for
	} // for

	return oMax - oMin;
} // calcMinMaxDistance

byte calcMinMaxDistanceDeleteHighLow(const image::BImage& p_rImageIn, int deletePerCent)
{

using std::numeric_limits;

	if (p_rImageIn.isValid() == false) {
		return 0;
	} // if

	if ((deletePerCent <= 0) || (deletePerCent >= 50))
	{
		return calcMinMaxDistance(p_rImageIn);
	}

	const int		oWidth = p_rImageIn.width();
	const int		oHeight = p_rImageIn.height();

	int totalCount = oWidth * oHeight;
	int toDelete = (int)(0.5 + ((double)deletePerCent / 100.0) * totalCount);

	std::vector<int> histo(256);
	for (int i = 0; i < 256; i++) histo[i] = 0;

	auto			oMin = 0;
	auto			oMax = 255;

	for (auto oYIn = 0; oYIn < oHeight; ++oYIn) {
		const byte* pLineIn(p_rImageIn[oYIn]); // get line pointer
		for (auto oXIn = 0; oXIn < oWidth; ++oXIn)
		{
			histo[pLineIn[oXIn]]++;
		} // for
	} // for

	// jetzt Min/Max bestimmen, ohne die hoehsten/tiefsten

	int sum = 0;
	for (int i = 0; i < 256; i++)
	{
		sum += histo[i];
		if (sum > toDelete)
		{
			oMin = i;
			break;
		}
	}

	sum = 0;
	for (int i = 255; i >= 0; i--)
	{
		sum += histo[i];
		if (sum > toDelete)
		{
			oMax = i;
			break;
		}
	}

	return oMax - oMin;
} // calcMinMaxDistance



double calcGradientSumX(const image::BImage& p_rImageIn) {
	const	auto oHeight		= p_rImageIn.height();
	const	auto oWidthMinOne	= p_rImageIn.width() - 1;
	const	auto oArea			= oHeight * oWidthMinOne;
			auto oSumX			= 0;

	for (auto oY = 0; oY < oHeight; ++oY) {
		const auto* pLine = p_rImageIn[oY];
		for (auto oX = 0; oX < oWidthMinOne; ++oX) {
			// diff benachbarter Punkte in x-richtung
			oSumX += std::abs(pLine[oX] - pLine[oX + 1]);
		} // for
	} // for
	double	oGradientSumX = oArea != 0 ? double(oSumX) / oArea : 0;
	return oGradientSumX;
} // calcGradientSumX



double calcGradientSumY(const image::BImage& p_rImageIn) {
	const	auto oHeightMinOne = p_rImageIn.height() - 1;
	const	auto oWidth = p_rImageIn.width();
	const	auto oArea = oHeightMinOne * oWidth;
	auto oSumY = 0;

	for (auto oY = 0; oY < oHeightMinOne; ++oY) {
		const auto* pLine = p_rImageIn[oY];
		const auto* pNextLine = p_rImageIn[oY + 1];
		for (auto oX = 0; oX < oWidth; ++oX) {
			// diff benachbarter Punkte in y-richtung
			oSumY += std::abs(pLine[oX] - pNextLine[oX]);
		} // for
	} // for
	double	oGradientSumY = oArea != 0 ? double(oSumY) / oArea : 0;
	return oGradientSumY;
} // calcGradientSumY


double calcTexture(const image::BImage& p_rImageIn, image::BImage & p_rImageTmp)
{
	const	auto oHeight = p_rImageIn.height() - 1;
	const	auto oWidth = p_rImageIn.width();

	p_rImageTmp.resizeFill(Size2d(oWidth, oHeight),0);
	binaerItMean(p_rImageIn, p_rImageTmp);
	filterBinNoise5x5(p_rImageTmp);

	return binaerSumX(p_rImageTmp);

}


double calcTexture(const image::BImage& p_rImageIn)
{
	image::BImage imageTmp;
	return calcTexture(p_rImageIn, imageTmp);

}



double calcMeanIntensity(const image::BImage& p_rImageIn)
{
	// Loop through frame sum up intensity

	unsigned long oSum = 0ul;
    if (p_rImageIn.isValid())
    {
        p_rImageIn.for_each([&oSum](byte pixel){ oSum += pixel;});
    }
    
	const auto oNbPixels = p_rImageIn.size().area();
	const auto oMeanIntensity = oNbPixels != 0 ? double(oSum) / oNbPixels : 0;

	return oMeanIntensity;
} // calcMeanIntensity

// step effect:
// 700X800, GCC 4.6 O2, ++x, ++y: ~36.5 us
// 700X800, GCC 4.6 O2, x+=1, y+=1: ~43.7 us
// 700X800, GCC 4.6 O2, x+=2, y+=2: ~ 12 us
double calcMeanIntensityWithStep(const image::BImage& p_rImageIn, unsigned int p_oStep)
{
	poco_assert_dbg(p_oStep != 0);

	// Loop through frame sum up intensity
	unsigned long oSum = 0ul;
    
        for (auto oY = 0, h = p_rImageIn.height(); oY < h; oY += p_oStep) {
            const auto pLine = p_rImageIn[oY];
            for (int oX = 0, w = p_rImageIn.width(); oX < w; oX += p_oStep)
            {
                oSum += pLine[oX];
            } // for
        } // for
    
	const auto oNbPixels = (p_rImageIn.size().width / p_oStep) * (p_rImageIn.size().height / p_oStep); // integer div ok
	const auto oMeanIntensity = oNbPixels != 0 ? double(oSum) / oNbPixels : 0;

	return oMeanIntensity;
} // calcMeanIntensity



void calcBinarizeDynamic(
	const BImage&					p_rImageIn,
	ComparisonType 					p_oComparisonType,
	byte							p_oDistToMeanIntensity,
	BImage&							p_rImageOut) {

    assert(p_rImageIn.size() == p_rImageOut.size());
    
    if (p_oComparisonType == eLess)
    {
        const int oThreshold = roundToT<int>(calcMeanIntensityWithStep(p_rImageIn, 2)) - p_oDistToMeanIntensity;
       p_rImageIn.transformTo( p_rImageOut, [&oThreshold] (byte p) {return p < oThreshold ? 255 : 0;});
    }
    else
    {
        const int oThreshold = roundToT<int>(calcMeanIntensityWithStep(p_rImageIn, 2)) + p_oDistToMeanIntensity;
        p_rImageIn.transformTo(p_rImageOut, [&oThreshold] (byte p) {return p >= oThreshold ? 255 : 0;});
    }
	
} // calcBinarizeDynamic



void calcBinarizeStatic(
	const BImage&					p_rImageIn,
	ComparisonType 					p_oComparisonType,
	byte							p_oThreshold,
	BImage&							p_rImageOut) {
    
    assert(p_rImageIn.size()  == p_rImageOut.size());
     if (p_oComparisonType == eLess)
    {
        p_rImageIn.transformTo(p_rImageOut, [&p_oThreshold] (byte p) {return p < p_oThreshold ? 255 : 0;});
    }
    else
    {
        p_rImageIn.transformTo(p_rImageOut, [&p_oThreshold] (byte p) {return p >= p_oThreshold ? 255 : 0;});
    }
    
} // calcBinarizeStatic



void calcBinarizeLocal(
	const image::BImage&					p_rImageIn,
	ComparisonType 							p_oComparisonType,
	byte									p_oDistToMeanIntensity,
	image::BImage&							p_rImageOut) {
	const auto oSizeFilter		= 5;	// souvis proven value
	const auto oSearchWinStep	= 2;	// souvis proven value
	const auto oSizeSearchWin	= 20;	// souvis proven value
	const auto oNbPixSearchWin	= (oSizeSearchWin * oSizeSearchWin) / (oSearchWinStep * oSearchWinStep);
	const auto oNbRows			= p_rImageIn.height() - oSizeSearchWin;
	const auto oNbCols			= p_rImageIn.width() - oSizeSearchWin;

	for (auto oRow = 0; oRow < oNbRows; oRow += oSizeFilter) {
		for (auto oCol = 0; oCol < oNbCols; oCol += oSizeFilter) {
			//Durchnitt rechnen in 20*20 Kasten (aber nur 100 werte summiert)
			auto oSum = 0;
			for (auto oY = 0; oY < oSizeSearchWin; oY += oSearchWinStep) {
				const unsigned char* line = p_rImageIn[oRow + oY];
				for (auto oX = 0; oX <oSizeSearchWin; oX += oSearchWinStep) {
					oSum += line[oCol + oX];
				} // for
			} // for

			const auto oSign		= p_oComparisonType == eGreaterEqual ? +1 : -1;
			const auto oThreshold	= (oSum / oNbPixSearchWin) + (oSign * p_oDistToMeanIntensity);
			const auto oStartY		= (oSizeSearchWin - oSizeFilter) / 2;
			const auto oStartX		= oStartY;
			const auto oEndY		= oStartY + oSizeFilter;
			const auto oEndX		= oEndY;

			//Gleich in TempRoi schreiben, ohne dummy speicherung, ohne ueberlappung
			for (auto oY = oStartY; oY < oEndY; ++oY) {
				const auto* pLine		= p_rImageIn[oRow + oY];
				auto* 		pLineOut	= p_rImageOut[oRow + oY];
				for (auto oX = oStartX; oX < oEndX; ++oX) {
					if (p_oComparisonType == eLess) {
						pLineOut[oCol + oX] = (pLine[oCol + oX] < oThreshold) ? 255 : 0;
					}
					else {
						pLineOut[oCol + oX] = (pLine[oCol + oX] >= oThreshold) ? 255 : 0;
					}
				} // for
			} // for
		} // for
	} // for
} // calcBinarizeLocal



void filterBinNoise5x5(image::BImage& p_rImageIn)
{
	const int cols = p_rImageIn.width();
	const int rows = p_rImageIn.height();
	int x, y, xi;
	for (y = 1; y<rows - 2; y++)
	{
		const unsigned char* linePrev = p_rImageIn.rowBegin(y - 1);
		const unsigned char* lineCurr = p_rImageIn.rowBegin(y);
		const unsigned char* lineNext = p_rImageIn.rowBegin(y + 1);
		const unsigned char* lineNxt2 = p_rImageIn.rowBegin(y + 2);
		for (x = 1; x<cols - 2; x++)
		{
			if (lineCurr[x] > 0)
			{
				// Filterkern
				int sum = 0;
				for (int i = -1; i<3; i++)
				{
					xi = x + i;
					sum += linePrev[xi] + lineNxt2[xi];
					if (-1 == i || 2 == i)
						sum += lineCurr[xi] + lineNext[xi];
				}
				if (sum == 0)
				{
					p_rImageIn(x, y) = 0;
				}
			}
		}//for(x+...
	}//for(y=...
} // filterBinNoise5X5

double binaerSumX(const image::BImage& p_rImageIn)
{
	const int cols = p_rImageIn.width();
	const int rows = p_rImageIn.height();

	double valueX;

	const double area(cols*rows);
	if ( area == 0 )
	{
		return 0.0;
	}
	// rechne Flaeche im Bin image
	int   iSumX = 0;

	for ( int i = 0; i<rows; i++ )
	{

		const unsigned char* line = p_rImageIn.rowBegin(i);
		for ( int j = 0; j < (cols - 1); j++ ) // j ist Spaltenindex
		{
			// diff benachbarter Punkte in x-richtung und in y-richtung
			iSumX += std::abs(line[j] - line[j + 1]);
		}
	};
	valueX = iSumX / area;

	return valueX;
}

void binaerItMean(const image::BImage & p_rImageIn, image::BImage& p_rImageTmp)
{
	const int cols = p_rImageIn.width() - 20;
	const int rows = p_rImageIn.height() - 20;

	int var = 7;
	//const int pitch = aoi->pitch();
	//const int pitchTmp = tempRoi->pitch();
	int y,	//tmp indice
		n, m,	//source indices
		i, j,	//mask indices
		sum, threshold;
	for (n = 0; n<rows; n += 5)
	{
		for (m = 0; m<cols; m += 5)
		{
			//Durchnitt rechnen in 20*20 Kasten (aber nur 100 werte summiert)
			sum = 0;
			for (i = 0; i<20; i += 2)
			{
				const unsigned char* lineCurr = p_rImageIn.rowBegin(n + i);
				//const unsigned char* line = &(image[(n + i)*pitch]) + m;
				for (j = 0; j<20; j += 2)
				{
					sum += lineCurr[j+m];
				}
			}
			threshold = (int)((var + sum)*0.01);
			//Gleich in TempRoi schreiben, ohne dummy speicherung, ohne Ueberlappung
			for (i = 7; i<12; i++)
			{
				y = n + i;
				const unsigned char* lineCurr = p_rImageIn.rowBegin(y);
				//const unsigned char* line = &(image[y*pitch]);
				unsigned char* lineCurrTmp = p_rImageTmp.rowBegin(y);
				//unsigned char* lineTmp = &(imageTmp[y*pitchTmp]);
				for (j = 7; j<12; j++)
				{
					lineCurrTmp[j + m] = (lineCurr[j + m] <= threshold) ? 0 : 255;
				}
			}
		}
	}
}

//gray scale erosion  + image difference + subsampling + binarization
void MorphEdge2Buf2(const image::BImage & p_rImageIn, image::BImage& p_rImageTmp, const byte p_threshold )
{
	//p_rImageTmp size must be checked outside, it's actually important because the border could affect the mean later

	//compute the borders
	const int jOffset = 1;
	const int jMax = p_rImageIn.width() - 1;
	const int iOffset = 1;
	const int iMax = p_rImageIn.height() - 1;
	for ( int i = iOffset; i < iMax; i += 2 )
	{
		int j1;
		int j0 = 0;
		int j2 = 1;
		for ( int j = jOffset; j < jMax; j += 2 )
		{
			j0 = j2;
			j1 = j0;
			j2 = j + 1;
			const byte iPix0 = p_rImageIn.getValue(j0, i);
			const byte iPix1 = p_rImageIn.getValue(j1, i);
			const byte iPix2 = p_rImageIn.getValue(j2, i);

			const byte resultErosion = std::min({iPix0, iPix1, iPix2});

			const byte delta = iPix0 - resultErosion;
			const byte result = delta >= p_threshold ? 0 : 255;

			for ( auto && ii : {-1,0} )
			{
				for ( auto && jj : {-1,0} ) //in souvis it was {0,1}
				{
					p_rImageTmp.setValue(j+ jj, i + ii, result);
				} //for ii
			}//for jj
		}//for j
	}//for i
}

double calcStructure(const image::BImage& p_rImageIn)
{
	BImage imageTmp;
	return calcStructure(p_rImageIn, imageTmp);

} // calcStructure


double calcStructure(const image::BImage& p_rImageIn, image::BImage& p_rImageTmp)
{
	//it makes sense only for image size > 2,2

	byte oTreshold = 10;

	int outW = std::floor((p_rImageIn.width() - 1) / 2.0) * 2;
	int outH = std::floor((p_rImageIn.height() - 1) / 2.0) * 2;

	p_rImageTmp.resizeFill(Size2d(outW, outH),0);
	MorphEdge2Buf2(p_rImageIn, p_rImageTmp, oTreshold);
	double valX = binaerSumX(p_rImageTmp);

	return valX;

} // calcStructure with temp image



byte calcMinIntensity(const image::BImage& p_rImageIn)
{
	byte minIntensity = 255;
    p_rImageIn.for_each([&minIntensity](byte pixel){ minIntensity = minIntensity < pixel ? minIntensity : pixel;});

	return minIntensity;
}

byte calcMaxIntensity(const image::BImage& p_rImageIn)
{
	byte maxIntensity = 0;
    p_rImageIn.for_each([&maxIntensity](byte pixel){
            maxIntensity = maxIntensity > pixel ? maxIntensity : pixel;});

	return maxIntensity;
}



template <typename TInput, typename TTable>
SummedAreaTable<TInput, TTable>::SummedAreaTable(const image::TLineImage<TInput>  & p_rImageIn, SummedAreaTableOperation p_oOperation )
{
    switch  (p_oOperation)
    {
        case SummedAreaTableOperation::sumValues:
            init<SummedAreaTableOperation::sumValues>(p_rImageIn);
            break;
        case SummedAreaTableOperation::sumSquaredValues:
            init<SummedAreaTableOperation::sumSquaredValues>(p_rImageIn);
            break;
    };
}


template <typename TInput, typename TTable>
template<SummedAreaTableOperation t_operation>
void SummedAreaTable<TInput, TTable>::init(const image::TLineImage<TInput>  & p_rImageIn)
{
    const unsigned int BORDER = 1; //integral image has a left and upper border filled with 0
    
    const geo2d::Size oInSize = p_rImageIn.size();
    m_IntegralImage.resize(geo2d::Size(oInSize.width + BORDER, oInSize.height + BORDER));

    //make sure first row is filled with 0
    std::fill_n(m_IntegralImage.rowBegin(0), m_IntegralImage.width(), 0);

    for ( int y_input = 0, y_int = 0; y_input < oInSize.height; ++y_input, ++y_int )
    {
        assert(y_int == y_input );
        auto rIntegralImageCurrentRow = m_IntegralImage.rowBegin(BORDER + y_int);
        auto rIntegralImagePreviousRow = m_IntegralImage.rowBegin(BORDER + y_int - 1);
        auto rImageInCurrentRow = p_rImageIn.rowBegin(y_input);
        
        //make sure first column is filled with 0
        rIntegralImageCurrentRow[0] = 0; 
        
        auto pIntegralImageAbove = rIntegralImagePreviousRow; 
        auto pIntegralImage = rIntegralImageCurrentRow;
        auto pInputImage = rImageInCurrentRow;
        
        for ( int x_input = 0, x_int = 0; x_input < oInSize.width; ++x_input, ++x_int )
        {
            assert(x_int == x_input);
            TTable val = (*pInputImage);
            if (t_operation == SummedAreaTableOperation::sumSquaredValues)
            {
                val *=  (*pInputImage);
            }
            val -= (*pIntegralImageAbove) ;
            val += (*(++pIntegralImageAbove));
            val += *pIntegralImage;
            ++pIntegralImage;
            (*pIntegralImage) = val;
            
            if (t_operation == SummedAreaTableOperation::sumValues)
            {
                assert(rIntegralImageCurrentRow[BORDER+x_int] == 
                    rImageInCurrentRow[x_input] - rIntegralImagePreviousRow[BORDER + x_int - 1] + rIntegralImagePreviousRow[BORDER + x_int] + rIntegralImageCurrentRow[BORDER + x_int - 1]);
            }
            else
            {
                assert(rIntegralImageCurrentRow[BORDER+x_int] ==  
                    rImageInCurrentRow[x_input]*rImageInCurrentRow[x_input] - rIntegralImagePreviousRow[BORDER + x_int - 1] + rIntegralImagePreviousRow[BORDER + x_int] + rIntegralImageCurrentRow[BORDER + x_int - 1]);
            }
            
            ++pInputImage;

        }
    }
}

template <typename TInput, typename TTable>
TTable SummedAreaTable<TInput, TTable>::calcSum( int xMin, int xMax, int yMin, int yMax) const
{
    assert( xMin >= 0 && xMin < m_IntegralImage.width());
    assert( xMax >= 0 && xMax < m_IntegralImage.width());
    assert( yMin >= 0 && yMin < m_IntegralImage.height());
    assert( yMax >= 0 && yMax < m_IntegralImage.height());
    assert( xMin <= xMax && yMin <= yMax);
    
    auto * firstRow = m_IntegralImage.rowBegin(yMin);
    TTable A =*(firstRow+xMin);
    TTable B =*(firstRow+xMax);
    auto * lastRow = m_IntegralImage.rowBegin(yMax);
    TTable C = *(lastRow+xMin);
    TTable D = *(lastRow+xMax);
    return D - B - C + A;
}


//nearest neighbour interpolation
void upsampleImage(image::BImage & outImage, const image::BImage& rImage, unsigned int dx, unsigned int dy)
{
    assert( !(rImage.overlapsWith(outImage)) && "output image can't overlap with input image");

    //example: originalH=38, tileJump = 10
	// h=3 , dy = 12 , H = 36 

	unsigned int srcW = rImage.width();
	unsigned int srcH = rImage.height();
	unsigned int dstW = srcW*dx;
	unsigned int dstH = srcH*dy;
    
    outImage.resize(Size2d( dstW, dstH ));
    if ( srcW==dstW && srcH==dstH )
    {
        rImage.copyPixelsTo(outImage);
        return;
    }
    for ( unsigned int srcY = 0; srcY < srcH; srcY++ )
    {
        unsigned int dstY = srcY*dy;
        for ( unsigned int ty = 0; ty < dy; ++ty, ++dstY )
        {            
            auto * pSrc = rImage.rowBegin(srcY);
            auto * pDst = outImage.rowBegin(dstY);
            for ( unsigned int srcX = 0; srcX < srcW; srcX++, pSrc++ )
            {
                unsigned int dstX = srcX*dx;
                for ( unsigned int tx = 0; tx < dx; ++tx, ++dstX, ++pDst)
                {
                    assert(dstX < dstW );
                    assert(rImage[srcY] + srcX == pSrc);
                    assert(outImage[dstY] + dstX == pDst);
                    *pDst = *pSrc;
                }
            }
        }
    }
}

//skip pixels, fastest method but can create artifacts 
void downsampleImage(image::BImage & outImage, const image::BImage& rImage, unsigned int dx, unsigned int dy)
{
    assert( !(rImage.overlapsWith(outImage)) && "output image can't overlap with input image");
    
    //example: originalH=38, tileJump = 10
	// h=3 , dy = 12 , H = 36 

	unsigned int srcW = rImage.width();
	unsigned int srcH = rImage.height();
	unsigned int dstW = srcW/dx; //integer division
	unsigned int dstH = srcH/dy; //integer division
	unsigned int x0 = ( srcW%dx)/2;
    unsigned int y0 = ( srcH%dy)/2;
    
    outImage.resize(Size2d( dstW, dstH ));
    if ( srcW==dstW && srcH==dstH )
    {
        rImage.copyPixelsTo(outImage);
        return;
    }
    
    for (unsigned int dstY = 0; dstY < dstH; dstY++)
    {
        unsigned int srcY = dstY * dy + y0;
        assert(srcY < srcH );
        auto * pDst = outImage.rowBegin(dstY);
        auto * pSrc = rImage.rowBegin(srcY) + x0;
        for (unsigned int dstX = 0; dstX < dstW; dstX++, pSrc += dx, pDst++)
        {
#ifndef NDEBUG
            unsigned int srcX = dstX * dx + x0;
            assert(srcX < srcW );
            assert(outImage[dstY] + dstX == pDst);
            assert(rImage[srcY] + srcX == pSrc);
#endif
            *pDst = *pSrc;
        }
    }
}


void resampleFrame (BImage& rOutputImage, interface::ImageContext& rOutputContext, const BImage& rInputImage, const interface::ImageContext& rInputContext)
{
    using interface::ImageContext;
    
    const auto inputSize = rInputImage.size();
    SamplingInformation inputSamplingInfo {inputSize, rInputContext.SamplingX_, rInputContext.SamplingY_};
    SamplingInformation outputSamplingInfo {inputSamplingInfo.getReferenceImageSize(SamplingInformation::BorderMode::excluded), 1.0, 1.0 };;
    
    const auto & outputSize = outputSamplingInfo.getImageSize();
    
    rOutputImage.resize(outputSize);
    
    //generate a new context with the required sampling
    {
        interface::LinearTrafo oSubTrafo(outputSamplingInfo.m_offsetToReferenceImage); //from original image to sampled image
        auto pOutputTrafo = oSubTrafo(rInputContext.trafo()); // from image source
        
        rOutputContext = interface::ImageContext(rInputContext, std::move(pOutputTrafo));
        rOutputContext.SamplingX_ = 1.0;
        rOutputContext.SamplingY_ = 1.0;

    }
    if (inputSize == outputSize)
    {
        rInputImage.copyPixelsTo(rOutputImage);
        return;
    }
    
        
    if ( (rInputContext.SamplingX_ >= 1) && (rInputContext.SamplingY_ >= 1) )
    {        
        downsampleImage(rOutputImage, rInputImage, inputSamplingInfo.m_samplingFactorX.m_factor, inputSamplingInfo.m_samplingFactorY.m_factor);
    }
    else if ( (rInputContext.SamplingX_ <= 1) && (rInputContext.SamplingY_ <= 1) )
    {
        upsampleImage(rOutputImage, rInputImage, inputSamplingInfo.m_samplingFactorX.m_factor, inputSamplingInfo.m_samplingFactorY.m_factor);
    }
    else
    {
        //upsampling in one direction, downsampling in the other
        unsigned int srcW = inputSize.width;
        unsigned int srcH = inputSize.height;
        double dx = outputSize.width / srcW;
        double dy = outputSize.height / srcH;
        
        for ( unsigned int srcY = 0; srcY < srcH; srcY++ )
        {
            unsigned int dstY = srcY*dy;
            const unsigned int dstYEnd = dstY+dy;
            auto * pSrc = rInputImage.rowBegin(srcY);
            for (; dstY < dstYEnd; ++dstY )
            {            
                assert((int)dstY < outputSize.height);
                auto * pDst = rOutputImage.rowBegin(dstY);
                for (unsigned int srcX = 0; srcX < srcW; srcX++, pSrc++ )
                {
                    unsigned int dstX = srcX*dx;
                    const unsigned int dstXEnd = dstX+dx;
                    for ( ; dstX < dstXEnd; ++dstX, ++pDst)
                    {
                        assert((int)dstX < outputSize.width);
                        assert(rInputImage[srcY] + srcX == pSrc);
                        assert(rInputImage[dstY] + dstX == pDst);
                        *pDst = *pSrc;
                    }
                }
            }
        }
    }
    
}


BImage genNoiseImage (Size2d imageSize, int seed )
{
    
    std::mt19937 gen(seed); //Standard mersenne_twister_engine seeded with seed
    std::uniform_int_distribution<int> distribution(0,255);
    BImage outImage{imageSize};
    assert(outImage.isContiguos());
    std::for_each(outImage.begin(), outImage.end(),[&distribution, &gen](byte & pixel)
        {
            pixel = static_cast<byte>(distribution(gen));
        } 
    );
    return outImage;

}
 


} // namespace filter
} // namespace precitec


