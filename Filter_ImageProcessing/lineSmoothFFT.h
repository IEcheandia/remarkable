#pragma once

#include "fliplib/Fliplib.h"
#include "fliplib/TransformFilter.h"
#include "geo/geo.h"
#include "common/frame.h"

namespace precitec
{
namespace filter
{
/**
 * LineSmoothFFT is a low pass filter implemented with the Fast Fourier Transform
 **/
class FILTER_API LineSmoothFFT : public fliplib::TransformFilter
{
private:
    const fliplib::SynchronePipe<interface::GeoVecDoublearray>* m_lineIn;
    fliplib::SynchronePipe<interface::GeoVecDoublearray> m_lineOut;

    int m_maxFrequency;
    int m_ransacIteration;
    double m_ransacThreshold;

    interface::GeoVecDoublearray m_lineOutGeo;

public:
    LineSmoothFFT();

    void setParameter() override;
    void paint() override;

    bool subscribe(fliplib::BasePipe& pipe, int group) override;
    void proceedGroup(const void* sender, fliplib::PipeGroupEventArgs& event) override;
};

} //namespace filter
} //namespace precitec
