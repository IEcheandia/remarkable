/**
 * 	@file
 * 	@copyright	Precitec Vision GmbH & Co. KG
 * 	@author		Simon Hilsenbeck (HS)
 * 	@date		2011
 * 	@brief		Manages and executes videoRecorder procedures.
 */

// local includes
#include "videoRecorder/videoRecorder.h"
#include "videoRecorder/nioModeType.h"
#include "videoRecorder/literal.h"
#include "system/tools.h"					///< poco_stdout_dbg, disk usage
#include "common/defines.h"					///< isDebug
#include "module/moduleLogger.h"
#include "filter/algoStl.h"
// stl includes
#include <iostream>
#include <sstream>

using namespace Poco;

namespace precitec {
	using namespace image;
	using interface::isDebug;

	using namespace interface;
namespace vdr {


VideoRecorder::VideoRecorder(grabberStatusProxy_t &p_rGabberStatusProxy,
		schedulerEventsProxy_t &p_rSchedulerEventsProxy ) :
	m_rGabberStatusProxy		( p_rGabberStatusProxy ),
	m_rSchedulerEventsProxy		( p_rSchedulerEventsProxy ),
	m_oProductType				( g_oLiveModeProdType ),
	m_oWriter					( p_rGabberStatusProxy, p_rSchedulerEventsProxy, m_oParameter ),
	m_oNioSeamOccured			( false ),
	m_oIsRecordCycleActive		( false ),
	m_oProductRecordSignaled	( true )
{} // VideoRecorder



void VideoRecorder::onAutomaticStart(const ProductInstData& p_rProductInstData) {
	Poco::FastMutex::ScopedLock oLock( m_oWriterMutex );
	m_oWriter.setIsLiveMode(false); // Live mode false means always automatic mode active.
	m_oProductRecordSignaled = false;
	m_oWriter.setProductInstData(p_rProductInstData);
	m_oProductType		= p_rProductInstData.m_oProductType;

	// make folder tree if enabled
	if (m_oParameter.isEnabled() == true) {
		m_oWriter.createDirectories();
	} // if
} // onAutomaticStart



void VideoRecorder::onAutomaticEnd() {
	Poco::FastMutex::ScopedLock oLock( m_oWriterMutex );
	if (m_oWriter.getIsLiveMode() == true) { // only erase if an automatic cycle has been started before and folders were created
		wmLogTr(eWarning, "QnxMsg.Vdr.ProcNonCritical", "A non-critcal  error occurred in the procedure '%s'. Message: '%s'. See log file.\n", __FUNCTION__, "Automatic cycle end signal received, but in live mode state.\n");
	} // if

	if (m_oWriter.getAutoFoldersCreated() == true) {
		m_oWriter.processRecord();
	} // if
	m_oWriter.setIsLiveMode(true);
	m_oProductType = g_oLiveModeProdType; // change to live mode default product
} // onAutomaticEnd



void VideoRecorder::onLivemodeStart(const ProductInstData& p_rProductInstData) {
	Poco::FastMutex::ScopedLock oLock( m_oWriterMutex );
	// m_oWriter.setProductInstData(makeLivemodeInstance()); is called on startup and on live mode end
	// createDirectories() is called on startup and on live mode end

	poco_assert_dbg(p_rProductInstData.m_oSeamData.empty() == false);
	m_oWriter.setFinalProductInstDirLiveAndProdInstId(
			p_rProductInstData.m_oProductInstId,
			p_rProductInstData.m_oSeamData.front().m_oTriggerDelta);
} // onLivemodeStart



void VideoRecorder::onLivemodeEnd() {
	Poco::FastMutex::ScopedLock oLock( m_oWriterMutex );
	m_oWriter.writeSequenceInfo(); // for automnatic mode on seamEnd
	m_oWriter.processRecord(); // for automatic mode done on automatic end.

	m_oWriter.setProductInstData(makeLivemodeInstance()); // prepare next live mode cycle
	m_oWriter.createDirectories(); // prepare next live mode cycle
} // onLivemodeEnd



void VideoRecorder::onSeamStart(interface::SeamData p_oSeamData) {
	Poco::FastMutex::ScopedLock oLock( m_oWriterMutex );

	m_oWriter.resetSeriesCounter();
	m_oWriter.resetInterruption();

	m_oIsRecordCycleActive	= true;
	m_oWriter.setSeamData(p_oSeamData);
} // onSeamStart



void VideoRecorder::onSeamEnd() {
	Poco::FastMutex::ScopedLock oLock( m_oWriterMutex );
	m_oIsRecordCycleActive	= false;

	// if we want to save nio seams check if a nio occured and then save

	if (m_oParameter.m_oNioMode.value() == eNioSeamOnly) { // we want to save nio seams.
		if (m_oNioSeamOccured == true) { // so check at seam end if a nio occured.
			while(m_oImageWriteCache.empty() == false) { // transfer the cached seam into the writer
				insertWriteQueue( m_oImageWriteCache.front() );
				m_oImageWriteCache.pop();
			} // while
			m_oNioSeamOccured = false; // reset flag
			// wait for writer queue

			int		oNbCyclesWaited			( 0 );
			while (m_oWriter.checkIfBusy() == true) {
				Thread::sleep(g_oSleepAfterNioSeam);
				++oNbCyclesWaited;
				if (oNbCyclesWaited >= g_oNbSleepAfterNioSeam) {
					wmLog(eDebug, "Not enough time for processing NIO sequence record.\n");
					wmLogTr(eWarning, "QnxMsg.Vdr.ProcException", "An error occurred in the procedure '%s'. Message: '%s'. See log file.\n", __FUNCTION__, "Not enough time for processing NIO sequence record.");
					break;
				} // if
			} // while

			Thread::sleep(g_oSleepAfterNioSeam); // wait a bit helps to prevent printing a wrong status or a wrong folder deletetion
		} // if
		else { // no nio occured, discard cache
			filter::clear(m_oImageWriteCache);
		} // else
	} // if

	if (m_oWriter.getAutoFoldersCreated() == true) {
		m_oWriter.writeSequenceInfo(); // only write if auto folders created. happens for live mode in 'onLivemodeEnd'
	} // if

	m_oWriter.printStatus();
} // seamEnd



KeyHandle VideoRecorder::set(SmpKeyValue p_oSmpKeyValue) {
	const auto &rKey		( p_oSmpKeyValue->key() );
	const auto oFound		( m_oParameter.m_oParamMap.find(rKey) );
	const auto oItMapEnd	( m_oParameter.m_oParamMap.end() );
	if (oFound != oItMapEnd) {
		if (p_oSmpKeyValue->isValueValid() == false) { // pre condition: min max are maintained by win side
			return -1; // value invalid, ignore it.
		}
		const Types oType	( oFound->second->type() );
		switch (oType) {
		case TBool: {
			const auto oValue	(p_oSmpKeyValue->value<bool>());

			if ( rKey != g_oIsEnabled)
			{
				oFound->second->setValue(oValue); // assign value
			}
			else
			{
                Poco::FastMutex::ScopedLock oLock( m_oWriterMutex );
				m_oParameter.isEnabled(oValue);

				if ( oValue == true && m_oWriter.getIsLiveMode() == false)
				{
					// if recording enabled during automatic mode, we need to create the seam folder
					m_oWriter.createDirectories();
				} // if
			}
			break;
		} // case
		case TInt: {
			const auto oValue	(p_oSmpKeyValue->value<int>());
			oFound->second->setValue(oValue); // assign value
			break;
		} // case
		case TUInt: {
			const auto oValue	(p_oSmpKeyValue->value<uint32_t>());
			oFound->second->setValue(oValue); // assign value

			// if nb of instances to store changes, update cache

			if (rKey == g_oNbProductsToKeep)
			{

				m_oWriter.updateCache(false);
			} // if
			else if (rKey == g_oNbLiveModeToKeep)
			{
				m_oWriter.updateCache(true);
			} // if

			break;
		} // case
		case TString:
			oFound->second->setValue(p_oSmpKeyValue->value<std::string>()); // assign value
			break;
		case TDouble:
			oFound->second->setValue(p_oSmpKeyValue->value<double>()); // assign value
			break;
		default:
			std::ostringstream oMsg;
			oMsg << __FUNCTION__ << ": invalid value type: " << oType;
			wmLog(eDebug, oMsg.str().c_str()); std::cerr << oMsg.str();
			wmLogTr(eWarning, "QnxMsg.Vdr.ProcNonCritical", "A non-critcal  error occurred in the procedure '%s'. Message: '%s'. See log file.\n", __FUNCTION__, oMsg.str().c_str());
			break;
		} // switch

		return oFound->second->handle(); // return handle
	} // if
	else {
		std::ostringstream oMsg;
		oMsg << __FUNCTION__ << ":Key '" << p_oSmpKeyValue->key() << "'NOT found.\n";
		wmLog(eDebug, oMsg.str());
		wmLogTr(eWarning, "QnxMsg.Vdr.ProcNonCritical", "A non-critcal  error occurred in the procedure '%s'. Message: '%s'. See log file.\n", __FUNCTION__, oMsg.str().c_str());
		return -1;
	} // else
} // set



SmpKeyValue VideoRecorder::get(Key p_oKey) const {
	const auto oFound		( m_oParameter.m_oParamMap.find(p_oKey) );
	const auto oItMapEnd	( m_oParameter.m_oParamMap.end() );
	if (oFound != oItMapEnd) {
		return oFound->second->clone();
	} // if
	else {
		std::ostringstream oMsg;
		oMsg << "ERROR" <<  __FUNCTION__ << ":Key '" << p_oKey << "'NOT found.\n";
		wmLog(eDebug, oMsg.str());
		wmLogTr(eWarning, "QnxMsg.Vdr.ProcNonCritical", "A non-critcal  error occurred in the procedure '%s'. Message: '%s'. See log file.\n", __FUNCTION__, oMsg.str().c_str());
		return new KeyValue;
	} // else
} // get



 Configuration VideoRecorder::getConfig() const {
	 //std::cerr << "\n" <<  __FUNCTION__ << "\t: called.\n"; // debug
	return makeConfiguration(m_oParameter, true);
 } // getConfig



void VideoRecorder::handleImage(int p_oSensorId, const TriggerContext &p_rTriggerContext, const BImage &p_rData, SeamData p_oSeamData, bool p_oNioReceived) {
	//const system::ScopedTimer	oTimer(__FUNCTION__);
	Poco::FastMutex::ScopedLock oLock( m_oWriterMutex );

	// check enabled status

	if (m_oParameter.isEnabled() == false) {
		return; // recording disabled - return
	} // if

	if (m_oIsRecordCycleActive == false) {  // occurs rarely if long inspection of last image or fast trigger
		wmLog(eDebug, "Incoming data discarded, because no recording cycle being active.\n");
		return; // discard data
	} // if

	// check nio occurence

	if (p_oNioReceived == true) {
		m_oNioSeamOccured	= true; // remember that a nio occured for this seam
		if (isDebug == true) {
			wmLog(eDebug, "NIO received.\n");
		} // if
	} // if

	// check limitation to nios

	const int			oImageNumber		( p_rTriggerContext.imageNumber() );
	const NioModeType	oNioModeType		( static_cast<NioModeType>( m_oParameter.m_oNioMode.value()) );

	switch (oNioModeType) {
	case eAllImages:
		insertWriteQueue(std::make_tuple(
				p_rData,
				oImageNumber,
				p_oSeamData,
				p_rTriggerContext.HW_ROI_x0,
				p_rTriggerContext.HW_ROI_y0,
				p_rData.imageId()));
		break;
	case eNioSeamOnly: // record only nio seams. just put image into cache.
		m_oImageWriteCache.push(std::make_tuple(	p_rData,
				oImageNumber,
				p_oSeamData,
				p_rTriggerContext.HW_ROI_x0,
				p_rTriggerContext.HW_ROI_y0,
				p_rData.imageId()));
		break;
	case eNioOnly:
		if (p_oNioReceived == true) { // only write if its a nio image
			insertWriteQueue(std::make_tuple(
					p_rData,
					oImageNumber,
					p_oSeamData,
					p_rTriggerContext.HW_ROI_x0,
					p_rTriggerContext.HW_ROI_y0,
					p_rData.imageId()));
		}
		break;
	default:
		std::ostringstream oMsg;
		oMsg << __FUNCTION__ << ": ERROR: invalid nio mode type: " << oNioModeType << "\n";
		wmLog(eDebug, oMsg.str());
		wmLogTr(eWarning, "QnxMsg.Vdr.ProcNonCritical", "A non-critcal  error occurred in the procedure '%s'. Message: '%s'. See log file.\n", __FUNCTION__, oMsg.str().c_str());
		break;
	} // switch
} // handleImage



void VideoRecorder::handleSample(int p_oSensorId, int oImageNumber, const Sample &p_rData, SeamData p_oSeamData, bool p_oNioReceived) {
	//const system::ScopedTimer	oTimer(__FUNCTION__);
	Poco::FastMutex::ScopedLock oLock( m_oWriterMutex );

	// check enabled status

	if (m_oParameter.isEnabled() == false) {
		return; // recording disabled - return
	} // if

	if (m_oIsRecordCycleActive == false) {  // occurs rarely if long inspection of last image or fast trigger
		wmLog(eDebug, "Incoming data discarded, because no recording cycle being active.\n");
		return; // discard data
	} // if

	// check nio occurence

	if (p_oNioReceived == true) {
		m_oNioSeamOccured	= true; // remember that a nio occured for this seam
		if (isDebug == true) {
			wmLog(eDebug, "NIO received.\n");
		} // if
	} // if

	// check limitation to nios

	const NioModeType	oNioModeType		( static_cast<NioModeType>( m_oParameter.m_oNioMode.value()) );

	switch (oNioModeType) {
	case eAllImages:
		insertWriteQueue(std::make_tuple(
				p_rData,
				oImageNumber,
				p_oSeamData,
				p_oSensorId));
		break;
	case eNioSeamOnly: // record only nio seams. just put image into cache.
		m_oSampleWriteCache.push(std::make_tuple(	p_rData,
				oImageNumber,
				p_oSeamData,
				p_oSensorId));
		break;
	case eNioOnly:
		if (p_oNioReceived == true) { // only write if its a nio image
			insertWriteQueue(std::make_tuple(
					p_rData,
					oImageNumber,
					p_oSeamData,
					p_oSensorId));
		}
		break;
	default:
		std::ostringstream oMsg;
		oMsg << __FUNCTION__ << ": ERROR: invalid nio mode type: " << oNioModeType << "\n";
		wmLog(eDebug, oMsg.str());
		wmLogTr(eWarning, "QnxMsg.Vdr.ProcNonCritical", "A non-critcal  error occurred in the procedure '%s'. Message: '%s'. See log file.\n", __FUNCTION__, oMsg.str().c_str());
		break;
	} // switch
} // handleSample



void VideoRecorder::uninitialize() {
	Poco::FastMutex::ScopedLock oLock( m_oWriterMutex );
	m_oWriter.uninitialize();
} // uninitialize


bool VideoRecorder::insertWriteQueue(const vdrImage_t &p_rVdrImgage) {
	if (m_oProductRecordSignaled == false && m_oWriter.getIsLiveMode() == false ) {
		//wmLog(eDebug, "Debug: Changed prod id: %s  inst id: %s  SIGNALED.", rProductId.toString().c_str(), rProductInstId.toString().c_str()); // DEBUG
		m_oProductRecordSignaled = true;
	} // if
	return m_oWriter.insertWriteQueue(p_rVdrImgage);
} // insertWriteQueue



bool VideoRecorder::insertWriteQueue(const vdrSample_t &p_rVdrSample) {
	if (m_oProductRecordSignaled == false && m_oWriter.getIsLiveMode() == false ) {
		//wmLog(eDebug, "Debug: Changed prod id: %s  inst id: %s  SIGNALED.", rProductId.toString().c_str(), rProductInstId.toString().c_str()); // DEBUG
		m_oProductRecordSignaled = true;
	} // if
	return m_oWriter.insertWriteQueue(p_rVdrSample);
} // insertWriteQueue


void VideoRecorder::deleteAutomaticProductInstances(const std::vector<std::string>& paths)
{
    for (const auto& path : paths)
    {
        m_oWriter.deleteProductInstance(false, path);
    }
}

void VideoRecorder::deleteLiveModeProductInstances(const std::vector<std::string>& paths)
{
    for (const auto& path : paths)
    {
        m_oWriter.deleteProductInstance(true, path);
    }
}

} // namespace vdr
} // namespace precitec
