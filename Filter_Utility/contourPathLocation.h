#pragma once

#include "fliplib/Fliplib.h"
#include "fliplib/TransformFilter.h"
#include "geo/geo.h"
#include "common/frame.h"

namespace precitec
{
namespace filter
{

class FILTER_API ContourPathLocation : public fliplib::TransformFilter
{
private:
    const fliplib::SynchronePipe<interface::GeoVecAnnotatedDPointarray>* m_pipeContourIn;
    fliplib::SynchronePipe<interface::GeoDoublearray> m_pipePositionX;
    fliplib::SynchronePipe<interface::GeoDoublearray> m_pipePositionY;
    fliplib::SynchronePipe<interface::GeoDoublearray> m_pipeNormalX;
    fliplib::SynchronePipe<interface::GeoDoublearray> m_pipeNormalY;
    fliplib::SynchronePipe<interface::GeoDoublearray> m_pipeAngle;

    double m_relativePath;

public:
    ContourPathLocation();

    void setParameter() override;
    void paint() override;

    bool subscribe(fliplib::BasePipe& pipe, int group) override;
    void proceedGroup(const void* sender, fliplib::PipeGroupEventArgs& event) override;
};

} //namespace filter
} //namespace precitec
