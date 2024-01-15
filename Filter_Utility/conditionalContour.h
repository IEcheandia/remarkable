/**
 *  @file
 *  @copyrigh   Precitec GmbH & Co. KG
 *  @author     mm
 *  @date       2022
 *  @brief      This is a conditional contour filter. Similar to the simple conditional filter.
 *              Inputs: contour_a, contour_b, quality_a, quality_b
 *              Outputs: contour_out, compareResult
 *              Compare Mode:
 *                              if quality_a > quality_b then data_out == data_a and operationResult = 1
 *                              if quality_a < quality_b then data_out == data_b and operationResult = -1
 *                              if quality_a == quality_b then data_out == data_a and Result = 1
 */

#pragma once

// WM includes
#include "fliplib/Fliplib.h"
#include "fliplib/TransformFilter.h"
#include "fliplib/SynchronePipe.h"
#include <geo/geo.h>

#include "common/frame.h"               // ImageFrame
#include "image/image.h"                // BImage

namespace precitec
{
namespace filter
{

/**
 * @brief This is a conditional contour filter.
 */
class ConditionalContour : public fliplib::TransformFilter
{

private:
typedef fliplib::SynchronePipe< interface::GeoVecAnnotatedDPointarray >  contour_pipe_t;

public:
    ConditionalContour();
    ~ConditionalContour();
    static const std::string m_oFilterName;
    static const std::string m_oPipeOutDataName;
    static const std::string m_oPipeOutOperationResultName;
    void setParameter() override;
protected:
    bool subscribe(fliplib::BasePipe & p_rPipe, int p_oGroup) override;
    void proceedGroup(const void *p_pSender, fliplib::PipeGroupEventArgs & p_rEvent) override;
protected:
    const contour_pipe_t *m_pPipeInContourA;
    const contour_pipe_t *m_pPipeInContourB;
    const fliplib::SynchronePipe<interface::GeoDoublearray> *m_pPipeInQualityA;
    const fliplib::SynchronePipe<interface::GeoDoublearray> *m_pPipeInQualityB;

    fliplib::SynchronePipe<interface::GeoDoublearray> m_oPipeOutOperationResult;
    contour_pipe_t m_oPipeOutContour;
private:
    void proceedCompare();

};

}
}

