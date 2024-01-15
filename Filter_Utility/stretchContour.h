#pragma once

#include "fliplib/Fliplib.h"
#include "fliplib/TransformFilter.h"
#include "geo/geo.h"

namespace precitec
{
namespace filter
{

class FILTER_API StretchContour : public fliplib::TransformFilter
{
public:
    StretchContour();

    bool subscribe(fliplib::BasePipe& pipe, int group) override;
    void proceedGroup(const void* sender, fliplib::PipeGroupEventArgs& event) override;

private:
    const fliplib::SynchronePipe<interface::GeoVecAnnotatedDPointarray>* m_pipeContourIn;
    const fliplib::SynchronePipe<interface::GeoDoublearray>* m_pipeScaleX;
    const fliplib::SynchronePipe<interface::GeoDoublearray>* m_pipeScaleY;
    fliplib::SynchronePipe<interface::GeoVecAnnotatedDPointarray> m_pipeContourOut;

};

} //namspace filter
} //namespace precitec
