/*!
 *  @copyright: Precitec Vision GmbH & Co. KG
 *  @author     MM
 *  @date       2021
 *  @brief      Binarizes an image depending on threshold.
 */

#include "binarizeDynamicOnOff.h"

#include "image/image.h"
#include "overlay/overlayPrimitive.h"
#include "filter/algoImage.h"
#include "module/moduleLogger.h"

#include <fliplib/TypeToDataTypeImpl.h>


namespace precitec
{
namespace filter
{

extern void binarizeImage(const geo2d::Doublearray& thresholdIn, const image::BImage& imageIn, const ComparisonType& comparisonType,
                     const BinarizeType& binarizeType, image::BImage& rBinarizedImageOut);

BinarizeDynamicOnOff::BinarizeDynamicOnOff()
    : TransformFilter("BinarizeDynamicOnOff", Poco::UUID{"945258a7-8992-48c6-b637-b3b99e3825f7"})
    , m_pPipeInImageFrame (nullptr)
    , m_pPipeInThreshold (nullptr)
    , m_oPipeOutImgFrame (this, "ImageFrame")
    , m_paramComparisonType (eLess)
    , m_paramBinarizeType (BinarizeType::eGlobal)
    , m_paramOnOff (true)
{
    parameters_.add("ComparisonType", fliplib::Parameter::TYPE_int, static_cast<int>(m_paramComparisonType));
    parameters_.add("BinarizeType", fliplib::Parameter::TYPE_int, static_cast<int>(m_paramBinarizeType));
    parameters_.add("OnOffSwitch", fliplib::Parameter::TYPE_bool, static_cast<bool>(m_paramOnOff));

    setInPipeConnectors({{Poco::UUID("18f0d7cd-ebae-405a-89a6-cd649b355b1c"), m_pPipeInImageFrame, "Image", 1, "image"},
    {Poco::UUID("9b634ce7-03f8-407e-914f-d05d1b5edeea"), m_pPipeInThreshold, "Threshold", 1, "threshold"}});
    setOutPipeConnectors({{Poco::UUID("437efb66-acba-4a98-966f-ae83cdae817f"), &m_oPipeOutImgFrame, "ImageFrame", 0, ""}});
    setVariantID(Poco::UUID("1f0f4a8f-5107-4b11-af1f-69d7eae2b5b0"));
}

void BinarizeDynamicOnOff::setParameter()
{
    TransformFilter::setParameter();
    m_paramComparisonType = static_cast<ComparisonType>(parameters_.getParameter("ComparisonType").convert<int>());
    m_paramBinarizeType   = static_cast<BinarizeType>(parameters_.getParameter("BinarizeType").convert<int>());
    m_paramOnOff = parameters_.getParameter("OnOffSwitch").convert<bool>();
}

bool BinarizeDynamicOnOff::subscribe(fliplib::BasePipe& p_rPipe, int p_oGroup)
{
    if (p_rPipe.tag() == "image")
    {
        m_pPipeInImageFrame = dynamic_cast<image_pipe_t*>(&p_rPipe);
    }
    if (p_rPipe.tag() == "threshold")
    {
        m_pPipeInThreshold = dynamic_cast<threshold_pipe_t *>(&p_rPipe);
    }

    return BaseFilter::subscribe(p_rPipe, p_oGroup);
}

void BinarizeDynamicOnOff::proceedGroup(const void* sender, fliplib::PipeGroupEventArgs& e)
{
    // get data from frame
    const interface::ImageFrame& rFrameIn(m_pPipeInImageFrame->read(m_oCounter));

    const image::BImage& rImageIn (rFrameIn.data());
    const auto& rThresholdIn(m_pPipeInThreshold->read(m_oCounter).ref());

    m_oSpTrafo = rFrameIn.context().trafo();

    // input validity check
    if (rImageIn.isValid() == false || rThresholdIn.size() == 0)
    {
        // signal null image
        const interface::ImageFrame oNewFrame(rFrameIn.context(), image::BImage(), rFrameIn.analysisResult());
        preSignalAction();
        m_oPipeOutImgFrame.signal(oNewFrame);

        return;
    }

    auto& rBinarizedImageOut = m_binarizedImageOut[m_oCounter % g_oNbPar];
    // Just pass the input iamge to output if threshold is zero.
    const geo2d::Doublearray& threshold = m_paramOnOff ? rThresholdIn : geo2d::Doublearray(1, 0);
    binarizeImage(threshold, rImageIn, m_paramComparisonType, m_paramBinarizeType, rBinarizedImageOut);
    const interface::ImageFrame oFrameOut(rFrameIn.context(), rBinarizedImageOut, rFrameIn.analysisResult());
    preSignalAction();
    m_oPipeOutImgFrame.signal(oFrameOut);
}

void BinarizeDynamicOnOff::paint()
{
    if ((m_oVerbosity < eMedium) || m_oSpTrafo.isNull())
    {
        return;
    }

    const interface::Trafo &rTrafo     (*m_oSpTrafo);
    image::OverlayCanvas   &rCanvas    (canvas<image::OverlayCanvas>(m_oCounter));
    image::OverlayLayer    &rLayerImage(rCanvas.getLayerImage());

    const auto oPosition = rTrafo(geo2d::Point(0, 0));
    const auto oTitle    = image::OverlayText("Binarized image", image::Font(), geo2d::Rect(150, 18), image::Color::Black());

    rLayerImage.add<image::OverlayImage>(oPosition, m_binarizedImageOut[m_oCounter % g_oNbPar], oTitle);
}

}
}
