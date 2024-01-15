/**
 * 	@file
 * 	@copyright	Precitec Vision GmbH & Co. KG
 * 	@author		HS
 * 	@date		2016
 * 	@brief		Signal adapter for passing a pipe and a frame to a poco thread
 */

#include "analyzer/signalAdapter.h"
#include "analyzer/graphAssistent.h"
#include "analyzer/resultHandler.h"
#include "analyzer/graphVisitors.h"
#include "analyzer/imageSender.h"

#include "common/frame.h"
#include "module/moduleLogger.h"
#include "system/timer.h"
#include "system/tools.h"
#include <util/calibDataSingleton.h>

namespace precitec {

using namespace interface;

namespace analyzer {

static const auto	g_oWinMaxFrameRate		= 15u;	// [Hz]
static const auto	g_oMinWinFrameDelta		= 1000u * 1000u / g_oWinMaxFrameRate ; // [Hz] to [us]

SignalAdapter::SignalAdapter(
		std::size_t							p_oIdxWorkerCur,
		InspectTimer*						p_pInspectTimer,
        ImageFramePipe*                     p_pImageFramePipe, 
        SampleFramePipe*                    p_pSampleFramePipe,
        fliplib::FilterGraph*				p_pGraph)
	:
    m_oIdxWorkerCur		{ p_oIdxWorkerCur },
    m_pInspectTimer		{ p_pInspectTimer },
	m_pImageFramePipe   { p_pImageFramePipe },
    m_pSampleFramePipe  { p_pSampleFramePipe },
    m_pGraph			{ p_pGraph },
    m_pResultHandler	{ nullptr },
	m_imageNumber		{ 0 }

{}

/*virtual*/ void SignalAdapter::run()
{
	try
	{
        if (m_overlayCanvas)
        {
            m_overlayCanvas->clearShapes();
        }
    
    std::cout << "SIGNAL_ADAPTER::run(); Starting Image Number: " << m_imageNumber << std::endl;
    wmLog(eDebug, "Signal adapter started \n");
        if (m_processingMode == InspectManager::ProcessingMode::OutOfOrderImage)
        { 
            std::cout << "SIGNAL_ADAPTER::run(); out of order image" << std::endl;
                system::ElapsedTimer imageTimer;
                wmLog(eError, "Processing mode overtriggered, inconsistency with image number: %d. \n", m_imageNumber);
                if (m_pInspectTimer)
                {
                    m_pInspectTimer->updateSkipImageProcessingTime(imageTimer.elapsed());
                }                
                return;
        }
        
        
        if (m_processingMode == InspectManager::ProcessingMode::Overtriggered 
            || m_processingMode == InspectManager::ProcessingMode::MissingImage)
        {
            std::cout << "SIGNAL_ADAPTER::run();  processingmode: overtriggered or missingmage" << std::endl;
            system::ElapsedTimer imageTimer;
            SkipImageProcessingVisitor visitor(m_lastProcessedImage + 1);
            auto filterGraph = m_pGraph->getFilterMap();
            for (auto it = filterGraph.begin(); it != filterGraph.end(); ++it)
            {
                visitor.control(*it->second->getFilter());
            }
            if (m_pResultHandler)
            {
                visitor.control(*m_pResultHandler);
            }
            GraphAssistent(m_pGraph).ensureImageNumber(m_imageNumber);
            if (m_pResultHandler)
            {
                m_pResultHandler->ensureImageNumber(m_imageNumber);
            }
            if ( m_processingMode != InspectManager::ProcessingMode::MissingImage)
            {
                assert(m_processingMode != InspectManager::ProcessingMode::OutOfOrderImage);
                sendData();
            }
            if (m_pInspectTimer)
            {
                m_pInspectTimer->updateSkipImageProcessingTime(imageTimer.elapsed());
            }
            if (m_processingMode == InspectManager::ProcessingMode::OutOfOrderImage)
            {
                return;
            }
        }
        
        // synchronize prior to start - ensure that all source filters of the graph reached their preSignalAction
        // otherwise it could happen that setting the sample input pipes overrides a previous setter
        std::cout << "SIGNAL_ADAPTER::run(); Synchronize the filters" << std::endl;
        system::ElapsedTimer imageTimer;
        GraphAssistent(m_pGraph).synchronizeSourceFilters(m_imageNumber);

        std::cout << "SIGNAL_ADAPTER::run(); Handle scanner position" << std::endl;
        handleScanmasterPosition();
 
        if (m_pResultHandler)
        {
            std::cout << "SIGNAL_ADAPTER::run(); Results start processing" << std::endl;
            m_pResultHandler->startProcessing();
        }
        
      //  wmLog(eDebug, "Starting calculations\n");
     //   std::cout << "Starting calculations... " << std::endl;
    

		if (m_pImageFramePipe->linked())
		{
            std::cout << "SIGNAL_ADAPTER::run(); Image pipe send frame" << std::endl;
			m_pImageFramePipe->signal(m_oFrame);
		}
        const auto imageProcessing = imageTimer.elapsed();

        system::ElapsedTimer sampleTimer;
        for (auto it = m_oSamples.begin(); it != m_oSamples.end(); it++)
        {
        	GraphAssistent(m_pGraph).setInputPipe(it->first, m_pSampleFramePipe);
        	m_pSampleFramePipe->signal(it->second);
        	m_pSampleFramePipe->uninstallAll();
        }
        std::cout << "SIGNAL_ADAPTER::run(); Samples sent frames" << std::endl;
        const auto sampleProcessing = sampleTimer.elapsed();
        
     //   wmLog(eDebug, "finishing calculations\n");
     //   std::cout << "Finishing calculations..." << std::endl;
     //   sleep(2);

		if (m_pResultHandler && m_pResultHandler->isProcessing())
		{
			// call proceedGroup manually on the result handler
			// the graph most likely has a SampleSource for which no signal is provided
			// by manually calling proceedGroup we ensure that the user results are transmitted
            std::cout << "SIGNAL_ADAPTER::run(); Result handler is processing" << std::endl;
			// first reset all PipeGroupEvents
			ResetGroupEventCounterVisitor visitor(m_imageNumber);
			m_pGraph->control(visitor);
			visitor.control(*m_pResultHandler);

			// dummy PipeGroupEventArgs as it's not used by the ResultHandler
			auto args = fliplib::PipeGroupEventArgs(0, nullptr, m_imageNumber);
			m_pResultHandler->proceedGroup(nullptr, args);
            poco_assert(!m_pResultHandler->isProcessing());

            // it is possible that some filter weren't processed. Thus we need to ensure the image number
            GraphAssistent(m_pGraph).ensureImageNumber(m_imageNumber + 1);
		}
        else if (!m_pResultHandler)
        {
            // special case filtertest which doesn't have a result handler, so unconditionally ensure image number
            GraphAssistent(m_pGraph).ensureImageNumber(m_imageNumber + 1);
        }

        system::ElapsedTimer sendDataTimer;
        sendData();
        std::cout << "SIGNAL_ADAPTER::run(); Sent data to video" << std::endl;
        const auto sendDataTime = sampleTimer.elapsed();

		if (m_pInspectTimer)
		{
            if (g_oDebugTimings)
            {
                wmLog(eDebug, "SignalAdapter image_number %d tot time %f \n", 
                  m_imageNumber, 
                  double(std::chrono::duration_cast<std::chrono::microseconds>(imageProcessing + sampleProcessing).count()) 
                 );
            }
            m_pInspectTimer->updateProcessingTime(imageProcessing, sampleProcessing, sendDataTime);
		}

        std::cout << "SIGNAL_ADAPTER::run(); Finished (Image number: " << m_imageNumber << ")" << std::endl;

	}
	catch(...) {
		system::logExcpetion(__FUNCTION__, std::current_exception());
		wmFatal(eInternalError, "QnxMsg.Fatal.InternalError", "Filter exception, inspection of parts is not possible anymore.\n");	// worker will starve
	} // catch
} // run

void SignalAdapter::sendData()
{
    std::vector<precitec::interface::SampleFrame> samples;
    samples.reserve(m_oSamples.size());
    std::transform(m_oSamples.begin(), m_oSamples.end(), std::back_inserter(samples),
        [] (const std::pair<int, SampleFrame> &pair)
        {
            return pair.second;
        }
    );
    signalVideoRecorder(samples);
    sendImageAndOverlay(std::move(samples));
}

void SignalAdapter::sendImageAndOverlay(std::vector<precitec::interface::SampleFrame> &&samples)
{
    auto imageSender = m_imageSender.lock();
    if (!imageSender || !m_overlayCanvas )
    {
        return;
    }
    imageSender->setFrame(m_imageSensorId, m_oFrame.context(), m_oFrame.data(), *m_overlayCanvas, std::move(samples));
}

void SignalAdapter::signalVideoRecorder(const std::vector<precitec::interface::SampleFrame> &samples)
{
    if (!m_videoRecorder || !m_pResultHandler)
    {
        return;
    }
    const auto	oNioReceived	=	m_pResultHandler->nioReceived(m_oIdxWorkerCur);	// check if current inspection signaled a nio.

    if (m_hasHardwareCamera && m_pImageFramePipe->linked())
    {
    	m_videoRecorder->data(m_imageSensorId, m_context, m_oFrame.data(), SeamData{m_context.getSeamSeriesNumber(), m_context.getSeamNumber()}, oNioReceived);
    }

    if (!samples.empty())
    {
        m_videoRecorder->multiSampleData(samples, SeamData{m_context.getSeamSeriesNumber(), m_context.getSeamNumber()}, oNioReceived);
    }
}


bool SignalAdapter::readFirstDataFromSensor(int & data, interface::Sensor sensorId) const
{
    auto itSample = m_oSamples.find(sensorId);
    if ( itSample == m_oSamples.end())
    {
        return false;
    }                            
    auto & rData = itSample->second.data();
    if (rData.getSize() > 0)
    {
        if (rData.getSize() > 1)
        {
            wmLog(eDebug, "Oversampling data (%d values) for sensor %d, using only first value \n", rData.getSize(), sensorId);
        }
        data = rData.data()[0];
        return true;
    }        
    return false;
}

void SignalAdapter::handleScanmasterPosition()
{
    // update calibration singleton
    if (g_oUseScanmasterPosition)
    {
        int scannerX_um = 0;
        int scannerY_um = 0;
        //TODO what to do when scanner positions are not found?
        
        bool valid = readFirstDataFromSensor(scannerX_um, eScannerXPosition)
            && readFirstDataFromSensor(scannerY_um, eScannerYPosition);
        

        if (valid)
        {
            auto &rCalibData = precitec::system::CalibDataSingleton::getCalibrationDataReference(precitec::math::SensorId::eSensorId0);            
            {
                //wmLog(eDebug, "CalibData, updateCorrection %d %d [um] imageNumber(%d)\n", scannerX_um, scannerY_um, m_imageNumber);
                auto & rScannerContextImage = m_oFrame.context().m_ScannerInfo;
                rScannerContextImage.m_hasPosition = true;
                rScannerContextImage.m_x = scannerX_um /1000.0; // from um to mm
                rScannerContextImage.m_y = scannerY_um /1000.0; // from um to mm
            
                if (rCalibData.hasCameraCorrectionGrid())
                {
                    rCalibData.updateCameraCorrection(rScannerContextImage.m_x, rScannerContextImage.m_y, (m_imageNumber % g_oNbPar)); //TODO: test with parallelization enabled
                }
                else
                {
                    wmLog(eWarning, "Scanner position received  %d %d , but calibration correction is not defined(image %d)\n", 
                    scannerX_um, scannerY_um, m_imageNumber);
                }
            }
            
            auto itIDMSample = m_oSamples.find(eIDMWeldingDepth);
            if (itIDMSample != m_oSamples.end())
            {
                auto & rScannerContextIDM = itIDMSample->second.context().m_ScannerInfo;
                rScannerContextIDM.m_hasPosition = true;
                rScannerContextIDM.m_x = scannerX_um /1000.0; // from um to mm
                rScannerContextIDM.m_y = scannerY_um /1000.0; // from um to mm
            }
            
        }             
        else
        {
            wmLog(eWarning, "Calibration correction requested, but no scanner position received (image %d) %d %d \n", 
                    m_imageNumber, scannerX_um, scannerY_um);
        }
    }
}
        
    

            
} // namespace analyzer
} // namespace precitec
