/**
 * 	@file
 *  @copyright		Precitec Vision GmbH & Co. KG
 *  @author			HS
 *  @date			2014
 *  @brief			'InspectTimer' measures image- and sample-processing time, frame-to-frame time, overlay time.
 */

#include "analyzer/inspectTimer.h"
#include "common/defines.h"
#include "module/moduleLogger.h"
#include "Poco/ScopedLock.h"

#include <numeric>

namespace precitec {
namespace analyzer {

static const auto	g_oMaxNbOverTrigInARow	= 5u;


InspectTimer::InspectTimer() 
	:
	m_oNbOverTrigInARow				( 0 ),
	m_oNbImagesAccumulated			( 0 ),
	m_oNbSamplesAccumulated			( 0 ),
	m_oNbSendDataAccumulated		( 0 ),
    m_inspectManagerAccumulated(0),
    m_oNbFramesSkipped(0)
{} // InspectTimer


void InspectTimer::resetAccumulatedTimings() {
    Poco::ScopedLock<Poco::FastMutex> lock(m_updateProcessingTimeMutex);
    m_imageProcessing = std::chrono::nanoseconds::zero();
    m_sampleProcessing = std::chrono::nanoseconds::zero();
    m_inspectManager = std::chrono::nanoseconds::zero();
    m_sendData = std::chrono::nanoseconds::zero();
    m_lastImageTimes.clear();
    
    m_lastInspectManagerTime = std::chrono::nanoseconds::zero();
    m_currentPartialInspectManagerTime =  std::chrono::nanoseconds::zero();

	m_oNbImagesAccumulated	=	0;
	m_oNbSamplesAccumulated	=	0;
	m_oNbSendDataAccumulated	=	0;
    m_inspectManagerAccumulated = 0;
    m_oNbFramesSkipped = 0;
} // resetAccumulatedTimings



void InspectTimer::logAccumulatedTimings() {
	auto	oSampleProcTime	=	0.;
	auto	oImgProcTime	=	0.;
    auto	oOverlayTime	=	0.;

    wmLog(eInfo, "Total number of frames: %d , skipped: %d \n", m_oNbImagesAccumulated, m_oNbFramesSkipped);
    
    if (m_oNbImagesAccumulated != m_oNbSamplesAccumulated || m_oNbImagesAccumulated != m_inspectManagerAccumulated)
    {
        wmLog(eDebug, "Accumulated %d images %d samples %d inspect manager timings \n", m_oNbImagesAccumulated, m_oNbSamplesAccumulated, m_inspectManagerAccumulated);
    }
	if (m_oNbSamplesAccumulated != 0) {
		oSampleProcTime	=	std::chrono::duration<float, std::chrono::milliseconds::period>(m_sampleProcessing).count() / m_oNbSamplesAccumulated;
		wmLogTr(eInfo, "QnxMsg.Workflow.SampleProcTime",	
			"Mean processing time sample: %f ms.\n", oSampleProcTime);
	} // if
	if (m_oNbImagesAccumulated != 0) {
		oImgProcTime	=	std::chrono::duration<float, std::chrono::milliseconds::period>(m_imageProcessing).count() / m_oNbImagesAccumulated;
		wmLogTr(eInfo, "QnxMsg.Workflow.ImgProcTime",		
			"Mean processing time image: %f ms.\n",	oImgProcTime);
	} // if
	if (m_oNbSendDataAccumulated != 0) {
		oOverlayTime	=	std::chrono::duration<float, std::chrono::milliseconds::period>(m_sendData).count() / m_oNbSendDataAccumulated;
		wmLog(eInfo,
			"Mean sendData time: %f ms.\n",	oOverlayTime);
	} // if
	if (m_inspectManagerAccumulated != 0) {
        double inspectManagerTime = std::chrono::duration<float, std::chrono::milliseconds::period>(m_inspectManager).count() / m_inspectManagerAccumulated;
		wmLog(eInfo,
			"Mean waiting time: %f ms.\n",	inspectManagerTime);
		const auto	oTotalTime	=	oSampleProcTime + oImgProcTime + oOverlayTime + inspectManagerTime;
		wmLogTr(eInfo, "QnxMsg.Workflow.TotalTime",	
			"Mean processing time overlay included: %f ms.\n", oTotalTime);
	} // if
} // logAccumulatedTimings



// Output timings - warn user about overtriggering if not 1st img and trigger delta by > 1ms surpassed
InspectTimer::OvertriggeringResult InspectTimer::checkOvertriggering(int p_oTimeDelta, int p_oImageNumber, bool p_oConservative, int toleranceOvertriggering_us)
{

    Poco::ScopedLockWithUnlock<Poco::FastMutex> lock(m_updateProcessingTimeMutex);

	if (p_oImageNumber <= 5)
    { // checking the frame-to-frame time starting with the 5th image, as the first timings seem to fluctuate quite a bit.
		return OvertriggeringResult::EverythingOk;
	} // if

    const auto oSumLastImageTimes = std::accumulate(m_lastImageTimes.cbegin(), m_lastImageTimes.cend(), std::chrono::nanoseconds::zero());
    const auto oNumLastImageTimes = m_lastImageTimes.size();//starts from 1 and it goes to g_oNbPar

    //in order not to lose precision in the comparison with the "oSumLastImageTimes", from here all the times are multiplied by rNumLastImageTimes
    
    const auto meanInspect = std::chrono::duration<float, std::chrono::microseconds::period>(m_lastInspectManagerTime).count() * oNumLastImageTimes;
	const auto	oFrameTimeAccumulated	=	meanInspect + std::chrono::duration_cast<std::chrono::microseconds>(oSumLastImageTimes).count(); // [us] Frame-to-frame time (multiplied by rNumLastImageTimes)

	lock.unlock();
	//std::ostringstream oStFrameTime;
	//oStFrameTime << "check overtrigger - ImageNumber: " << p_oImageNumber << " - FrameTime: "<< oFrameTime <<" p_oTimeDelta: "<<p_oTimeDelta  <<".\n";
	//wmLog(eDebug, oStFrameTime.str());


	const auto oAccumulatedTimeDelta = p_oTimeDelta * oNumLastImageTimes;
	auto oMaxAcceptableAccumulatedTime = oAccumulatedTimeDelta * g_oNbPar + toleranceOvertriggering_us;
	if (p_oConservative)
	{	    
	    oMaxAcceptableAccumulatedTime = oAccumulatedTimeDelta * (g_oNbPar > 1 ? g_oNbPar -1 : 1);
	}
    
	if ( oFrameTimeAccumulated >  oMaxAcceptableAccumulatedTime)
    {
		++m_oNbOverTrigInARow;
        wmLog(eDebug, "Frame time avg  %f + %f [us] Nb Overtrigger in  a row %d\n",
        meanInspect/ float(oNumLastImageTimes) + (oFrameTimeAccumulated - meanInspect )/float(oNumLastImageTimes), m_oNbOverTrigInARow );
	} // if
	else
    {
		m_oNbOverTrigInARow	= 0;
	} // else

	// if we are over-triggered for n images, or if we have a frame-to-frame-time that is larger than 3 times the expected time delta.
	if (m_oNbOverTrigInARow > g_oMaxNbOverTrigInARow) 
	{
		return OvertriggeringResult::Overtriggered;
	} // if
	if (oFrameTimeAccumulated > (3*p_oTimeDelta * oNumLastImageTimes * g_oNbPar + (std::max(toleranceOvertriggering_us,1000) -1000) ))
	{
		return OvertriggeringResult::Overtriggered;
	}
	

    if (m_oNbOverTrigInARow > 0)
    {
        return OvertriggeringResult::Critical;
    }
    
    if (oFrameTimeAccumulated > oMaxAcceptableAccumulatedTime * 0.9)
    {
        if (oFrameTimeAccumulated > oMaxAcceptableAccumulatedTime * 0.95)
        {
            return OvertriggeringResult::Critical;
        }
        return OvertriggeringResult::Dangerous;
    }

	return OvertriggeringResult::EverythingOk;

} // checkOvertriggering

void InspectTimer::updateProcessingTime(const std::chrono::nanoseconds &imageProcessing, const std::chrono::nanoseconds &sampleProcessing, const std::chrono::nanoseconds &sendDataTime)
{
    Poco::ScopedLock<Poco::FastMutex> lock(m_updateProcessingTimeMutex);
    m_imageProcessing += imageProcessing;
    m_sampleProcessing += sampleProcessing;
    m_sendData += sendDataTime;

    m_oNbImagesAccumulated++;
    m_oNbSamplesAccumulated++;
    ++m_oNbSendDataAccumulated;

    m_lastImageTimes.emplace_back(imageProcessing + sampleProcessing);
    if (m_lastImageTimes.size() > g_oNbPar)
    {
        m_lastImageTimes.pop_front();
    }
    if (g_oDebugTimings)
    {
        wmLog(eDebug, "Last Processing Time [%d images, %d samples] = %f us\n", 
            m_oNbImagesAccumulated, m_oNbSamplesAccumulated, 
            double(std::chrono::duration<float, std::chrono::microseconds::period>(imageProcessing + sampleProcessing).count()) );
    }
}    

void InspectTimer::updateSkipImageProcessingTime(const std::chrono::nanoseconds & imageProcessing)
{
    
    Poco::ScopedLock<Poco::FastMutex> lock(m_updateProcessingTimeMutex);
    m_imageProcessing += imageProcessing;
    
    m_oNbImagesAccumulated++;
    m_oNbSamplesAccumulated++;
    m_oNbFramesSkipped++;
    

    m_lastImageTimes.emplace_back(imageProcessing);
    if (m_lastImageTimes.size() > g_oNbPar)
    {
        m_lastImageTimes.pop_front();
    }
    
    if (g_oDebugTimings)
    {
        wmLog(eDebug, "SkipImageProcessingTime [%d images, %d samples] = %f us\n", 
          m_oNbImagesAccumulated, m_oNbSamplesAccumulated, 
          double(std::chrono::duration<float, std::chrono::microseconds::period>(imageProcessing).count()) );
    }
}

double InspectTimer::processingTime() const
{
    auto oInspectManagerTime    = 0.;
	auto oSampleProcTime	    = 0.;
	auto oImgProcTime	        = 0.;
    auto oTotalTime             = 0.;

	if (m_oNbSamplesAccumulated != 0) {
		oSampleProcTime	=	std::chrono::duration<float, std::chrono::milliseconds::period>(m_sampleProcessing).count() / m_oNbSamplesAccumulated;
	} // if
	if (m_oNbImagesAccumulated != 0) {
		oImgProcTime	=	std::chrono::duration<float, std::chrono::milliseconds::period>(m_imageProcessing).count() / m_oNbImagesAccumulated;
	} // if
	if (m_inspectManagerAccumulated != 0) {
        oInspectManagerTime = std::chrono::duration<float, std::chrono::milliseconds::period>(m_inspectManager).count() / m_inspectManagerAccumulated;
	} // if
    
	oTotalTime	=	oSampleProcTime + oImgProcTime + oInspectManagerTime;
    return oTotalTime;
}

void InspectTimer::updateInspectManagerTime(const std::chrono::nanoseconds &time, bool increaseFrameCount )
{
    Poco::ScopedLock<Poco::FastMutex> lock(m_updateProcessingTimeMutex);
    
    m_inspectManager += time;
    
    if (increaseFrameCount)
    {
        {// check if the inspect manager time in the previous frame was particularly long
            double lastTime_us = std::chrono::duration<double, std::chrono::microseconds::period>(m_lastInspectManagerTime).count();
            if (lastTime_us > g_oDebugInspectManagerTime_us)
            {
                wmLog(eDebug, "updateInspectManagerTime at new frame: InspectManager at %d spent %f us waiting\n",
                    m_inspectManagerAccumulated, lastTime_us);
            }
        }
        m_inspectManagerAccumulated++;
        m_lastInspectManagerTime = time + m_currentPartialInspectManagerTime;
        m_currentPartialInspectManagerTime =  std::chrono::nanoseconds::zero();
    }
    else
    {
         m_currentPartialInspectManagerTime += time;
    }
}    

} // namespace analyzer
} // namespace precitec

