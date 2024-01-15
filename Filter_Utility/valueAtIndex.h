#pragma once

#include "fliplib/Fliplib.h"
#include "fliplib/TransformFilter.h"
#include "geo/geo.h"

namespace precitec
{
namespace filter
{

class FILTER_API ValueAtIndex : public fliplib::TransformFilter
{
private:
    const fliplib::SynchronePipe<interface::GeoDoublearray>* m_arrayIn;
    const fliplib::SynchronePipe<interface::GeoDoublearray>* m_indexIn;
    fliplib::SynchronePipe<interface::GeoDoublearray> m_valueOut;

public:
    ValueAtIndex();

    void setParameter() override;

    bool subscribe(fliplib::BasePipe& pipe, int group) override;
    void proceedGroup(const void* sender, fliplib::PipeGroupEventArgs& event) override;
};

} //namspace filter
} //namespace precitec
