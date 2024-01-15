#include "filter/samplingInformation.h"

namespace precitec {
	using namespace	image;
namespace filter {
    
    
SamplingInformation::SamplingFactor::SamplingFactor(int length, double contextSampling)
{
    assert(contextSampling != 0);
    if (contextSampling == 1.0)
    {
        m_type =  Type::Constant;
        m_factor = 1.0;
        m_border = 0.0;
    }
    else
    {
        if (contextSampling <1.0)
        {
            int originalLength = std::round(length / contextSampling);
            m_type =  Type::Downsampling;
            m_factor = originalLength / length;
            m_border = originalLength % length;
        }
        else
        {
            m_type = Type::Upsampling;
            m_factor = contextSampling ;
            m_border = 0 ;
        }
    }
}

int SamplingInformation::SamplingFactor::getReferenceLength(int length, BorderMode borderMode) const 
{
    int result = (m_type == Type::Downsampling) ? length * m_factor : length / m_factor;
    if (borderMode == BorderMode::included)
    {
        result += m_border;
    }
    return result;
}

double SamplingInformation::SamplingFactor::transformLengthToReferenceSpace(double lengthOnCurrentSampling) const
{
    double dFactor = m_factor;
    return (m_type == Type::Downsampling) ? lengthOnCurrentSampling * dFactor : lengthOnCurrentSampling / dFactor;
}

double SamplingInformation::SamplingFactor::transformLengthToCurrentSampling(double lengthOnReferenceSpace) const
{
    double dFactor = m_factor;
    return (m_type == Type::Downsampling) ? lengthOnReferenceSpace / dFactor : lengthOnReferenceSpace * dFactor;
}


SamplingInformation::SamplingInformation(const image::Size2d & rImageSize, double contextSamplingX, double contextSamplingY )
    :m_imageSize{rImageSize},
    m_contextSamplingX{contextSamplingX},
    m_contextSamplingY{contextSamplingY},
    m_samplingFactorX{m_imageSize.width, m_contextSamplingX},
    m_samplingFactorY{m_imageSize.height, m_contextSamplingY},
    m_offsetToReferenceImage{geo2d::Point{m_samplingFactorX.m_border/2, m_samplingFactorY.m_border/2}}
{
}


geo2d::DPoint SamplingInformation::transformToReferenceImage(double x_sampled, double y_sampled, BorderMode borderMode)  const
{
    assert(m_samplingFactorX.m_factor != 0);
    assert(m_samplingFactorY.m_factor != 0);
    double x = m_samplingFactorX.transformLengthToReferenceSpace(x_sampled);
    double y = m_samplingFactorY.transformLengthToReferenceSpace(y_sampled);
    if (borderMode == BorderMode::included)
    {
        x += m_offsetToReferenceImage.x;
        y += m_offsetToReferenceImage.y;
    }
    return geo2d::DPoint{x, y};
}

geo2d::DPoint SamplingInformation::transformToCurrentImage (double x_reference, double y_reference, BorderMode borderMode) const
{
    assert(m_samplingFactorX.m_factor != 0);
    assert(m_samplingFactorY.m_factor != 0);
    double x0 = (borderMode == BorderMode::included) ? x_reference - m_offsetToReferenceImage.x : x_reference;
    double y0 = (borderMode == BorderMode::included) ? y_reference - m_offsetToReferenceImage.y : y_reference;
    double x = m_samplingFactorX.transformLengthToCurrentSampling(x0);
    double y = m_samplingFactorY.transformLengthToCurrentSampling(y0);
    return {x, y};
    
}
    
image::Size2d SamplingInformation::getImageSize() const 
{
    return m_imageSize;
}

image::Size2d SamplingInformation::getReferenceImageSize(BorderMode borderMode)
{
    const auto & rSize = m_imageSize;
    return {m_samplingFactorX.getReferenceLength(rSize.width, borderMode), 
        m_samplingFactorY.getReferenceLength(rSize.height, borderMode)};
}
    
} // namespace filter
} // namespace precitec
