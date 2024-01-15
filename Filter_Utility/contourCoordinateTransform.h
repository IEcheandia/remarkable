#pragma once

#include "fliplib/Fliplib.h"
#include "fliplib/TransformFilter.h"
#include "geo/geo.h"
#include "common/frame.h"

namespace precitec
{
namespace filter
{

class FILTER_API ContourCoordinateTransform : public fliplib::TransformFilter
{
private:
    const fliplib::SynchronePipe<interface::GeoVecAnnotatedDPointarray>* m_contourIn;
    fliplib::SynchronePipe<interface::GeoVecAnnotatedDPointarray> m_contourOut;

    enum Mode {
        PixelToWorld = 0,
        WorldToPixel = 1,
    };

    Mode m_conversionMode;
    bool m_logToFile;
    std::string m_logFileName;

public:
    ContourCoordinateTransform();

    void setParameter() override;
    void paint() override;

    bool subscribe(fliplib::BasePipe& pipe, int group) override;
    void proceedGroup(const void* sender, fliplib::PipeGroupEventArgs& event) override;

private:
    interface::GeoVecAnnotatedDPointarray m_contourOutArray;
};

} //namespace filter
} //namespace precitec
