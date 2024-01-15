#pragma once

#include "fliplib/Fliplib.h"
#include "fliplib/TransformFilter.h"
#include "geo/geo.h"

namespace precitec
{
namespace filter
{

class FILTER_API ShiftContour : public fliplib::TransformFilter
{
public:
    ShiftContour();

    bool subscribe(fliplib::BasePipe& pipe, int group) override;
    void proceedGroup(const void* sender, fliplib::PipeGroupEventArgs& event) override;

private:
    const fliplib::SynchronePipe<interface::GeoVecAnnotatedDPointarray>* m_pipeContourIn;
    const fliplib::SynchronePipe<interface::GeoDoublearray>* m_pipeShiftX;
    const fliplib::SynchronePipe<interface::GeoDoublearray>* m_pipeShiftY;
    fliplib::SynchronePipe<interface::GeoVecAnnotatedDPointarray> m_pipeContourOut;

};

} //namspace filter
} //namespace precitec
