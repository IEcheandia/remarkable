/**
 *  @file
 *  @copyrigh   Precitec GmbH & Co. KG
 *  @author     djb
 *  @date       2021
 *  @brief      This is a conditional line filter. Similar to the simple conditional filter.
 *              Inputs: line_a, line_b, quality_a, quality_b
 *              Outputs: line_out, compareResult
 *              Compare Mode:
 *                              if quality_a > quality_b then data_out == data_a and operationResult = 1
 *                              if quality_a < quality_b then data_out == data_b and operationResult = -1
 *                              if quality_a == quality_b then data_out == data_a and Result = 1
 */

#ifndef CONDITIONALLINE_H_
#define CONDITIONALLINE_H_

// WM includes
#include "fliplib/Fliplib.h"
#include "fliplib/TransformFilter.h"
#include "fliplib/SynchronePipe.h"
#include <geo/geo.h>

#include "common/frame.h"               // ImageFrame
#include "image/image.h"                // BImage

namespace precitec {
namespace filter {

/**
 * @brief This is a conditional line filter.
 */
class ConditionalLine : public fliplib::TransformFilter
{

private:
typedef fliplib::SynchronePipe< interface::GeoVecDoublearray >  line_pipe_t;

public:
    ConditionalLine();
    virtual ~ConditionalLine();
    static const std::string m_oFilterName;
    static const std::string m_oPipeOutDataName;
    static const std::string m_oPipeOutOperationResultName;
    void setParameter();
protected:
    bool subscribe(fliplib::BasePipe & p_rPipe, int p_oGroup);
    void proceedGroup(const void *p_pSender, fliplib::PipeGroupEventArgs & p_rEvent);
protected:
    const line_pipe_t *m_pPipeInLineA;
    const line_pipe_t *m_pPipeInLineB;
    const fliplib::SynchronePipe<interface::GeoDoublearray> *m_pPipeInQualityA;
    const fliplib::SynchronePipe<interface::GeoDoublearray> *m_pPipeInQualityB;

    fliplib::SynchronePipe<interface::GeoDoublearray> m_oPipeOutOperationResult;
    line_pipe_t m_oPipeOutLine;
private:
    void ProceedCompare();

}; // class ConditionalLine

} // namespace filter
} // namespace precitec

#endif // CONDITIONALIMAGE_H_
