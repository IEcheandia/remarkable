#include "sparseKernel.h"

namespace precitec
{
namespace filter
{

SparseKernel::SparseKernel ()
{}
SparseKernel::SparseKernel (const image::BImage & rBmpImage)
{
    m_kernelSize.x = rBmpImage.width() / 2.0;
    m_kernelSize.y = rBmpImage.height() / 2.0;
    m_sum = 0;
    m_squareSum = 0;
    for ( int y = 0, h = rBmpImage.height(); y < h; y++)
    {
        auto * pPixel = rBmpImage.rowBegin(y);
        for ( int x = 0, w = rBmpImage.width(); x < w; x++, pPixel++)
        {
            if (*pPixel != 0)
            {
                m_pointlist.push_back({double(x),double(y),static_cast<double>(*pPixel)});
                if (m_pointlist.size() == 1)
                {
                    m_minCoord = {double(x), double(y)};
                    m_maxCoord = m_minCoord;
                }
                else
                {
                    m_minCoord.x = std::min(m_minCoord.x, double(x));
                    m_minCoord.y = std::min(m_minCoord.y, double(y));
                    m_maxCoord.x = std::min(m_maxCoord.x, double(x));
                    m_maxCoord.y = std::min(m_maxCoord.y, double(y));
                }
                m_sum += double(*pPixel);
                m_squareSum += (double(*pPixel) * double(*pPixel));
            }
        }
    }
    assert(m_sum > 0 || m_pointlist.empty());
}

SparseKernel SparseKernel::getNormalizedKernel() const
{
    SparseKernel result = *this;
    if (m_pointlist.empty())
    {
        assert(result.empty());
        return result;
    }
    result.m_sum = 0;
    result.m_squareSum = 0;
    for (auto && p : result.m_pointlist)
    {
        p.v /= this->m_sum;
        result.m_sum += p.v;
        result.m_squareSum += (p.v * p.v);
    }
    return result;
}


bool SparseKernel::empty() const
{
    return m_pointlist.empty();
}

image::DImage SparseKernel::crossCorrelation (const image::BImage & rImage, bool normalizeKernel) const
{
    return computeCrossCorrelation<false>(rImage, normalizeKernel);
}

image::DImage SparseKernel::normalizedCrossCorrelation (const image::BImage & rImage, bool normalizeKernel) const
{
    return computeCrossCorrelation<true>(rImage, normalizeKernel);
}

template<bool t_normalizedCrossCorrelation>
image::DImage SparseKernel::computeCrossCorrelation (const image::BImage & rImage, bool normalize) const
{

    if (normalize && m_sum != 1)
    {
        auto oNormalizedKernel = getNormalizedKernel();
        return oNormalizedKernel.normalizedCrossCorrelation(rImage, false);
    }

    image::DImage oCorrelationImage (rImage.size());
    oCorrelationImage.resizeFill(rImage.size(),0);
    if ( empty())
    {
        return oCorrelationImage;
    }


#ifndef NDEBUG
    //TODO find actual border
    int minYImage = rImage.height();
    int maxYImage = 0;
    int minXImage = rImage.width();
    int maxXImage = 0;
#endif
    auto oSumSquarePattern = m_squareSum;
    geo2d::Point kernelSize = m_kernelSize;

    for (int y = 0, yMax = rImage.height(); y < yMax; y++)
    {
        for (int x = 0,  xMax = rImage.width(); x < xMax; x++)
        {
            auto outX = x + kernelSize.x;
            auto outY = y + kernelSize.y;

            int skippedPixels = 0;
            double oSumProduct = 0;
            double oSumSquareInput = 0;
            for (auto & p : m_pointlist)
            {
                int i = std::round(p.x) + x;
                int j = std::round(p.y) + y;
                if ( i < 0 || i >= xMax || j < 0 || j >= yMax)
                {
                    skippedPixels ++;
                    continue;
                }
#ifndef NDEBUG
                minYImage = std::min(y, minYImage);
                maxYImage = std::max(y, maxYImage);
                minXImage = std::min(x, minXImage);
                maxXImage = std::max(x, maxXImage);
#endif
                if (skippedPixels > 0)
                {
                    continue;
                }
                assert(p.v != 0);
                double inputPixel =  rImage[j][i];
                oSumProduct += ( p.v * inputPixel);
                if (t_normalizedCrossCorrelation)
                {
                    oSumSquareInput += (inputPixel * inputPixel);
                }
            }
            if (skippedPixels > 0)
            {
                continue;
            }
            assert( oCorrelationImage.getValue(outX, outY) == 0);
            if (t_normalizedCrossCorrelation)
            {
                oCorrelationImage[outY][outX] =  oSumSquareInput != 0 ? oSumProduct/std::sqrt(oSumSquareInput*oSumSquarePattern): 0;
            }
            else
            {
                oCorrelationImage[outY][outX] =  oSumProduct;
            }

        }
    }
    return oCorrelationImage;


}

image::DImage SparseKernel::sumOfSquaredDifferences(const image::BImage & rImage ) const
{
    image::DImage oOutputImage (rImage.size());
    oOutputImage.resizeFill(rImage.size(),0);
    if ( empty())
    {
        return oOutputImage;
    }

    for (int y = 0, yMax = rImage.height(); y < yMax; y++)
    {
        for (int x = 0, xMax = rImage.width(); x < xMax; x++)
        {
            double oSumSquaredDifferences = 0; //without normalization
            bool skippedPixels = false;
            for (auto & p : m_pointlist)
            {
                int i = std::round(p.x) + x;
                int j = std::round(p.y) + y;
                if (i < 0 || i >= rImage.width() || j < 0 || j >= rImage.height())
                {
                    skippedPixels = true;
                    break;
                }
                auto & inputPixel =  rImage[j][i];
                auto oDiff = (inputPixel - p.v);
                oSumSquaredDifferences += (oDiff*oDiff);
            }
            assert( oOutputImage.getValue(x,y) == 0);
            if (skippedPixels)
            {
                continue;
            }
            oOutputImage[y][x] = oSumSquaredDifferences;
        }
    }
    return oOutputImage;


}

}
}
