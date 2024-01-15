#include "stencil.h"
#include <fliplib/TypeToDataTypeImpl.h>
#include <unistd.h>
#include <common/bitmap.h>
#include <overlay/overlayCanvas.h>
#include <overlay/overlayPrimitive.h>
#include <module/moduleLogger.h>

#define FILTER_ID "3f2bfb30-5246-4796-913b-372ff33775b3"
#define VARIANT_ID "7b02c289-a67e-400b-987c-da94821d2539"

#define PIPE_ID_IMAGEFRAMEIN "1f1e8907-5e48-4d94-af14-506fb4d02cab"
#define PIPE_ID_IMAGEFRAMEOUT "48f33dc7-ff35-436f-97fb-bb3ac3f96889"


namespace precitec
{
namespace filter
{

using fliplib::Parameter;

const std::string Stencil::m_filterName{"Stencil"};
const std::string Stencil::m_imageFrameInName{"ImageIn"};
const std::string Stencil::m_imageFrameOutName{"ImageOut"};

Stencil::Stencil()
    : TransformFilter{Stencil::m_filterName, Poco::UUID{FILTER_ID}}
    , m_pipeInImageFrame{nullptr}
    , m_pipeOutImageFrame{this, m_imageFrameOutName}
    , m_maskFileName{"config/mask.bmp"}
{
    parameters_.add("MaskFileName", Parameter::TYPE_string, m_maskFileName);
    unsigned int group = 1;
    setInPipeConnectors({
        {Poco::UUID{PIPE_ID_IMAGEFRAMEIN}, m_pipeInImageFrame, m_imageFrameInName, group, m_imageFrameInName}
    });
    group = 0;
    setOutPipeConnectors({
         {Poco::UUID{PIPE_ID_IMAGEFRAMEOUT}, &m_pipeOutImageFrame, std::string{m_imageFrameOutName}, group, std::string{m_imageFrameOutName}}
    });
    setVariantID(Poco::UUID{VARIANT_ID});
}

void Stencil::setParameter()
{
    TransformFilter::setParameter();
    const auto maskFileName = parameters_.getParameter("MaskFileName").convert<std::string>();
    if (maskFileName != m_maskFileName)
    {
        m_mask.clear();
    }
    m_maskFileName = maskFileName;
    wmLog(eDebug, "mask file name: %s\n", m_maskFileName);
}

void Stencil::paint()
{
    if (m_oVerbosity == eNone || !m_imageOut.isValid())
    {
        return;
    }

    if (m_oVerbosity >= eMedium)
    {
        const interface::Trafo &trafo (*m_trafo);
        const auto point = trafo(geo2d::Point(0, 0));
        auto &overlayCanvas(canvas<image::OverlayCanvas>(m_oCounter));
        auto &imageLayer(overlayCanvas.getLayerImage());
        imageLayer.add<image::OverlayImage>(point, m_imageOut, image::OverlayText());
    }
}

bool Stencil::subscribe(fliplib::BasePipe& pipe, int group)
{
    if (pipe.tag() == m_imageFrameInName)
    {
        m_pipeInImageFrame = dynamic_cast<fliplib::SynchronePipe<interface::ImageFrame>*>(&pipe);
    }
    return BaseFilter::subscribe(pipe, group);
}

void Stencil::proceedGroup(const void* sender, fliplib::PipeGroupEventArgs& event)
{
    auto isValid = true;
    const auto &frameIn = m_pipeInImageFrame->read(m_oCounter);
    const auto geoAnalysisResult = frameIn.analysisResult();
    m_trafo = frameIn.context().trafo();
    if (geoAnalysisResult != interface::AnalysisOK && m_pipeInImageFrame == nullptr)
    {
        isValid = false;
    }
    else
    {
        generateMask();
        if (!m_mask.isValid())
        {
            isValid = false;
            wmLog(eError, "Stencil: mask image is empty!\n");

        }
    }

    if (isValid)
    {
        const auto &imageIn = frameIn.data();
        if (imageIn.height() != m_mask.height() || imageIn.width() != m_mask.width())
        {
            isValid = false;
            wmLog(eError, "Stencil: size of the frame and mask image do not fit!\n");
        }
        else
        {
            m_imageOut = imageMasking(imageIn);
        }
    }
    if(!isValid || !m_imageOut.isValid())
    {
        interface::ImageFrame frameOut(frameIn.context(), image::BImage(), geoAnalysisResult); // signal null image
        preSignalAction();
        m_pipeOutImageFrame.signal(frameOut);
        return;
    }
    const interface::ImageFrame frameOut(frameIn.context(), m_imageOut, geoAnalysisResult); // put image into frame
    preSignalAction();
    m_pipeOutImageFrame.signal(frameOut);
}

image::BImage Stencil::imageMasking(const image::BImage& image) const
{
    image::BImage maskedImage{geo2d::Size{image.width(), image.height()}};
    image.copyPixelsTo(maskedImage);
    if (m_mask.isValid() && image.isValid())
    {
        for (int y = 0; y < image.height(); y++)
        {
            const auto& maskRow = m_mask[y];
            for (int x = 0; x < image.width(); x++)
            {
                if (maskRow[x] == 0)
                {
                    maskedImage[y][x] = 0;
                }
            }
        }
    }
    else
    {
        wmLog(eError, "Stencil: mask image is not valid\n");
    }
    return maskedImage;
}

void Stencil::generateMask()
{
    if (!m_mask.isValid())
    {
        const auto filePath = (getenv("WM_BASE_DIR") ? std::string(getenv("WM_BASE_DIR")) : "") +
            "/" + m_maskFileName;
        if (access(filePath.c_str(), F_OK) == -1)
        {
            wmLog(eError, "Stencil: cannot access mask file: " + filePath + "\n");
            return;
        }
        fileio::Bitmap bmp{filePath};
        image::BImage image{geo2d::Size(bmp.width(), bmp.height())};
        if (!bmp.load(image.begin()))
        {
            std::ostringstream oss;
            oss << "Stencil: error in loading bitmap file " << filePath << std::endl;
            oss << "Bitmap header: " << bmp << std::endl;
            wmLog(eError, oss.str());
            return;
        }
        m_mask = image;
    }
}

}
}
