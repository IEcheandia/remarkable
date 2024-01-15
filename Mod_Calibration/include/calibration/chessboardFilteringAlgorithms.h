#ifndef CHESSBOARDFILTERINGALGORITHM_H
#define CHESSBOARDFILTERINGALGORITHM_H
/*
 * To be included in weldmaster and wmcalibration 
 */

#include <vector>
#include <image/image.h>
#include <numeric>
#include <array>

namespace precitec {
namespace calibration_algorithm {

class array2D
{
public:
    typedef std::vector<std::vector<double> > tConvMatrix;
    explicit array2D(const int p_oSize);  ///< Init with real size, not kernel size.
    explicit array2D();                   ///< Empty constructor.
    void init();                          ///< Init and allocate/ assign array.
    virtual ~array2D(){}                  ///< Virtual destructor.

    double at(const int p_oX, const int p_oY) const;          ///< Return element at line p_oY, column p_oX.
    void set(const int p_oX, const int p_oY, double p_oVal);  ///< Set element at line p_oY, column p_oX to value p_oValue.
    int size() const { return m_oSize; }                      ///< Return size of array.
    int kernelSize() const { return m_oKernelSize; }          ///< Return size of kernel.
    void setSize(const int p_oSize);                          ///< Set real size and reinit matrix. p_oSize should be an odd number.
    /**
     * @brief setArray     Copy p_rVec entries to array.
     * @param p_rVec       Source vector.
     * @param p_oSize      Number of rows of array.
     * @param oMultiplier  Multiplier used to multiply p_rVec entries.
     * @return Returns false, if size of p_rVec is unequal to p_oSize*p_oSize.
     *
     * Copy p_rVec entries to array, multiplying values > 0.000001 with oMultiplier.
     */
    bool setArray(const std::vector<double> &p_rVec, const int p_oSize, const double oMultiplier=1);
	double getNorm() const; ///< Get norm of a filter kernel (sum of entries)
    image::DImage::constIterator1D transposedBegin() const;
    image::DImage::constIterator1D transposedEnd() const;

protected:
    tConvMatrix m_oData; ///< The data itself.
    image::DImage m_oFilterTransposed;
    int m_oSize;         ///< Real size = 2*m_oKernelSize + 1.
    int m_oKernelSize;   ///< Kernel size.
};

struct FilteringAlgorithms
{
    enum CornerDetectionType
    {
        eBlackToWhite, ///< Black to white convolution kernel (from top to bot and left to right)
        eWhiteToBlack ///< White to black convolution kernel (from top to bot and left to right)
    };

    FilteringAlgorithms()
    {
        //initialize all masks
        detectionMask(CornerDetectionType::eBlackToWhite);
        detectionMask(CornerDetectionType::eWhiteToBlack);
        maskErosion();
        maskDilation();
        maskGaussian();

    }

    static const int m_oDetectionMaskSize = 13;
    static const int m_oGaussianMaskSize = 3;
    static const int m_oMorphologicalMaskSize = 3;

    static const array2D & maskErosion()    ///< Morphological erosion
    {
        static const array2D oMaskErosion = [](){
            array2D mask;
            std::vector<double> oErosion { 1,1,1, 1,1,1, 1,1,1 };
            mask.setArray(oErosion, m_oMorphologicalMaskSize);
            return mask;
        }();
        return oMaskErosion;

    }
    static const array2D & maskDilation()    ///< Morphological dilation
    {
        static array2D oMaskDilation = [](){
            array2D mask;
            std::vector<double> oDilation{ 0,1,0, 1,1,1, 0,1,0 };
            mask.setArray(oDilation, m_oMorphologicalMaskSize);
            return mask;
        }();
        return oMaskDilation;
    }

    static const array2D & maskGaussian()
    {
        static array2D oMaskGaussian = [](){
            array2D mask;
            createGaussSmoothingFilter(mask, m_oGaussianMaskSize);
            return mask;
        }();
        return oMaskGaussian;
    }

    static void createGaussSmoothingFilter(array2D &p_rGaussFilter, const int p_oDim)          ///< Create simple gaussian 2D smoothing filter kernel
    {
        const double oNorm = (p_oDim*p_oDim);
        double oSum = 0;
        p_rGaussFilter.setSize(2*p_oDim+1);
        for (int i=-p_oDim; i <= p_oDim; ++i)
        {
            for (int j=-p_oDim; j <= p_oDim; ++j)
            {
                double val = std::exp(-i / oNorm)*std::exp(-j / oNorm);
                p_rGaussFilter.set( p_oDim+i, p_oDim+j,  val);
                oSum += val;
            }
        }
        for ( int i = 0; i < 2*p_oDim+1; ++i )
        {
            for ( int j = 0; j < 2 * p_oDim + 1; ++j )
            {
                double val = p_rGaussFilter.at(i,j);
                p_rGaussFilter.set(i,j, val/oSum);
            }
        }

    }

    static const array2D & detectionMask(CornerDetectionType p_type)  ///< Create edge detection filter kernels
    {
        static const array2D invalidMask;

        const static std::array<array2D, 2> detectionMasks = [] ()
        {
            /* these smaller masks can be used for smaller patterns!
            std::vector<double> oMask { 1,0,1,0,0,0,0,0,-1,0,-1,
                                        0,1,0,0,0,0,0,0,0,-1, 0,
                                        1,0,1,0,0,0,0,0,-1,0,-1,
                                        0,0,0,1,0,0,0,-1,0,0,0,

                                        0,0,0,0,0,0,0,0,0,0,0,
                                        0,0,0,0,0,0,0,0,0,0,0,
                                        0,0,0,0,0,0,0,0,0,0,0,

                                        0,0,0,-1,0,0,0,1,0,0,0,
                                        -1,0,-1,0,0,0,0,0,1,0,1,
                                        0,-1,0,0,0,0,0,0,0,1,0,
                                        -1,0,-1,0,0,0,0,0,1,0,1
                                    };

            std::vector<double> oMask { 1,0,0,0,0,0,0,0,0,0,-1,
                                        1,1,0,0,0,0,0,0,0,-1,-1,
                                        1,1,0,0,0,0,0,0,0,-1,-1,
                                        0,0,1,0,0,0,0,0,-1,0,0,

                                        0,0,0,0,0,0,0,0,0,0,0,
                                        0,0,0,0,0,0,0,0,0,0,0,
                                        0,0,0,0,0,0,0,0,0,0,0,

                                        0,0,-1,0,0,0,0,0,1,0,0,
                                        -1,-1,0,0,0,0,0,0,0,1,1,
                                        -1,-1,0,0,0,0,0,0,0,1,1,
                                        -1,0,0,0,0,0,0,0,0,0,1
                                    };*/
            std::vector<double> oMask { 1,0,0,0,0,0,0,0,0,0,0,0,-1,  // this mask worked best
                                        1,1,0,0,0,0,0,0,0,0,0,-1,-1,
                                        1,1,0,0,0,0,0,0,0,0,0,-1,-1,
                                        0,0,1,0,0,0,0,0,0,0,-1,0,0,

                                        0,0,0,0,0,0,0,0,0,0,0,0,0,
                                        0,0,0,0,0,0,0,0,0,0,0,0,0,
                                        0,0,0,0,0,0,0,0,0,0,0,0,0,
                                        0,0,0,0,0,0,0,0,0,0,0,0,0,
                                        0,0,0,0,0,0,0,0,0,0,0,0,0,

                                        0,0,-1,0,0,0,0,0,0,0,1,0,0,
                                        -1,-1,0,0,0,0,0,0,0,0,0,1,1,
                                        -1,-1,0,0,0,0,0,0,0,0,0,1,1,
                                        -1,0,0,0,0,0,0,0,0,0,0,0,1
                                    };

            double oPositiveSum = std::accumulate(oMask.begin(), oMask.end(), 0.0, [] (double acc, double entry)
            {
                return entry > 0 ? acc + entry : acc;
            });
            array2D m_oMaskW2B, m_oMaskB2W;
            m_oMaskW2B.setArray(oMask, m_oDetectionMaskSize, 1 / oPositiveSum); // kernel size 5 -> array size 2*5 + 1
            m_oMaskB2W.setArray(oMask, m_oDetectionMaskSize, -1 / oPositiveSum);
            return std::array<array2D, 2>{m_oMaskW2B, m_oMaskB2W};
        }();

        if (p_type >= 0 && p_type < 2)
        {
            return detectionMasks[p_type];
        }
        return invalidMask;
    }

    /**
     * @brief Convolution Convolutes p_rSource with kernel p_rFilter and return the result.
     * @param p_rSource          Source image to be convoluted.
     * @param p_rFilter          Kernel to apply.
     * @param p_oFilter          Kernel/Filter name.
     * @param p_oImgToReplace    Memory to the new image. Will be allocated if necessary.
     * @return Concoluted image as QImage.
     *
     * Convolution Convolutes p_rSource with kernel p_rFilter and return the result as a QImage pointer. The filtername can be given in p_oFilter
     * and is shown at the GUI, so the user is informed about the current action. 
     *
     */
    
    // for an 11x11 kernel, the filtersizes are 5, 5
    // Convolute image with filter
    template <class CallbackOnLine>
    static precitec::image::BImage convolution(const precitec::image::BImage &p_rSource, const array2D &p_rFilter, CallbackOnLine p_callbackOnLine)
    {
        if (! p_rSource.isValid())
        {
            return {};
        }
        
        precitec::image::BImage oRetImg(p_rSource.size());
        
        double oNorm = p_rFilter.getNorm(); 
        if ( std::abs(oNorm) < 0.00001 ) oNorm = 1;
        const int oKernelSize = p_rFilter.kernelSize();
    
        for (int y=oKernelSize, end_y =  p_rSource.height()-oKernelSize; y < end_y; ++y)
        {
            unsigned char * oLine = oRetImg.rowBegin(y);
            for (int x=oKernelSize, end_x =  p_rSource.width()-oKernelSize; x < end_x; ++x)
            {
                
                auto itFilter = p_rFilter.transposedBegin();

                double oVal = 0;

                auto fMultiplyFilter = [&itFilter, & oVal] (const byte & pixelSource)
                {
                    oVal += (double(pixelSource) * (*itFilter)) ;
                    itFilter ++;
                };

                p_rSource.for_each(fMultiplyFilter, x- oKernelSize, y - oKernelSize, 2*oKernelSize + 1, 2*oKernelSize + 1); 

                
                if (oVal <= 0)
                {
                    oLine[x] = 0;
                } else
                {
                    oLine[x] = (oVal < 255 ? oVal : 255);
                }
            }
            p_callbackOnLine (y);
        }
        
        return oRetImg;
    }

    static precitec::image::BImage convolution(const precitec::image::BImage &p_rSource, const array2D &p_rFilter)
    {
        return convolution(p_rSource, p_rFilter, [](int){});
    }

    template <class CallbackOnLine>
    static precitec::image::BImage morphBW(const precitec::image::BImage &p_rSource, const array2D &p_rFilter, CallbackOnLine p_callbackOnLine)
    {
        using precitec::image::BImage;
        if (! p_rSource.isValid())
        {
            return {};
        }
        
        BImage oRetImg(p_rSource.size());

        double oNorm = p_rFilter.getNorm(); 
        if ( std::abs(oNorm) < 0.00001 ) oNorm = 1;
        const int oKernelSize = p_rFilter.kernelSize();

        for (int y=oKernelSize, end_y =  p_rSource.height()-oKernelSize; y < end_y; ++y)
        {
            unsigned char * oLine = oRetImg.rowBegin(y);
            for (int x=oKernelSize, end_x =  p_rSource.width()-oKernelSize; x < end_x; ++x)
            {
                                        
                int oVal = 0; 
                oNorm = 1; 
                
                auto itFilter = p_rFilter.transposedBegin();

                auto fMultiplyFilter = [&itFilter, & oVal] (const byte & pixelSource)
                    {
                        oVal += (int)  (pixelSource* (*itFilter) );
                        itFilter ++;
                    };
                p_rSource.for_each(fMultiplyFilter, x - oKernelSize, y - oKernelSize, 2*oKernelSize+1, 2* oKernelSize+1); 
                
                if (oVal <= ( (p_rFilter.size()-oNorm)*255))
                {
                    oLine[x] = (unsigned char)0;
                } else
                {
                    oLine[x] = (unsigned char)255;
                }
            }
            p_callbackOnLine (y);
        }
        return oRetImg;

    }
    
    // simple binarizer, setting all values below threshold to zero
    static precitec::image::BImage binarize(const precitec::image::BImage  & p_pImg, const int p_oThreshold )
    {   
        if (!p_pImg.isValid())
        {
            return {};
        }
        precitec::image::BImage outImage(p_pImg.size());
        p_pImg.transformTo(outImage, [&p_oThreshold] (byte p) {return p < p_oThreshold ? 0 : 255;});
        return outImage;
    }
};


}//end namespace
}


#endif
