#include "analyzer/imageSender.h"
#include "analyzer/inspectTimer.h"
#include <common/sample.h>

namespace precitec
{
namespace analyzer
{

void ImageSender::setFrame(int sensorId, const interface::ImageContext &context, const image::BImage &data, const image::OverlayCanvas &canvas, std::vector<interface::SampleFrame> &&samples)
{
    Poco::FastMutex::ScopedLock lock{m_mutex};
    m_imageSensorId = sensorId;
    m_image = data;
    m_overlayCanvas = canvas;
    m_context = context;
    if (!samples.empty())
    {
        std::move(samples.begin(), samples.end(), std::back_inserter(m_samples));
    }

    m_validData = !m_samples.empty() || m_hasFramegrabber || (m_image.width() != 0 && m_image.height() != 0);

    if (!m_readyToSend && !m_simulation)
    {
        return;
    }

    sendImageAndOverlay();
}

void ImageSender::sendImageAndOverlay()
{
    if (!m_recorder || !m_validData)
    {
        return;
    }

    bool allSamplesSend = true;
    if (!m_samples.empty())
    {
        const std::size_t sendAllSize{1000u};
        if (m_samples.size() < sendAllSize)
        {
            m_recorder->multiSampleData(m_samples, m_overlayCanvas);
            m_samples.clear();
        }
        else
        {
            allSamplesSend = false;
            std::vector<interface::SampleFrame> temp;
            temp.reserve(sendAllSize);
            auto end = m_samples.begin();
            std::advance(end, sendAllSize);
            std::move(m_samples.begin(), end, std::back_inserter(temp));
            m_samples.erase(m_samples.begin(), end);
            m_recorder->multiSampleData(temp, m_overlayCanvas);
        }
    }

    if (m_hasFramegrabber || (m_image.width() != 0 && m_image.height() != 0))
    {
        m_recorder->data(m_imageSensorId, m_context, m_image, m_overlayCanvas);
    }

    m_validData = !allSamplesSend;
    m_readyToSend = !allSamplesSend;
}

void ImageSender::clear()
{
    Poco::FastMutex::ScopedLock lock{m_mutex};
    m_samples.clear();
    m_validData = false;
}

void ImageSender::signalReadyForFrame()
{
    Poco::FastMutex::ScopedLock lock{m_mutex};
    if (m_validData)
    {
        sendImageAndOverlay();
    } else
    {
        m_readyToSend = true;
    }
}

void ImageSender::reserveForSamples(std::size_t expectedNumberOfSamples)
{
    Poco::FastMutex::ScopedLock lock{m_mutex};
    m_samples.reserve(expectedNumberOfSamples);
}

}
}
