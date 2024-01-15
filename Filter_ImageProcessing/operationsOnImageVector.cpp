#include "operationsOnImageVector.h"
#include "filter/algoStl.h"

namespace precitec {
namespace filter {
    
using namespace image;

OperationsOnImageVector::OperationsOnImageVector (int width, int height) :
m_x(0),
m_y(0),
m_w(width),
m_h(height)
{}

void OperationsOnImageVector::clear(int width, int height)
{
    m_x = 0;
    m_y = 0;
    m_w = width;
    m_h = height;
    m_Images.clear();
    m_ImagePixelsPointers.clear();
}


bool OperationsOnImageVector::insertImage(const BImage & rImage)
{
    if ( rImage.width() != m_w || rImage.height() != m_h)
    {
        return false;
    }
    m_IntermediateResults.reset();

    //BImage shallow copy
    m_Images.push_back(rImage);
    m_ImagePixelsPointers.push_back(rImage.rowBegin(m_y) + m_x);
    return true;
}


bool OperationsOnImageVector::prepareNextRow()
{
    ++m_y;
    m_x = -1;
    return m_y < m_h;
}

void OperationsOnImageVector::updateRowPointers()
{
    m_x = 0;
    assert (m_y != m_h);
    
    auto pPixelPointer = m_ImagePixelsPointers.data();
    for (auto && rImage: m_Images)
    {
        (*pPixelPointer) = rImage.rowBegin(m_y);
        ++pPixelPointer;
    }
}

void OperationsOnImageVector::incrementX()
{
    assert(m_x>=0 && " updateRowPointers was not called");
    ++m_x;
    
    const auto itEndImagePixelsPointers = m_ImagePixelsPointers.end();
    
    for (auto itImagePixelsPointers = m_ImagePixelsPointers.begin();
            itImagePixelsPointers != itEndImagePixelsPointers; ++itImagePixelsPointers)
    {
        ++(*itImagePixelsPointers);
    }
}

const std::vector<BImage> & OperationsOnImageVector::images() const
{
    return m_Images;
}

const std::vector<const byte *> & OperationsOnImageVector::imagePixelsPointers() const
{
    return m_ImagePixelsPointers;
}

int OperationsOnImageVector::w() const
{
    return m_w;
}
int OperationsOnImageVector::h() const
{
    return m_h;
}

std::size_t OperationsOnImageVector::numberOfImages() const
{
    assert(imagePixelsPointers().size() == m_Images.size());
    return m_Images.size();
}

template<Operations t_operation, typename T> inline
T OperationsOnImageVector::applyOnPixel() const
{
    static_assert(t_operation != Operations::ePixelMedian, "ePixelMedian needs another specialization");
    static_assert(t_operation != Operations::eRepeat, "eRepeat needs another specialization");
    static_assert(std::is_integral<T>::value , "Not an integral type");
    static_assert(
        (t_operation == Operations::ePixelSum || t_operation == Operations::ePixelDiff || t_operation == Operations::ePixelStd) 
        ||  std::is_same<T,byte>::value," the template argument  an unnecessary cast");
    
    assert(m_ImagePixelsPointers.size() > 0);
    
    auto itImagePixelsPointers = m_ImagePixelsPointers.begin();
    const auto itEndImagePixelsPointers = m_ImagePixelsPointers.end();
    
    const auto & pixelValue = (* (*itImagePixelsPointers)) ; //dereference the iterator (get a pointer), then dereference the pointer 
    
    T  result(pixelValue); 
    int resultInt(pixelValue);
    
    byte maxValue = result; //for ePixelRange , minValue is stored in result
    
    ++itImagePixelsPointers;
    
    constexpr bool bUseResultInt = !(std::is_same<T,int>::value); //avoid overflow/underflow when saving int to byte, clamp the results between 0 and 255
    
    for (; itImagePixelsPointers != itEndImagePixelsPointers; ++itImagePixelsPointers)
    {
        const auto & pixelValue = (* (*itImagePixelsPointers)) ; //dereference the iterator (get a pointer), then dereference the pointer 
        switch(t_operation)
        {
            case Operations::ePixelSum:
                if (bUseResultInt)
                {                    
                    resultInt += pixelValue;
                }
                else
                {
                    result += pixelValue;
                }
                break;
            case Operations::ePixelMean:
            case Operations::ePixelStd:
                resultInt += pixelValue;
                break;
            case Operations::ePixelDiff:
                if (bUseResultInt)
                {                    
                    resultInt -= pixelValue;
                }
                else
                {
                    result -= pixelValue;
                }
                break;
            case Operations::ePixelMax:
                result = pixelValue > result ? pixelValue : result;
                break;
            case Operations::ePixelRange:
                maxValue = pixelValue > maxValue ? pixelValue : maxValue;
                //fallthrough
            case Operations::ePixelMin:
                result = pixelValue < result? pixelValue : result;
                break;                                                
        }
    }
    
    switch(t_operation)
    {
        case Operations::ePixelMin:
        case Operations::ePixelMax:
            //do nothing
            break;
        case Operations::ePixelMean:
            result = resultInt / double(m_ImagePixelsPointers.size() );
            break;
        case Operations::ePixelSum:
        case Operations::ePixelDiff:
            if (bUseResultInt)
            {   
                resultInt = resultInt < std::numeric_limits<T>::max() ? resultInt : std::numeric_limits<T>::max();
                result = resultInt > std::numeric_limits<T>::min() ? resultInt : std::numeric_limits<T>::min();            
            }
            break;
        case Operations::ePixelStd:
        {                    
            //Two-pass algorithm
            //https://en.wikipedia.org/wiki/Algorithms_for_calculating_variance
            //in Excel (German) STABW.S, in Python numpy.std(vector,ddof=1)
            double sum2 = 0.0;
            double mean = double(resultInt) / double(m_ImagePixelsPointers.size());
            for ( const byte* pixel : m_ImagePixelsPointers )
            {
                double diffMean = double(*pixel) -mean;
                sum2 += (diffMean)*(diffMean);
            }
            result = static_cast<T>(sqrt(sum2 / double(m_ImagePixelsPointers.size() - 1)));
        }
        break;
        case Operations::ePixelRange:
            result = maxValue - result; 
            break;
    }    
    return result;
}

template<>
void OperationsOnImageVector::applyOnImage<Operations::ePixelMedian, byte> (image::TLineImage<byte> & rOutputImage)
{
    assert(rOutputImage.width() == w());
    assert(rOutputImage.height() == h());
    
    const int validValues = m_ImagePixelsPointers.size();
    std::vector<byte> oCachedPixels(validValues);
    
    //start iteration: move to first row
    m_y = 0;
    for( bool hasNextRow = m_y < m_h; hasNextRow; hasNextRow = prepareNextRow() ) 
    {
        updateRowPointers();        
        byte * pPixelOut=rOutputImage.rowBegin(m_y);
        
        for ( ; m_x < m_w; incrementX(), ++pPixelOut)
        {
            for ( int i = 0; i < validValues; ++i )
            {
                oCachedPixels[i] = *(m_ImagePixelsPointers[i]);
            }
            (*pPixelOut)  = *calcMedian(std::begin(oCachedPixels), std::end(oCachedPixels));
        } //for x
    }// for y
}

template<Operations t_operation, typename T> inline
void OperationsOnImageVector::applyOnImage(image::TLineImage<T> & rOutputImage)
{
    assert(rOutputImage.width() == w());
    assert(rOutputImage.height() == h());
    
    m_y = 0;    
    for( bool hasNextRow = m_y < m_h; hasNextRow; hasNextRow = prepareNextRow() ) 
    {
        updateRowPointers();
        T * pPixelOut=rOutputImage.rowBegin(m_y);
        
        for ( ; m_x < m_w; incrementX(), ++pPixelOut)
        {
            (*pPixelOut) = applyOnPixel<t_operation, T>();
        } // for x
    } // for y
}

template<>
void OperationsOnImageVector::applyOnImage<Operations::eRepeat, byte> (image::TLineImage<byte> & rOutputImage)
{
    assert(rOutputImage.width() == w());
    assert(rOutputImage.height() == h());
    
    //this is just a deep copy of the last(oldest) image
    
    auto & rCachedImage = m_Images.back();
    assert(rCachedImage.isContiguos() && rOutputImage.isContiguos() && "this application should work with contiguous images");
    
    rCachedImage.copyPixelsTo(rOutputImage);
         
}


template<>
void OperationsOnImageVector::applyOnImage<Operations::ePixelMedian, byte> (image::TLineImage<byte> & rOutputImage, byte & rMin, byte & rMax)
{
    assert(rOutputImage.width() == w());
    assert(rOutputImage.height() == h());
    
    rMin = 255;
    rMax = 0;
    
    const int validValues = m_ImagePixelsPointers.size();
    std::vector<byte> oCachedPixels(validValues);
    
    m_y = 0;
    for( bool hasNextRow = m_y < m_h; hasNextRow; hasNextRow = prepareNextRow() ) 
    {
        updateRowPointers();        
        byte * pPixelOut=rOutputImage.rowBegin(m_y);
        
        for ( ; m_x < m_w; incrementX(), ++pPixelOut)
        {
            for ( int i = 0; i < validValues; ++i )
            {
                oCachedPixels[i] = *(m_ImagePixelsPointers[i]);
            }
            (*pPixelOut)  = *calcMedian(std::begin(oCachedPixels), std::end(oCachedPixels));
            rMin = (*pPixelOut > rMin) ? rMin : (*pPixelOut);
            rMax = (*pPixelOut < rMax) ? rMax : (*pPixelOut);
        } //for x
    }//for y
}


template<Operations t_operation, typename T> 
void OperationsOnImageVector::applyOnImage(image::TLineImage<T> & rOutputImage, T & rMin, T & rMax)
{
    assert(rOutputImage.width() == w());
    assert(rOutputImage.height() == h());
    rMin = Max<T>::Value;
    rMax = Min<T>::Value;
    
    //start iteration: move to first row
    m_y = 0;
    for( bool hasNextRow = m_y < m_h; hasNextRow; hasNextRow = prepareNextRow() ) 
    {
        updateRowPointers();
        T * pPixelOut=rOutputImage.rowBegin(m_y);
        
        for ( ; m_x < m_w; incrementX(), ++pPixelOut)
        {
            (*pPixelOut) = applyOnPixel<t_operation, T>();
            rMin = (*pPixelOut > rMin) ? rMin : (*pPixelOut);
            rMax = (*pPixelOut < rMax) ? rMax : (*pPixelOut);
        } // for x
    } // for y
}

template<Operations t_operation> 
void  OperationsOnImageVector::applyOnImageAndStretchContrast(image::TLineImage<byte> & rOutput)
{
    // possible overflow, write intermediate results to another image before rescale
    auto & rIntermediateResultsImage = m_IntermediateResults.ref(t_operation, m_Images.size());
    rIntermediateResultsImage.resize(geo2d::Size(w(),h()));
    int minValue, maxValue;
    
    applyOnImage<t_operation, int>(rIntermediateResultsImage, minValue, maxValue);
    
    const double range = double(maxValue-minValue);
    
    assert(rIntermediateResultsImage.isContiguos() && rOutput.isContiguos());
    std::transform(rIntermediateResultsImage.begin(), rIntermediateResultsImage.end(), rOutput.begin(), 
                    [&range,&minValue](int p)
                    {
                        return (p - minValue)/range*255.0;
                    });
    
    assert(m_IntermediateResults.valid(t_operation, m_Images.size()));
    return;                    
}


//actually unused in imageArithmetic, because it is faster not to pass through imagesVector
template<> 
void  OperationsOnImageVector::applyOnImageAndStretchContrastInPlace<Operations::eRepeat>(image::TLineImage<byte> & rOutput)
{
    //repeat last (oldest) element (no operation to apply, just copy all pixels)
    applyOnImage<Operations::eRepeat, byte>(rOutput);
    
    assert(rOutput.isContiguos());
    auto bounds = std::minmax_element(rOutput.begin(), rOutput.end());
    byte minValue = *(bounds.first);
    byte maxValue = *(bounds.second);
    const double range(maxValue - minValue);
    std::transform(rOutput.begin(), rOutput.end(), rOutput.begin(), [&range, &minValue](byte v) {return (v-minValue)/range*255;});
}

template<Operations t_operation> 
void  OperationsOnImageVector::applyOnImageAndStretchContrastInPlace(image::TLineImage<byte> & rOutput)
{
    // rescale can happen in-place
    byte minValue, maxValue;
    applyOnImage<t_operation, byte>(rOutput, minValue, maxValue);
    
    const double range = double(maxValue-minValue);
    
    assert(rOutput.isContiguos());
    std::transform(rOutput.begin(), rOutput.end(), rOutput.begin(), 
                    [&range,&minValue](byte p)
                    {
                        return (p - minValue)/range*255.0;
                    });
    
}


bool OperationsOnImageVector::resultWithMovingWindowAvailable(Operations p_operation, bool stretchContrast, unsigned int expectedWindow) const
{
    if (!m_IntermediateResults.valid(p_operation,expectedWindow))
    {
        return false;
    }
    //check for supported operations
    if (p_operation == Operations::ePixelSum && stretchContrast)
    {
        return true;
    }
    return false;
}

bool OperationsOnImageVector::tryUpdateResultWithMovingWindow(image::TLineImage<byte> & rOutput,
                                                const image::BImage & rImageToAdd, Operations p_operation, bool stretchContrast, unsigned int expectedWindow)
{
    if (!m_IntermediateResults.valid(p_operation,expectedWindow))
    {
        return false;        
    }
    auto oSize = m_IntermediateResults.get().size();
    if (oSize != rImageToAdd.size())
    {
        return false;
    }    
    if (p_operation == Operations::ePixelSum && stretchContrast)
    {
        assert(resultWithMovingWindowAvailable(p_operation,stretchContrast, expectedWindow));
        
        int newWindow = expectedWindow + 1; //no image is removed, window grows
        auto & rIntermediateResultsImage = m_IntermediateResults.ref(p_operation, newWindow);
        
        {
            //clear variables not needed anymore
            m_Images.clear();
            m_ImagePixelsPointers.clear();
        }
        
        int minValue = Max<int>::Value;
        int maxValue = Min<int>::Value;
        
        for (int y = 0, h = rIntermediateResultsImage.height(); y < h; ++y)
        {
            int * pResult = rIntermediateResultsImage.rowBegin(y);
            const int * pResultEnd = rIntermediateResultsImage.rowEnd(y);
            const byte * pAdd = rImageToAdd.rowBegin(y);
            for (; pResult != pResultEnd; ++pResult, ++pAdd)
            {
                (*pResult) = (*pResult) + static_cast<int>(*pAdd);                    
                minValue = (*pResult > minValue) ? minValue : (*pResult);
                maxValue = (*pResult < maxValue) ? maxValue : (*pResult);
            }            
        }
                
        const double range = double(maxValue-minValue);
        
        std::transform(rIntermediateResultsImage.begin(), rIntermediateResultsImage.end(), rOutput.begin(), 
                [&range,&minValue](int p)
                {
                    return (p - minValue)/range*255.0;
                });
        assert(m_IntermediateResults.valid(p_operation,expectedWindow+1));
        return true;
    }
    
    //other operations not implemented
    assert(!resultWithMovingWindowAvailable(p_operation,stretchContrast, expectedWindow) && "list of supported operations in resultWithMovingWindowAvailable needs to be updated");
    return false;
    
}


bool OperationsOnImageVector::tryUpdateResultWithMovingWindow(image::TLineImage<byte> & rOutput, const image::BImage & rImageToRemove, 
                                                const image::BImage & rImageToAdd, Operations p_operation, bool stretchContrast, unsigned int expectedWindow)
{
    if (!m_IntermediateResults.valid(p_operation,expectedWindow))
    {
        return false;        
    }
    auto oSize = m_IntermediateResults.get().size();
    if (oSize != rImageToRemove.size() || oSize != rImageToAdd.size())
    {
        return false;
    }    
    if (p_operation == Operations::ePixelSum && stretchContrast)
    {
        auto & rIntermediateResultsImage = m_IntermediateResults.ref(p_operation, expectedWindow);
        
        {
            //clear variables not needed anymore
            m_Images.clear();
            m_ImagePixelsPointers.clear();
        }
        
        int minValue = Max<int>::Value;
        int maxValue = Min<int>::Value;
     
        for (int y = 0, h = rIntermediateResultsImage.height(); y < h; ++y)
        {
            int * pResult = rIntermediateResultsImage.rowBegin(y);
            const int * pResultEnd = rIntermediateResultsImage.rowEnd(y);
            const byte * pAdd = rImageToAdd.rowBegin(y);
            const  byte * pRemove = rImageToRemove.rowBegin(y);
            for (; pResult != pResultEnd; ++pResult, ++pAdd, ++pRemove)
            {
                (*pResult) = (*pResult) - static_cast<int>(*pRemove) + static_cast<int>(*pAdd);                    
                minValue = (*pResult > minValue) ? minValue : (*pResult);
                maxValue = (*pResult < maxValue) ? maxValue : (*pResult);
            }            
        }
                
        const double range = double(maxValue-minValue);
        
        std::transform(rIntermediateResultsImage.begin(), rIntermediateResultsImage.end(), rOutput.begin(), 
                [&range,&minValue](int p)
                {
                    return (p - minValue)/range*255.0;
                });
        assert(m_IntermediateResults.valid(p_operation,expectedWindow));
        return true;
    }
    
    //other operations not implemented
    return false;
    
}

void OperationsOnImageVector::computeResultImage(image::TLineImage<byte> & rOutput,Operations p_operation, bool stretchContrast)
{
    m_IntermediateResults.reset();
    
    if (stretchContrast)
    {
        switch(p_operation)
        {
            case Operations::ePixelSum:
                return applyOnImageAndStretchContrast<Operations::ePixelSum>(rOutput);
            case Operations::ePixelDiff:
                return applyOnImageAndStretchContrast<Operations::ePixelDiff>(rOutput);
            case Operations::ePixelMin:
                return applyOnImageAndStretchContrastInPlace<Operations::ePixelMin>(rOutput);
            case Operations::ePixelMax:
                return applyOnImageAndStretchContrastInPlace<Operations::ePixelMax>(rOutput);
            case Operations::ePixelStd:
                return applyOnImageAndStretchContrastInPlace<Operations::ePixelStd>(rOutput);
            case Operations::ePixelRange:
                return applyOnImageAndStretchContrastInPlace<Operations::ePixelRange>(rOutput);
            case Operations::ePixelMedian:
                return applyOnImageAndStretchContrastInPlace<Operations::ePixelMedian>(rOutput);
            case Operations::eRepeat:
                return applyOnImageAndStretchContrastInPlace<Operations::eRepeat>(rOutput);
            case Operations::eInvalid:
            case Operations::NumberValidOperations:
                //do nothing
                assert(false);
                break;               
            case Operations::ePixelMean:
                return applyOnImageAndStretchContrastInPlace<Operations::ePixelMean>(rOutput);
        }
    }
    else
        //no rescale
    {
        switch(p_operation)
        {
            case Operations::ePixelSum:
                return applyOnImage <Operations::ePixelSum, byte>(rOutput);
            case Operations::ePixelDiff:
                return applyOnImage<Operations::ePixelDiff, byte>(rOutput);
            case Operations::ePixelMin:
                return applyOnImage<Operations::ePixelMin, byte>(rOutput);
            case Operations::ePixelMax:
                return applyOnImage<Operations::ePixelMax, byte>(rOutput);
            case Operations::ePixelStd:
                return applyOnImage<Operations::ePixelStd, byte>(rOutput);
            case Operations::ePixelRange:
                return applyOnImage<Operations::ePixelRange, byte>(rOutput);
            case Operations::ePixelMedian:
                return applyOnImage<Operations::ePixelMedian, byte>(rOutput);
            case Operations::eRepeat:
                return applyOnImage<Operations::eRepeat, byte>(rOutput);
            case Operations::eInvalid:
            case Operations::NumberValidOperations:
                //do nothing
                assert(false);
                break;               
            case Operations::ePixelMean:
                return applyOnImage<Operations::ePixelMean, byte>(rOutput);
        }
    }
    
} //OperationsOnImageVector

} // namespace filter
} // namespace precitec
