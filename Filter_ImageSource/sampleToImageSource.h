#pragma once

#include "fliplib/Fliplib.h"
#include "fliplib/SourceFilter.h"
#include "fliplib/PipeEventArgs.h"
#include "fliplib/SynchronePipe.h"

#include "common/frame.h"
#include "geo/geo.h"

namespace precitec {
namespace filter {

class FILTER_API SampleToImageSource : public fliplib::SourceFilter
{
public:
    SampleToImageSource();

private:
    bool subscribe(fliplib::BasePipe&, int) override;
    void proceed(const void*, fliplib::PipeEventArgs&) override;

    const fliplib::SynchronePipe< interface::SampleFrame > *m_sampleIn;
    fliplib::SynchronePipe<interface::ImageFrame> m_pipeFrameOut;
};

} // namespace filter
} // namespace precitec
