/*!
*  @copyright:  Precitec Vision GmbH & Co. KG
*  @author      MM
*  @date        2021
*  @brief       Binarizes an image depending on threshold.
*/

#pragma once

#include "fliplib/Fliplib.h"            // export macro
#include "fliplib/TransformFilter.h"    // base class
#include "fliplib/SynchronePipe.h"      // in- / output

#include "system/types.h"               // byte
#include "common/frame.h"               // ImageFrame
#include "filter/parameterEnums.h"      // ComparisonType

namespace precitec
{
namespace filter
{

/**
* @brief    Binarize filter. Binarizes an image depending on threshold.
* @details  Binarizes an 8-bit grey image depending on threshold to zero or 255. Not inplace.
*/
class FILTER_API BinarizeDynamicOnOff : public fliplib::TransformFilter
{

public:
    BinarizeDynamicOnOff();
    /**
    * @brief Set filter parameters.
    */
    void setParameter() override;

private:
    /**
    * @brief In-pipe registration.
    * @param p_rPipe Reference to pipe that is getting connected to the filter.
    * @param p_oGroup Group number (0 - proceed is called whenever a pipe has a data element, 1 - proceed is only called when all pipes have a data element).
    */
    bool subscribe(fliplib::BasePipe& p_rPipe, int p_oGroup) override;

    /**
    * @brief Processing routine.
    * @param p_pSender pointer to
    * @param p_rEvent
    */
    void proceedGroup(const void* sender, fliplib::PipeGroupEventArgs& e) override;

    /**
    * @brief Paints overlay.
    */
    void paint() override;

    typedef fliplib::SynchronePipe<interface::ImageFrame> image_pipe_t;
    typedef fliplib::SynchronePipe<interface::GeoDoublearray> threshold_pipe_t;

    const image_pipe_t*     m_pPipeInImageFrame;
    const threshold_pipe_t* m_pPipeInThreshold;

    image_pipe_t            m_oPipeOutImgFrame;

    ComparisonType          m_paramComparisonType;
    BinarizeType            m_paramBinarizeType;
    bool                    m_paramOnOff;

    interface::SmpTrafo     m_oSpTrafo;             ///< roi translation
    std::array<image::BImage, g_oNbParMax> m_binarizedImageOut;
};

}
}
