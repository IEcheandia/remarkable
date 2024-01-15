/**
 * 	@file
 * 	@copyright	Precitec Vision GmbH & Co. KG
 * 	@author		HS
 * 	@date		2016
 * 	@brief		Signal adapter for passing a pipe and a frame to a poco thread
 */

#ifndef SIGNALADAPTER_H_20160916_INCLUDED
#define SIGNALADAPTER_H_20160916_INCLUDED

#include <fliplib/SynchronePipe.h>

#include "Poco/Runnable.h"

#include "Mod_Analyzer.h"
#include "common/frame.h"
#include "analyzer/inspectTimer.h"
#include "analyzer/inspectManager.h"
#include "overlay/overlayCanvas.h"

namespace fliplib {
class FilterGraph;
}

namespace precitec {
namespace analyzer {
class ImageSender;
class ResultHandler;

class MOD_ANALYZER_API SignalAdapter: public Poco::Runnable
{
public:
	typedef fliplib::SynchronePipe< interface::ImageFrame >     ImageFramePipe;
    typedef fliplib::SynchronePipe< interface::SampleFrame >    SampleFramePipe;
    typedef std::map<int, interface::SampleFrame> 				Samples;

    SignalAdapter(
    	std::size_t							p_oIdxWorkerCur		= std::size_t{},
    	InspectTimer*						p_pInspectTimer		= nullptr,
        ImageFramePipe*                     p_pImageFramePipe   = nullptr, 
        SampleFramePipe*                    p_pSampleFramePipe  = nullptr,
        fliplib::FilterGraph*				p_pGraph			= nullptr);

    virtual void run();

    void setResultHandler(ResultHandler *handler)
    {
    	m_pResultHandler = handler;
    }

	void setImageNumber(std::size_t imageNumber)
	{
		m_imageNumber = imageNumber;
	}

    void setSamples(const Samples &p_oSamples)
    {
        m_oSamples = p_oSamples;
    }

    void setImage(const interface::ImageFrame &p_oImage)
    {
        m_oFrame = p_oImage;
    }

    void setProcessingMode(InspectManager::ProcessingMode mode)
    {
        m_processingMode = mode;
    }

    void setImageSensorId(int id)
    {
        m_imageSensorId = id;
    }

    void setOverlayCanvas(image::OverlayCanvas *canvas)
    {
        m_overlayCanvas = canvas;
    }

    void setVideoRecorder(TVideoRecorder<AbstractInterface> *recorder)
    {
        m_videoRecorder = recorder;
    }

    void setTriggerContext(const TriggerContext &context)
    {
        m_context = context;
    }

    void setImageSender(const std::weak_ptr<ImageSender> &sender)
    {
        m_imageSender = sender;
    }

    void setHardwareCamera(bool hasCamera)
    {
        m_hasHardwareCamera = hasCamera;
    }

    void setLastProcessedImage(int image)
    {
        m_lastProcessedImage = image;
    }

private:
    void sendData();
    void sendImageAndOverlay(std::vector<precitec::interface::SampleFrame> &&samples);
    void signalVideoRecorder(const std::vector<precitec::interface::SampleFrame> &samples);
    bool readFirstDataFromSensor(int & data, interface::Sensor sensorId) const;
    void handleScanmasterPosition();

    std::size_t							m_oIdxWorkerCur;			///< for timer
    InspectTimer*						m_pInspectTimer;			///< measures image- and sample-processing time, frame-to-frame time, overlay time
    interface::ImageFrame				m_oFrame;
    Samples								m_oSamples;
	ImageFramePipe* 					m_pImageFramePipe;
    SampleFramePipe* 					m_pSampleFramePipe;
    fliplib::FilterGraph*				m_pGraph;
    ResultHandler*     					m_pResultHandler;
	std::size_t							m_imageNumber;
    InspectManager::ProcessingMode      m_processingMode = InspectManager::ProcessingMode::Normal;
    int m_imageSensorId = 0;
    TVideoRecorder<AbstractInterface> *m_videoRecorder = nullptr;
    image::OverlayCanvas *m_overlayCanvas = nullptr;
    TriggerContext m_context;
    std::weak_ptr<ImageSender> m_imageSender;
    bool m_hasHardwareCamera = true;
    int m_lastProcessedImage = 0;
};

} // namespace analyzer
} // namespace precitec

#endif /* SIGNALADAPTER_H_20160916_INCLUDED */
