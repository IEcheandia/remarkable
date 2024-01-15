/**
 * 	@file
 * 	@copyright	Precitec Vision GmbH & Co. KG
 * 	@author		Simon Hilsenbeck (HS)
 * 	@date		2012
 * 	@brief		Manages and executes image sequence writing.
 */

// local includes
#include "videoRecorder/writer.h"
#include "videoRecorder/literal.h"
#include "videoRecorder/fileCommand.h"
#include "videoRecorder/productInstanceMetaDataCommand.h"
#include "common/connectionConfiguration.h"
#include "message/device.h"					///< key value type
#include "system/tools.h"					///< poco_stdout_dbg, disk usage
#include "module/moduleLogger.h"
#include "filter/algoStl.h"
// poco includes
#include "Poco/File.h"
#include "Poco/Glob.h"
#include "Poco/DirectoryIterator.h"
#include "Poco/LocalDateTime.h"
#include "Poco/DateTimeFormatter.h"
#include "Poco/DateTimeParser.h"
#include "Poco/AutoPtr.h"
#include "Poco/Util/XMLConfiguration.h"
#include "Poco/UUIDGenerator.h"
// stl includes
#undef min
#undef max
#include <ctime>
#include <limits>
#include <set>
#include <vector>
#include <fstream>
#include <iterator>


namespace precitec {
	using image::BImage;
	using namespace Poco;
	using namespace interface;
namespace vdr {



Writer::Writer(grabberStatusProxy_t &p_rGabberStatusProxy, schedulerEventsProxy_t &p_rSchedulerEventsProxy, Parameter &p_rParameter) :
		m_oWmBaseDir				(getenv(g_oWmBaseDirEnv.c_str()) ? (std::string(getenv(g_oWmBaseDirEnv.c_str())) + "/") : g_oWmBaseDirFallback),
		m_rGabberStatusProxy		(p_rGabberStatusProxy),
		m_rSchedulerEventsProxy		(p_rSchedulerEventsProxy),
		m_rParameter				(p_rParameter),
		m_oConfigDir				(m_oWmBaseDir), // dirs pushed in constructor
		m_oStationDir				(m_oWmBaseDir), // dirs pushed in constructor
		m_oProductInstDirAuto		(""), // assigned on autmatic signal, when enabled, or on first image
		m_oProductInstDirLive		(""),
		m_oProductInstDirLiveFinal	(""),
		m_oIsLiveMode				(true),
		m_oAutoFoldersCreated		(false),
		m_oIsRecordInterrupted		(false),
		m_oFileCmdProcessor			(g_oPriorityVdr, g_oMaxQueueSize)
{
	// build xml configuration directory.

	m_oConfigDir.pushDirectory(g_oWmConfigDir);
	m_oConfigDir.setFileName(g_oWmConfigFileName);

	// build station directory and create it.

	m_oStationDir.pushDirectory(g_oWmVideoDir);
	m_oStationDir.pushDirectory( ConnectionConfiguration::instance().getString(g_oStationNameKey, g_oStationNameDefault) );
	Poco::File oStationFile(m_oStationDir);
	oStationFile.createDirectories(); // create basic recording directory

	// read xml-configuration and update cache depending on parameters

	parametrize();
	updateCache(true);
	updateCache(false);

	bool	oStationPathError		( false );
	if (! oStationFile.exists()) {
		std::ostringstream oMsg;
		oMsg << __FUNCTION__ << ": file doesnt exist: '" << oStationFile.path() << "'.\n";
		wmLog(eDebug, oMsg.str());
		oStationPathError = true;
	}
	if (! oStationFile.isDirectory()) {
		std::ostringstream oMsg;
		oMsg << __FUNCTION__ << ": is not a directory: '" << oStationFile.path() << "'.\n";
		wmLog(eDebug, oMsg.str());
		oStationPathError = true;
	}
	if (! oStationFile.canWrite()) {
		std::ostringstream oMsg;
		oMsg << __FUNCTION__ << ": file is not writable: '" << oStationFile.path() << "'.\n";
		wmLog(eDebug, oMsg.str());
		oStationPathError = true;
	}
	if (oStationPathError == true) {
		wmLogTr(eError, "QnxMsg.Vdr.InvalidFilePath", "Invalid file path: '%s' in procedure : '%s'. See log for details.\n", oStationFile.path().c_str(), __FUNCTION__);
	}

	// create new tmp directory for first live mode cycle. For late cycles done on live mode and.

	setProductInstData(makeLivemodeInstance());
	createDirectories();
} // Writer



void Writer::parametrize() {
	// fetch parameters from parsed xml.

	File oConfigFile(m_oConfigDir);
	if (oConfigFile.exists() == true) {
		AutoPtr<Util::XMLConfiguration> pConfIn;
		try { // poco syntax exception might be thrown or sax parse excpetion
			pConfIn	= new Util::XMLConfiguration(m_oConfigDir.toString());
		} // try
		catch(const Exception &p_rException) {
			wmLog(eDebug, "%s - '%s': %s\n", __FUNCTION__, p_rException.name(), p_rException.message().c_str());
			wmLog(eDebug, "Could not read parameters from file:\n");
			wmLog(eDebug, "'%s'.\n", m_oConfigDir.toString().c_str());
			wmLogTr(eWarning, "QnxMsg.Vdr.ProcNonCritical", "A non-critcal  error occurred in the procedure '%s'. Message: '%s'. See log file.\n", __FUNCTION__, p_rException.message().c_str());
		} // catch
		for(auto it(std::begin(m_rParameter.m_oParamMap)); it != std::end(m_rParameter.m_oParamMap); ++it) {
			//std::cout << __FUNCTION__ << "reading key:\n" << it->second->key() << "\n"; // debug
			const Types oType	( it->second->type() );
			try {
				switch (oType) {
				case TBool:
					it->second->setValue( pConfIn->getBool(		it->second->key(), it->second->defValue<bool>() 		) ); // assign value
					break;
				case TInt:
					it->second->setValue( pConfIn->getInt(		it->second->key(), it->second->defValue<int>() 			) ); // assign value
					break;
				case TUInt: // NOTE: theres is no 'setUInt()' or 'getUInt()', thus needs cast, because written with 'setInt()'
					it->second->setValue(
							static_cast<uint32_t>(pConfIn->getInt(
									it->second->key(), it->second->defValue<uint32_t>()))); // assign value
					break;
				case TString:
					it->second->setValue( pConfIn->getString(	it->second->key(), it->second->defValue<std::string>() 	) ); // assign value
					break;
				case TDouble:
					it->second->setValue( pConfIn->getDouble(	it->second->key(), it->second->defValue<double>() 		) ); // assign value
					break;
				default:
					std::ostringstream oMsg;
					oMsg << "Invalid value type: '" << oType << "'\n";
					wmLogTr(eWarning, "QnxMsg.Vdr.ProcNonCritical", "A non-critcal  error occurred in the procedure '%s'. Message: '%s'. See log file.\n", __FUNCTION__, oMsg.str().c_str());
					break;
				} // switch
			} // try
			catch(const Exception &p_rException) {
				wmLog(eDebug, "%s - '%s': %s\n", __FUNCTION__, p_rException.name(), p_rException.message().c_str());
				std::ostringstream oMsg;
				oMsg << "Parameter '" << it->second->key().c_str() << "' of type '" << oType << "' could not be converted. Reset to default value.\n";
				wmLog(eDebug, oMsg.str());
				wmLogTr(eWarning, "QnxMsg.Vdr.ProcNonCritical", "A non-critcal  error occurred in the procedure '%s'. Message: '%s'. See log file.\n", __FUNCTION__, oMsg.str().c_str());
			} // catch
		} // for

	} // if
	else {
		std::ostringstream oMsg;
		oMsg << "Configuration file '" << m_oConfigDir.toString() << "' not found.\n";
		wmLog(eDebug, oMsg.str()); oMsg.str("");
		oMsg << "Using default parameters. File will be created on exit.\n";
		wmLog(eDebug, oMsg.str());
	} // else

	invalidToDefault(m_rParameter);

	//print(m_rParameter);

	// provide current disk capacity

	const double oCurrentDiskUsage	= getDiskUsage("./");
	std::ostringstream oMsg;
	oMsg << std::setprecision(g_oPrecisionDiskUsage) << oCurrentDiskUsage * 100;
	wmLogTr(eInfo, "QnxMsg.Vdr.DiskUsage", "Current disk usage is %s%%.\n", oMsg.str().c_str());
} // parametrize



void Writer::setProductInstData(const ProductInstData &p_rProductInstData) {

	// if live mode set temporary live mode product folder

	if (m_oIsLiveMode == true) {
		m_oProductInstDataLive	= p_rProductInstData;
		m_oProductInstDirLive		= m_oStationDir; // create product folders within station directory

		const std::string	oNow				( DateTimeFormatter::format(LocalDateTime(), g_oDateTimeFormatLong) ); // "_061022-185505123" http://pocoproject.org/docs/Poco.DateTimeFormatter.html
		const std::string	oTmpLiveModeDirName	( g_oLiveModeDefaultDirName + oNow );

		m_oProductInstDirLive.pushDirectory( g_oLiveModeName );
		m_oProductInstDirLive.pushDirectory( oTmpLiveModeDirName );
	} // if
	else {
		m_oProductInstData	= p_rProductInstData;
		m_oProductInstDirAuto	= m_oStationDir; // create product folders within station directory
		m_oProductInstDirAuto.pushDirectory(m_oProductInstData.m_oProductId.toString());
		const std::string test = "-SN-" + std::to_string(m_oProductInstData.m_oProductNr);
		const std::string newGenerationGuid = "." + m_oProductInstData.m_oProductInstId.toString() + test;
		m_oProductInstDirAuto.pushDirectory(newGenerationGuid);
	} // else
	m_startTime = std::chrono::system_clock::now();

} // setProductInstData



const interface::ProductInstData& Writer::getProductInstData() const {
	return m_oProductInstData;
} // getProductInstData



void Writer::setFinalProductInstDirLiveAndProdInstId(const UUID& p_rProductInstId, int p_oTriggerDelta) {
	if (m_oIsLiveMode == false) {
		poco_assert_dbg("'Writer::setFinalProductInstDirLive' only in live mode supported.\n");
		return;
	} // if

	m_oProductInstDirLiveFinal		= m_oStationDir; // create product folders within station directory
	m_oProductInstDirLiveFinal.pushDirectory( g_oLiveModeName );
	m_oProductInstDirLiveFinal.pushDirectory( p_rProductInstId.toString() + "-SN-" + Poco::DateTimeFormatter::format(Poco::Timestamp{}, std::string("%Y%m%d%H%M%S")) );

	m_oProductInstDataLive.m_oProductInstId						=	p_rProductInstId; 	// replace present null id, now that id is known
	m_oProductInstDataLive.m_oSeamData.front().m_oTriggerDelta	=	p_oTriggerDelta; 	// replace present trigger delta, now that trigger delta is known

	auto 		oProductInstIdFile		=	Path		{ m_oProductInstDirLive };
	oProductInstIdFile.setFileName(UUID::null().toString());
	oProductInstIdFile.setExtension(g_oIdExtension);
	auto 		oNewProductInstIdFile	=	oProductInstIdFile;
	oNewProductInstIdFile.setBaseName(p_rProductInstId.toString());
	const auto oEnqueued = m_oFileCmdProcessor.pushBack( upBaseCommand_t(new RenameCmd(oProductInstIdFile, oNewProductInstIdFile)), parameterToEnqueueMode()); // rename preset null id file, now that id is known
	if (oEnqueued == false)	// worker queue full -> full stop
	{
		m_oIsRecordInterrupted = true;
		m_rParameter.isEnabled(false);
	}
} // setFinalProductInstDirLiveAndProdInstId



void Writer::setSeamData(SeamData p_oSeamData) {
	m_oSeamData = p_oSeamData;

	Path& 				rProductInstDir		( m_oIsLiveMode ? m_oProductInstDirLive : m_oProductInstDirAuto );
	m_oSeamDir	= rProductInstDir;  // set seam folders within product directory

	// append current seamseries and seam number to product directory

	m_oSeamDir.pushDirectory( makeEnumeratedRepoDirName(g_oSeamSeriesDirName, m_oSeamData.m_oSeamSeries) );
    int seamNumber = m_oSeamData.m_oSeam;
    bool createDir = false;
    if (!p_oSeamData.m_oSeamLabel.empty())
    {
        try
        {
            seamNumber = std::stoi(p_oSeamData.m_oSeamLabel);
            createDir = true;
        } catch (...)
        {
        }
    }
	m_oSeamDir.pushDirectory( makeEnumeratedRepoDirName(g_oSeamDirName, seamNumber) );
    if (!m_oIsLiveMode && createDir)
    {
        createDirectory(m_oSeamDir);
    }
} // setSeamData

void Writer::createDirectory(const Poco::Path &path)
{
    const auto oEnqueued = m_oFileCmdProcessor.pushBack( upBaseCommand_t(new CreateDirsCmd(path)), parameterToEnqueueMode());
    if (oEnqueued == false)	// worker queue full -> full stop
    {
        m_oIsRecordInterrupted = true;
        m_rParameter.isEnabled(false);
    }
}

void Writer::createDirectories() {
	if (m_oIsLiveMode == false && m_oAutoFoldersCreated == true) {
		wmLog(eDebug, "Automatic cycle folders have already been created - skipping.\n");
		return;
	} // if

	Path 				oProductInstDir		( m_oIsLiveMode ? m_oProductInstDirLive : m_oProductInstDirAuto );

	// check product direcoty depth

	if (oProductInstDir.depth() < 4) { // /video_share/station/prodname/prodnumber/ - 4 levels should exist
		wmLogTr(eError, "QnxMsg.Vdr.InvalidFilePath", "Invalid file path: '%s' in procedure : '%s'. See log for details.\n", oProductInstDir.toString().c_str(), __FUNCTION__);
	} // if

	// make folder tree for video recorder

	ProductInstData& rProductInstData	( m_oIsLiveMode ? m_oProductInstDataLive : m_oProductInstData );
	for(auto oItSeamData(std::begin( rProductInstData.m_oSeamData) ); oItSeamData != std::end(rProductInstData.m_oSeamData); ++oItSeamData){
		Poco::Path		oSeamDir	= oProductInstDir;  // create seam folders within product directory

		oSeamDir.pushDirectory( makeEnumeratedRepoDirName(g_oSeamSeriesDirName, oItSeamData->m_oSeamSeries) );
		oSeamDir.pushDirectory( makeEnumeratedRepoDirName(g_oSeamDirName, oItSeamData->m_oSeam) );

		createDirectory(oSeamDir);
	} // for

	// create product id and product inst id files

	auto	oProductInstIdFile	=	oProductInstDir;
	auto	oProductIdFile		=	oProductInstDir.popDirectory();

	oProductInstIdFile.setFileName(rProductInstData.m_oProductInstId.toString());
	oProductIdFile.setFileName(rProductInstData.m_oProductId.toString());
	oProductInstIdFile.setExtension(g_oIdExtension);
	oProductIdFile.setExtension(g_oIdExtension);

	auto oEnqueued = m_oFileCmdProcessor.pushBack( upBaseCommand_t(new CreateFileCmd(oProductInstIdFile)), parameterToEnqueueMode());
	if (oEnqueued == false)	// worker queue full -> full stop
	{
		m_oIsRecordInterrupted = true;
		m_rParameter.isEnabled(false);
	}
	oEnqueued = m_oFileCmdProcessor.pushBack( upBaseCommand_t(new CreateFileCmd(oProductIdFile)), parameterToEnqueueMode());
	if (oEnqueued == false)	// worker queue full -> full stop
	{
		m_oIsRecordInterrupted = true;
		m_rParameter.isEnabled(false);
	}

	if (m_oIsLiveMode == false) {
		m_oAutoFoldersCreated	= true;
	} // if
} // createDirectories



bool Writer::insertWriteQueue(const vdrImage_t &p_rVdrImage) {
	const std::string		oFilePath			( makeFilePath(std::get<vdr_image_type::eImageNumber>(p_rVdrImage), g_oImageExtension) );
	upBaseCommand_t			oUpBaseCommand		( new WriteImageCmd(
														oFilePath,
														p_rVdrImage,
														m_rParameter,
														m_rGabberStatusProxy,
														m_oSeriesCounters,
														m_oIsRecordInterrupted));
	const bool oInserted	= m_oFileCmdProcessor.pushBack( std::move(oUpBaseCommand), parameterToEnqueueMode());

	if (oInserted == true) {
		++m_oSeriesCounters.m_oNbImagesInserted;
	}
	else {	//maximum queue size reached
		++m_oSeriesCounters.m_oNbImagesMissed;

		m_oIsRecordInterrupted = true;
		m_rParameter.isEnabled(false);

		return false;
	} // else

	// nb of recored img plus new nb queued images has reached nb max img to be recorded in live mode. Disable recording.
	if (m_oIsLiveMode == true && m_oSeriesCounters.m_oNbImagesInserted >= m_rParameter.m_oNbMaxImgLive.value()) {
		wmLogTr(eWarning, "QnxMsg.Vdr.MaxImgLive", "Set live mode recording limit reached (%i).\n", m_rParameter.m_oNbMaxImgLive.value());
		// do not set interrupted flag, remaining images shall be written
		m_rParameter.isEnabled(false);
	} // if
	return true;
} // insertWriteQueue



bool Writer::insertWriteQueue(const vdrSample_t &p_rVdrSample) {
	const std::string		oFilePath			( makeFilePath(std::get<vdr_image_type::eImageNumber>(p_rVdrSample), g_oSampleExtension) );
	upBaseCommand_t			oUpBaseCommand		( new WriteSampleCmd(
														oFilePath,
														p_rVdrSample,
														m_rParameter,
														m_oSeriesCounters,
														m_oIsRecordInterrupted));
	const bool oInserted	= m_oFileCmdProcessor.pushBack( std::move(oUpBaseCommand), parameterToEnqueueMode());

	if (oInserted == true) {
		++m_oSeriesCounters.m_oNbSamplesInserted;
	}
	else {	//maximum queue size reached
		++m_oSeriesCounters.m_oNbSamplesMissed;

		m_oIsRecordInterrupted = true;
		m_rParameter.isEnabled(false);

		return false;
	} // else

	// nb of recored smp plus new nb queued samples has reached nb max img to be recorded in live mode. Disable recording.
	if (m_oIsLiveMode == true && m_oSeriesCounters.m_oNbSamplesInserted >= m_rParameter.m_oNbMaxImgLive.value()) {
		wmLogTr(eWarning, "QnxMsg.Vdr.MaxImgLive", "Set live mode recording limit reached (%i).\n", m_rParameter.m_oNbMaxImgLive.value());
		// do not set interrupted flag, remaining images shall be written
		m_rParameter.isEnabled(false);
	} // if
	return true;
} // insertWriteQueue



void Writer::printStatus() const {
	const auto	oNbImgPending	=	std::max(m_oSeriesCounters.m_oNbImagesInserted - m_oSeriesCounters.m_oNbImagesRecorded, 0u);
	if (m_oSeriesCounters.m_oNbImagesRecorded != 0) {
	wmLogTr(eInfo, "QnxMsg.Vdr.NbImagesRecorded", "%i images recorded, %i pending.\n",
			m_oSeriesCounters.m_oNbImagesRecorded, oNbImgPending);
	} // if
	if (m_oSeriesCounters.m_oNbSamplesRecorded != 0) {
		wmLogTr(eInfo, "QnxMsg.Vdr.NbSamplesRecorded", "%i samples recorded, %i pending.\n",
			m_oSeriesCounters.m_oNbSamplesRecorded, m_oSeriesCounters.m_oNbSamplesInserted - m_oSeriesCounters.m_oNbSamplesRecorded);
	} // if

	std::ostringstream oMsg;
	if (m_oSeriesCounters.m_oNbImagesMissed)
	{
		oMsg << "Number of images lost:\t\t"		<< 	m_oSeriesCounters.m_oNbImagesMissed 		<< ".\n"; 	wmLog(eDebug, oMsg.str()); oMsg.str("");
	}
	if (m_oSeriesCounters.m_oNbImageWritesFailed)
	{
		oMsg << "Number of failed image writes:\t"	<<	m_oSeriesCounters.m_oNbImageWritesFailed 	<< ".\n"; 	wmLog(eDebug, oMsg.str()); oMsg.str("");
	}
	if (m_oSeriesCounters.m_oNbSamplesMissed)
	{
		oMsg << "Number of samples lost:\t\t"		<< 	m_oSeriesCounters.m_oNbSamplesMissed 		<< ".\n"; 	wmLog(eDebug, oMsg.str()); oMsg.str("");
	}
	if (m_oSeriesCounters.m_oNbSampleWritesFailed)
	{
		oMsg << "Number of failed sample writes:\t"	<<	m_oSeriesCounters.m_oNbSampleWritesFailed 	<< ".\n"; 	wmLog(eDebug, oMsg.str()); oMsg.str("");
	}
} // printStatus

std::string Writer::cacheFilePath(bool liveMode) const
{
	const std::string&	rCacheFileName		( liveMode ? g_oLiveModeCacheFile : g_oProductCacheFile );

	Path oCacheFilePath(m_oWmBaseDir);
	oCacheFilePath.pushDirectory(g_oWmDataDir);	// use the data folder
	oCacheFilePath.setFileName(rCacheFileName);	// set file name
    return oCacheFilePath.toString();
}

void Writer::processRecord() {
	//const system::ScopedTimer	oTimer(__FUNCTION__);
	std::ostringstream oMsg;

	if (m_oIsLiveMode == false) {
		m_oAutoFoldersCreated	= false;	// reset flag
	} // if


	const Path&				rProductInstDir		(m_oIsLiveMode ? m_oProductInstDirLive : m_oProductInstDirAuto);
	// savety check

	const int	oDesiredDirectoryDepth	( g_oMinDirDepthDelete ); // /wm_inst/video/station/livemode_tmp_20130708-0952123 or depth 5: /wm_inst/video/station/prod_name/prod_nb
	const int	oProductInstanceDepth	( rProductInstDir.depth() );

	if (oProductInstanceDepth < oDesiredDirectoryDepth) {
		oMsg << "Depth of product instance path '" << rProductInstDir.toString() << "' is " << oProductInstanceDepth << " ...\n";
		wmLog(eDebug, oMsg.str()); oMsg.str("");
		oMsg << "... instead of " << oDesiredDirectoryDepth << ". Deletion skipped.\n";
		wmLog(eDebug, oMsg.str()); oMsg.str("");
		wmLogTr(eError, "QnxMsg.Vdr.InvalidFilePath", "Invalid file path: '%s' in procedure : '%s'. See log for details.\n", rProductInstDir.toString().c_str(), __FUNCTION__);

		return;
	} // if

	if (rProductInstDir.toString().find(g_oWmVideoDir) == std::string::npos) {
		oMsg << "Product instance path '" << rProductInstDir.toString() << "'...\n";
		wmLog(eDebug, oMsg.str()); oMsg.str("");
		oMsg << " ... does not contain '" << g_oWmVideoDir << "'. Deletion skipped.\n";
		wmLog(eDebug, oMsg.str()); oMsg.str("");
		wmLogTr(eError, "QnxMsg.Vdr.InvalidFilePath", "Invalid file path: '%s' in procedure : '%s'. See log for details.\n", rProductInstDir.toString().c_str(), __FUNCTION__);

		return;
	} // if

	// delete old product folders and save current product in cache file
    if (!m_oIsLiveMode)
    {
        ProductInstanceMetaData metaData;
        metaData.productUuid = m_oProductInstData.m_oProductId.toString();
        metaData.productName = m_oProductInstData.m_oProductName;
        metaData.productType = m_oProductInstData.m_oProductType;
        metaData.uuid = m_oProductInstData.m_oProductInstId.toString();
        metaData.serialNumber = m_oProductInstData.m_oProductNr;
        metaData.extendedProductInfo = m_oProductInstData.m_extendedProductInfo;
        std::time_t time = std::chrono::system_clock::to_time_t(m_startTime);
        char temp[1000];
        std::strftime(temp, 999, "%FT%T%z", std::localtime(&time));
        metaData.date = std::string{temp};
        Poco::Path filePath = rProductInstDir;
        filePath.setFileName("metadata.json");
        m_oFileCmdProcessor.pushBack(std::make_unique<ProductInstanceMetaDataCommand>(filePath, std::move(metaData)), CommandProcessor::EnqueueMode::Force);
    }

	const auto oNbEntriesToKeep	= m_oIsLiveMode ? m_rParameter.m_oNbLiveModeToKeep.value() : m_rParameter.m_oNbProductsToKeep.value();
    auto enqueue = [&] (CommandProcessor::EnqueueMode mode)
    {
        return m_oFileCmdProcessor.pushBack( upBaseCommand_t(new ProcessRecordCmd(
            rProductInstDir,
            m_oIsLiveMode,
            m_oProductInstDirLiveFinal.toString(),
            cacheFilePath(m_oIsLiveMode),
            oNbEntriesToKeep,
            m_rSchedulerEventsProxy)), mode );
    };
	const auto oEnqueued 		= enqueue(parameterToEnqueueMode());
	if (oEnqueued == false)	// worker queue full -> full stop
	{
		m_oIsRecordInterrupted = true;
		m_rParameter.isEnabled(false);
        enqueue(CommandProcessor::EnqueueMode::Force);
	}
} // processRecord



void Writer::uninitialize() {
	std::ostringstream oMsg;
	oMsg << "Joining threads, writing config to file and shutting down...\n";
	wmLog(eDebug, oMsg.str());

	// write current configuration to disk

	writeToFile(m_oConfigDir.toString(), makeConfiguration(m_rParameter, false) );
	oMsg.str("");
	oMsg << "Wrote current configuration to '" << m_oConfigDir.toString() <<  "'.\n";
	wmLog(eDebug, oMsg.str());

	const auto oEnqueued = m_oFileCmdProcessor.pushBack( upBaseCommand_t(new DeleteRecursivelyCmd(m_oProductInstDirLive)), parameterToEnqueueMode()); // delete temp live mode folder
	if (oEnqueued == false)	// worker queue full -> full stop
	{
		m_oIsRecordInterrupted = true;
		m_rParameter.isEnabled(false);
	}

	m_oFileCmdProcessor.uninitialize();
} // uninitialize



void Writer::setIsLiveMode(bool p_oIsLiveMode) {
	m_oIsLiveMode	= p_oIsLiveMode;
} // setIsLiveMode



bool Writer::getIsLiveMode() const {
	return m_oIsLiveMode;
} // getIsLiveMode



void Writer::resetSeriesCounter() {
	m_oSeriesCounters.m_oNbImagesInserted		= 0;
	m_oSeriesCounters.m_oNbImagesRecorded		= 0;
	m_oSeriesCounters.m_oNbImagesMissed			= 0;
	m_oSeriesCounters.m_oNbImageWritesFailed	= 0;
	m_oSeriesCounters.m_oNbSamplesInserted		= 0;
	m_oSeriesCounters.m_oNbSamplesRecorded		= 0;
	m_oSeriesCounters.m_oNbSamplesMissed		= 0;
	m_oSeriesCounters.m_oNbSampleWritesFailed	= 0;
} // resetSeriesCounter



void Writer::setInterruption() {
	m_oIsRecordInterrupted = true;
} // setInterruption



void Writer::resetInterruption() {
	m_oIsRecordInterrupted = false;
} // resetInterruption



void Writer::writeSequenceInfo() const {
	Path 					oSequenceInfo		( m_oSeamDir );

	oSequenceInfo.setFileName(g_oSequenceInfoFile);
	oSequenceInfo.setExtension(g_oXmlExtension);

	const File				oSequenceInfoFile	( oSequenceInfo );
	const ProductInstData&	rProductInstData	( m_oIsLiveMode ? m_oProductInstDataLive : m_oProductInstData );

	const auto oEnqueued = m_oFileCmdProcessor.pushBack( upBaseCommand_t(new WriteConfCmd(oSequenceInfoFile, rProductInstData, m_oSeamData, m_oSeriesCounters)), parameterToEnqueueMode());
	if (oEnqueued == false)	// worker queue full -> full stop
	{
		m_oIsRecordInterrupted = true;
		m_rParameter.isEnabled(false);
	}
} // writeSequenceInfo



bool Writer::getAutoFoldersCreated() const {
	return m_oAutoFoldersCreated;
} // getAutoFoldersCreated



void Writer::setAutoFoldersCreated(bool p_oAutoFoldersCreated) {
	m_oAutoFoldersCreated = p_oAutoFoldersCreated;
} // setAutoFoldersCreated



bool Writer::checkIfBusy() const {
	return m_oFileCmdProcessor.isBusy();
} // checkIfBusy



void Writer::updateCache(bool p_oIsLiveMode) const
{
	const auto	oNbEntriesToKeep	= p_oIsLiveMode ? m_rParameter.m_oNbLiveModeToKeep.value() : m_rParameter.m_oNbProductsToKeep.value();
	const auto 	oEnqueued 			= m_oFileCmdProcessor.pushBack( upBaseCommand_t(new UpdateCacheCmd(
			cacheFilePath(p_oIsLiveMode),
			oNbEntriesToKeep,
            g_oWmVideoDir)),
            parameterToEnqueueMode());

	if (oEnqueued == false)	// worker queue full -> full stop
	{
		m_oIsRecordInterrupted = true;
		m_rParameter.isEnabled(false);
	}
} // updateCache

void Writer::deleteProductInstance(bool liveMode, const std::string &path)
{
    m_oFileCmdProcessor.pushBack( upBaseCommand_t{new DeleteProductInstanceCmd{path, cacheFilePath(liveMode), g_oWmVideoDir}}, parameterToEnqueueMode());
}

std::string Writer::makeFilePath(unsigned int p_oImageNumber, const std::string& p_rExtension) const {
	Poco::Path			oSeamDir		( m_oSeamDir );  // create image path within seam folder
	std::ostringstream oImageNumberStream;
	oImageNumberStream << std::setw(g_oFillWidthImgName) << std::setfill('0') << p_oImageNumber;
	try {
		oSeamDir.setFileName( oImageNumberStream.str() );
		oSeamDir.setExtension( p_rExtension );
	} // try
	catch(const Exception &p_rException) {
		wmLog(eDebug, "%s: '%s' - %s\n", __FUNCTION__, p_rException.name(), p_rException.message().c_str());
		wmLogTr(eError, "QnxMsg.Vdr.InvalidFilePath", "Invalid file path: '%s' in procedure : '%s'. See log for details.\n", oSeamDir.toString().c_str(), __FUNCTION__);
	} // catch

	return oSeamDir.toString();
} // makeFilePath

CommandProcessor::EnqueueMode Writer::parameterToEnqueueMode() const
{
    return m_rParameter.m_skipIfFull.value() == false ? CommandProcessor::EnqueueMode::Force : CommandProcessor::EnqueueMode::FailOnFull;
}

// free functions

std::string makeEnumeratedRepoDirName(const std::string &p_rLabel, int p_oNumber) {
	std::ostringstream	oEnumeratedRepoDirName;
	oEnumeratedRepoDirName << p_rLabel << std::setw(g_oFillWidthDirName) << std::setfill('0') << p_oNumber;
	return oEnumeratedRepoDirName.str();
} // makeEnumeratedRepoDirName



interface::ProductInstData makeLivemodeInstance() {
	static const seamDataVector_t	oLivemodeSeamData	( 1, SeamData(0, 0, 0) );
	return ProductInstData(g_oLiveModeName, UUID(), UUID::null(), g_oLiveModeProdType, g_oLiveModeProdNumber, {}, oLivemodeSeamData);
} // makeLivemodeInstance

} // namespace vdr
} // namespace precitec
