/**
*  @copyright: Precitec Vision GmbH & Co. KG
*  @author     MM
*  @date       2022
*  @file
*  @brief      Checks if ROI is on blank or (partly) on background. Output: 1 on blank, 2: not completely on blank
*/

#pragma once

#include <common/frame.h> // ImageFrame

#include "fliplib/Fliplib.h"
#include "fliplib/TransformFilter.h"
#include "fliplib/PipeEventArgs.h" // event processing
#include "fliplib/SynchronePipe.h" // in- / output

namespace precitec
{
namespace filter
{

class FILTER_API StartEndROIChecker : public fliplib::TransformFilter
{
public:
    StartEndROIChecker();
    ~StartEndROIChecker();

    void setParameter() override;
    void paint() override;

    bool subscribe(fliplib::BasePipe& pipe, int group) override;
    void proceedGroup(const void* sender, fliplib::PipeGroupEventArgs& event) override;

private:
    typedef geo2d::TPoint<int> Point;

    const fliplib::SynchronePipe<interface::GeoStartEndInfoarray>* m_startEndInfoIn;
    const fliplib::SynchronePipe<interface::ImageFrame>* m_imageIn;
    fliplib::SynchronePipe<interface::GeoDoublearray> m_onBlankOut;

    geo2d::StartEndInfo::FittedLine m_edge;
    Point m_roiStart;
    Point m_roiEnd;
    bool m_topMaterial;
    bool m_onBlank;

    int m_startEndTrafoX = 0;
    int m_startEndTrafoY = 0;

    enum Result
    {
        onMaterial = 1,
        partlyOnBackground
    };
};

}
}

