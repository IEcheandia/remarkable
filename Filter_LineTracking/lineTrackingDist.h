/**
 * @file
 * @copyright   Precitec Vision GmbH & Co. KG
 * @author      Michelle Meier
 * @date        2021
 * @brief       Laserline tracking filter, based on lineTrackingXT with additional allowence the vertical distance to the starting point of the laserline.
 */

#pragma once

#include <iostream>

#include "fliplib/Fliplib.h"			///< fliplib image processing pipes and filter framework
#include "fliplib/TransformFilter.h"	///< base class
#include "fliplib/PipeEventArgs.h"		///< event processing
#include "fliplib/SynchronePipe.h"		///< in- / output

#include "common/frame.h"
#include "geo/geo.h"

#include "laserlineTracker1.h"

#include "souvisSourceExportedTypes.h"

namespace precitec
{
namespace filter
{
class FILTER_API LineTrackingDist  : public fliplib::TransformFilter
{
public:
    LineTrackingDist();
    ~LineTrackingDist();

    static const std::string m_filterName;

    void setParameter() override;
    void paint() override;
    void arm(const fliplib::ArmStateBase& state) override;

protected:
    bool subscribe(fliplib::BasePipe& p_rPipe, int p_oGroup) override;
    void proceedGroup(const void* p_pSender, fliplib::PipeGroupEventArgs& p_rEventArg) override;

private:
    typedef fliplib::SynchronePipe<interface::ImageFrame> image_pipe_t;
    typedef fliplib::SynchronePipe<interface::GeoDoublearray> scalar_pipe_t;

    const image_pipe_t*     m_pPipeInImageFrame;        ///< in pipe Image
    const scalar_pipe_t*    m_pPipeInSearchThreshold;   ///< in pipe Search threshold
    const scalar_pipe_t*    m_pPipeInTrackStart;        ///< in pipe Track start
    const scalar_pipe_t*    m_pPipeInUpperMaxDiff;      ///< in pipe Max upper diff laserline to straight line
    const scalar_pipe_t*    m_pPipeInLowerMaxDiff;      ///< in pipe Max lower diff laserline to straight line

    fliplib::SynchronePipe<interface::GeoVecDoublearray>* m_pipeResY; //output: new structure Y with context

    geo2d::VecDoublearray m_oLaserlineOutY; ///< output laser line

    interface::SmpTrafo m_oSpTrafo; ///< roi translation

    LaserlineTracker1T m_laserlineTracker;

    Size2d m_imageSize;
    int m_threshold;
    int m_upperLower;
    int m_pixelX;
    int m_pixelY;
    int m_averagingY;
    int m_resolutionX;
    int m_resolutionY;
    int m_maxGapWidth;
    int m_maxNumberOfGaps;
    int m_maxLineJumpY;

    int m_startAreaX;
    int m_startAreaY;
};

}
}
