#ifndef SAMPLINGINFORMATION_H
#define SAMPLINGINFORMATION_H

#include <cassert>
#include <cmath>
#include "image/image.h"

namespace precitec {
namespace filter {

class SamplingInformation
{
public:

    enum class BorderMode {included, excluded};
    enum class Type{Downsampling, Upsampling, Constant};
    struct SamplingFactor
    {
        SamplingFactor(int length, double contextSampling);
        int getReferenceLength(int length, BorderMode borderMode) const; // length in the reference space (sampling =1) 
        double transformLengthToReferenceSpace(double lengthOnCurrentSampling) const;
        double transformLengthToCurrentSampling(double lengthOnReferenceSpace) const;
        
        Type m_type;
        int m_factor;
        int m_border;
    };

    SamplingInformation(const image::Size2d & rImageSize, double contextSamplingX, double contextSamplingY);   
    geo2d::DPoint transformToReferenceImage(double x_sampled, double y_sampled, BorderMode borderMode)  const;
    geo2d::DPoint transformToCurrentImage (double x_reference, double y_reference, BorderMode borderMode) const;
    image::Size2d getImageSize() const;
    image::Size2d getReferenceImageSize(BorderMode borderMode);
    
    const image::Size2d m_imageSize;
    const double m_contextSamplingX;
    const double m_contextSamplingY;
    const SamplingFactor m_samplingFactorX;
    const SamplingFactor m_samplingFactorY;
    const geo2d::Point m_offsetToReferenceImage;
};


} // namespace filter
} // namespace precitec

#endif
