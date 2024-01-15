#pragma once

#include "fliplib/Fliplib.h"
#include "fliplib/TransformFilter.h"
#include "geo/geo.h"
#include "common/frame.h"

namespace precitec
{
namespace filter
{

enum IdmZCorrectionMode
{
    None = 0,
    VirtualPlane,
    PhysicalCalibrationSurface
};

class FILTER_API IdmZCorrection : public fliplib::TransformFilter
{
private:
    const fliplib::SynchronePipe<interface::GeoDoublearray>* m_pipeReferenceArm;
    const fliplib::SynchronePipe<interface::GeoDoublearray>* m_pipeIdmZ;
    fliplib::SynchronePipe<interface::GeoDoublearray> m_pipeCorrectZ;

    IdmZCorrectionMode m_mode;

public:
    IdmZCorrection();

    void setParameter() override;

    bool subscribe(fliplib::BasePipe& pipe, int group) override;
    void proceedGroup(const void* sender, fliplib::PipeGroupEventArgs& event) override;
};

} //namespace filter
} //namespace precitec
