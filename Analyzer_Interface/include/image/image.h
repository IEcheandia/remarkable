#ifndef IMAGE_H
#define IMAGE_H


#include "image/ipSignal.h"
#include "image/ipImage.hpp" // includiert  ipImage.h ipLineImage.h ipBlockImage.h

namespace precitec
{
namespace image
{
	typedef TLineImage<byte> BImage;
	typedef TLineImage<uInt> U32Image;
	typedef TLineImage<double> DImage;

	typedef Poco::SharedPtr<BImage>		SmpBImage;
	typedef Poco::SharedPtr<U32Image>	SmpU32Image;
	typedef Poco::SharedPtr<DImage>		SmpDImage;


    enum class ImageFillMode
    {
        Direct, FlippedHorizontal, FlippedVertical, Reverse
    };
    


    template<ImageFillMode tFillMode = ImageFillMode::Direct>
	inline void fillBImage(BImage & rImage, unsigned char* p_oData)
	{
        auto oSize = rImage.size();
        switch (tFillMode)
        {
            case ImageFillMode::Direct:
                assert(rImage.isContiguos());
                //probably here it would have been more efficient to use a shArrayPtr or ShMemPtr instead of initializing the whole image
                std::copy(p_oData, p_oData + oSize.area(), rImage.begin());
                break;
            case ImageFillMode::FlippedHorizontal:
                for (int y=0; y < oSize.height; ++y)
                {
                    auto * pDest = rImage.rowBegin(y);
                    auto * pSrc = p_oData + y*oSize.width;
                    std::reverse_copy(pSrc, pSrc + oSize.width , pDest);
                }
                break;
            case ImageFillMode::FlippedVertical:
                for (int y = 0; y < oSize.height; ++y)
                {
                    auto * pDest = rImage.rowBegin(oSize.height - y -1);
                    auto * pSrc = p_oData + y*oSize.width;
                    std::copy(pSrc, pSrc + oSize.width , pDest);
                }
                break;
            case ImageFillMode::Reverse:
                assert(rImage.isContiguos());
                std::reverse_copy(p_oData, p_oData + oSize.area(), rImage.begin());
                break;
        }
	}

    template<ImageFillMode tFillMode = ImageFillMode::Direct>
	inline BImage fillBImage(const unsigned int p_oWidth, const unsigned int p_oHeight, unsigned char* p_oData)
    {
        BImage oImage(Size2d{static_cast<int> ( p_oWidth ), static_cast<int> ( p_oHeight ) });
        fillBImage<tFillMode>(oImage, p_oData);
        return oImage;
    }

    void transposeImage(BImage & rOutImage, const BImage & rInImage);
    BImage transposeImage(const BImage & rInImage);


	// Testpattern
	inline BImage genModuloPattern(Size2d size, int seed) {
		seed = iMax(seed, 1);
		BImage image(size);
		for (int y=0; y<size.height; ++y)
			for (int x=0; x<size.width; ++x)
				image[y][x] = x%seed; //0xff ; //byte(i& 0x7);
		return image;
	}

	inline bool testModuloPattern(Size2d size, int seed) {
		bool ok = true;
		BImage image(size);
		for (int y=0, i=seed; y<size.height; ++y)
			for (int x=0; x<size.width; ++x, ++i)
				ok &= image[y][x] == byte(i& 0x7);
		return ok;
	}
	
	//test pattern with intensity changes in both directions
    inline BImage genSquaresImagePattern(int squareWidth, int squareHeight, int squaresPerRow, int squaresPerColumn, byte minIntensity = 0, byte maxIntensity = 255)
    {
        BImage out(precitec::geo2d::Size(squareWidth*squaresPerRow, squareHeight*squaresPerColumn));
        if (!out.isValid())
        {
            return out;
        }
        const int numShades = std::max( std::min((squaresPerColumn * squaresPerRow), int(maxIntensity - minIntensity)), 2);
        const double rescaleColor = double(maxIntensity - minIntensity)/(numShades-1);
        
        for (int squareCounter = 0, j = 0; j < squaresPerColumn; j++)
        {
            for (int i = 0; i < squaresPerRow; i++, ++squareCounter)
            {
                auto intensity = std::floor((squareCounter % numShades) * rescaleColor ) + minIntensity;
                assert(intensity >= minIntensity);
                assert(intensity <= maxIntensity);
                for (int y = j * squareHeight, lastY = y + squareHeight; y < lastY; ++y)
                {
                    unsigned char * pPixel = out.rowBegin(y) + i*squareWidth;
                    unsigned char * pLastPixel = pPixel + squareWidth;
                    for ( ; pPixel < pLastPixel; ++pPixel)
                    {
                        *pPixel = intensity;
                    } // for x
                } //for y                          
            } // for i (i-th square on row)
        } // for j (j-th square on column)
        return out;
    };
    
    inline unsigned char testSquaresImagePatternIntensity (int x, int y, int squareWidth, int squareHeight, int squaresPerRow, int squaresPerColumn, byte minIntensity = 0, byte maxIntensity = 255)
    {
        const int numShades = std::max( std::min((squaresPerColumn * squaresPerRow), int(maxIntensity - minIntensity)), 2);
        const double rescaleColor = double(maxIntensity - minIntensity)/(numShades-1);
        int i = x / squareWidth;
        int j = y / squareHeight;
        return ((i + j * squaresPerRow) % numShades) * rescaleColor + minIntensity;
        
    }

} // namespace image
} // namespace precitec

#endif // IMAGE_H
