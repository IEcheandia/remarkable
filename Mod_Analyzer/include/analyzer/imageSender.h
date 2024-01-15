#pragma once

#include "overlay/overlayCanvas.h"
#include "common/frame.h"
#include "event/recorder.interface.h"
#include "event/recorderPoll.interface.h"

#include <Poco/Mutex.h>

namespace precitec
{
namespace analyzer
{

class InspectTimer;

class ImageSender : public interface::TRecorderPoll<interface::AbstractInterface>
{
public:
    void setRecorder(interface::TRecorder<interface::AbstractInterface> *recorder)
    {
        m_recorder = recorder;
    }


    void setFrame(int sensorId, const interface::ImageContext &context, const image::BImage &data, const image::OverlayCanvas &canvas, std::vector<interface::SampleFrame> &&samples);

    void clear();

    void signalReadyForFrame() override;

    void setSimulation()
    {
        m_simulation = true;
    }

    void setHasFramegrabber(bool set)
    {
        m_hasFramegrabber = set;
    }

    void reserveForSamples(std::size_t expectedNumberOfSamples);

private:
    void sendImageAndOverlay();

    bool m_validData = false;
    int m_imageSensorId = 0;
    interface::ImageContext m_context;
    image::BImage m_image;
    image::OverlayCanvas m_overlayCanvas;
    interface::TRecorder<interface::AbstractInterface> *m_recorder = nullptr;
    Poco::FastMutex m_mutex;
    std::vector<interface::SampleFrame> m_samples;
    bool m_readyToSend = false;
    bool m_simulation = false;
    bool m_hasFramegrabber = false;
};

}
}
