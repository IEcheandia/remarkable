/**
 * 	@file
 * 	@copyright	Precitec Vision GmbH & Co. KG
 * 	@author		Simon Hilsenbeck (HS)
 * 	@date		2012
 * 	@brief		File commands which can be executed by a command processor.
 */

// local includes
#include "videoRecorder/fileCommand.h"
#include "videoRecorder/literal.h"
#include "module/moduleLogger.h"
#include "system/tools.h"
#include "system/timer.h"
#include "common/bitmap.h"					///< Bitmap writing
#include "common/sample.h"					///< Sample writing
// poco includes
#include "Poco/DirectoryIterator.h"
#include "Poco/TextConverter.h"
#include "Poco/UTF8Encoding.h"
#include "Poco/Windows1252Encoding.h"
#include "Poco/DateTimeFormatter.h"
#include "Poco/DateTime.h"
// stl includes
#include <deque>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <sstream>
// os includes
#include <cstdlib>							///< system()
#include <sys/wait.h>

using namespace fileio;
namespace precitec {
	using namespace Poco;
	using namespace system;
namespace vdr {


void DeleteRecursivelyCmd::execute() {
	std::ostringstream oMsg;
	try {
		if (m_oFile.exists() == false) {
			return;
		} // if
		m_oFile.remove(true);
#ifndef NDEBUG
		oMsg << "Removed: '" << m_oFile.path() << "'.\n";
		wmLog(eDebug, oMsg.str());
#endif // #ifndef NDEBUG
	} // try
	catch(const Exception &p_rException) {
		oMsg << "Not removed: '" << m_oFile.path() << "'.\n";
		wmLog(eDebug, oMsg.str());
		wmLogTr(eWarning, "QnxMsg.Vdr.FileCmdError", "File handling error at file '%s'. Action: '%s'. See log for details.\n", formatPath(m_oFile.path()).c_str(), "DeleteRecursivelyCmd");
		throw; // handle actual exception in caller 'CommandProcessor::work()'
	} // catch
} // execute



void WriteConfCmd::execute() {
	using interface::ProductInstData;

	std::ostringstream oMsg;
	try {
		// convert from codepage 1252 to utf8, to avoid writing stop bug. With poco 146 also the non-ascii-string content will be correct.

		const auto o1252ToUtf8	( [](const std::string& p_rIn)->std::string{
			static const Windows1252Encoding	oWindows1252encoding; // also seems to work under posix
			static const UTF8Encoding			oUtf8encoding;
			static TextConverter				oTextconverter			( oWindows1252encoding, oUtf8encoding );
			std::string							oOut;
			oTextconverter.convert(p_rIn, oOut);
			return oOut;
		} );

		AutoPtr<Util::XMLConfiguration> pConf(new Util::XMLConfiguration);
		pConf->loadEmpty(g_oConfigFileName);
		const std::string		oDateTimeNow		( DateTimeFormatter::format(LocalDateTime(), g_oDateTimeFormatLong) ); // "061022-185505123"
		pConf->setString	(g_oTagDateTime,		oDateTimeNow);
		pConf->setBool		(g_oTagIsLiveMode,		m_oProductInstData.m_oProductName == g_oLiveModeName);
		pConf->setString	(g_oTagProductName,		o1252ToUtf8(m_oProductInstData.m_oProductName)); // convert non-ascii user string
		pConf->setString	(g_oTagProductInst,		m_oProductInstData.m_oProductInstId.toString());
		pConf->setString	(g_oTagProductUUID,		m_oProductInstData.m_oProductId.toString());
		pConf->setInt		(g_oTagProductType,		m_oProductInstData.m_oProductType); // uint not possible. Needs reinterpret when read with getInt().
		pConf->setInt		(g_oTagProductNumber,	m_oProductInstData.m_oProductNr); // uint not possible. Needs reinterpret when read with getInt().
		pConf->setString	(g_oTagExtendedProductInfo, m_oProductInstData.m_extendedProductInfo);
		pConf->setInt		(g_oTagSeamSeries,		m_oSeamData.m_oSeamSeries);
		pConf->setInt		(g_oTagSeam,			m_oSeamData.m_oSeam);
		pConf->setInt		(g_oTagTriggerDelta,	m_oSeamData.m_oTriggerDelta);
		pConf->setInt		(g_oTagNbImages,		m_rRecordCounters.m_oNbImagesRecorded);
		pConf->setInt		(g_oTagNbSamples,		m_rRecordCounters.m_oNbSamplesRecorded);

		m_oFile.createFile();
		pConf->save(m_oFile.path());
#ifndef NDEBUG
		wmLog(eDebug, "Config file written:\n");
		oMsg << "'" << m_oFile.path() << "'.\n";
		wmLog(eDebug, oMsg.str());
#endif // #ifndef NDEBUG
	} // try
	catch(const Exception &p_rException) {
		wmLog(eDebug, "Config file not written:\n");
		oMsg << "'" << m_oFile.path() << "'.\n";
		wmLog(eDebug, oMsg.str());
		wmLogTr(eWarning, "QnxMsg.Vdr.FileCmdError", "File handling error at file '%s'. Action: '%s'. See log for details.\n", formatPath(m_oFile.path()).c_str(), "WriteConfCmd");
		throw; // handle actual exception in caller 'CommandProcessor::work()'
	} // catch
} // execute



void CreateFileCmd::execute() {
	std::ostringstream oMsg;
	try {
		m_oFile.createFile();
#ifndef NDEBUG
		oMsg << "Created: '" << m_oFile.path() << "'.\n";
		wmLog(eDebug, oMsg.str());
#endif // #ifndef NDEBUG
	} // try
	catch(const Exception &p_rException) {
		oMsg << "Not created: '" << m_oFile.path() << "'.\n";
		wmLog(eDebug, oMsg.str());
		wmLogTr(eWarning, "QnxMsg.Vdr.FileCmdError", "File handling error at file '%s'. Action: '%s'. See log for details.\n", formatPath(m_oFile.path()).c_str(), "CreateDirsCmd");
		throw; // handle actual exception in caller 'CommandProcessor::work()'
	} // catch
} // execute



void CreateDirsCmd::execute() {
	std::ostringstream oMsg;
	try {
		m_oFile.createDirectories();
#ifndef NDEBUG
		oMsg << "Created: '" << m_oFile.path() << "'.\n";
		wmLog(eDebug, oMsg.str());
#endif // #ifndef NDEBUG
	} // try
	catch(const Exception &p_rException) {
		oMsg << "Not created: '" << m_oFile.path() << "'.\n";
		wmLog(eDebug, oMsg.str());
		wmLogTr(eWarning, "QnxMsg.Vdr.FileCmdError", "File handling error at file '%s'. Action: '%s'. See log for details.\n", formatPath(m_oFile.path()).c_str(), "CreateDirsCmd");
		throw; // handle actual exception in caller 'CommandProcessor::work()'
	} // catch
} // execute



void RenameCmd::execute() {
	std::ostringstream oMsg;
	const std::string oFileOrigin	(m_oFile.path());
	try {
		auto	oNewFilePath	= Path{ m_oNewFilePath };
		oNewFilePath.setBaseName(m_oNewFilePath.getBaseName());
		oNewFilePath.setExtension(m_oNewFilePath.getExtension());
		m_oFile.renameTo(oNewFilePath.toString());
#ifndef NDEBUG
		oMsg << "Renamed: " << oFileOrigin << "\n";
		wmLog(eDebug, oMsg.str()); oMsg.str("");
		oMsg << "to: " << m_oNewFilePath.toString() << ".\n";
		wmLog(eDebug, oMsg.str());
#endif // #ifndef NDEBUG
	} // try
	catch(const Exception &p_rException) {
		oMsg << "Not renamed: " << oFileOrigin << "\n";
		wmLog(eDebug, oMsg.str()); oMsg.str("");
		oMsg << "to: " << m_oNewFilePath.toString() << ".\n";
		wmLog(eDebug, oMsg.str());
		wmLogTr(eWarning, "QnxMsg.Vdr.FileCmdError", "File handling error at file '%s'. Action: '%s'. See log for details.\n", formatPath(m_oFile.path()).c_str(), "RenameCmd");
		throw; // handle actual exception in caller 'CommandProcessor::work()'
	} // catch
} // execute



void MoveUnixCmd::execute() {
	//system::ScopedTimer oTimer("MoveUnixCmd"); // ~ 16 ms
	std::ostringstream oMsg;
	const std::string 		oFileOrigin					(m_oFile.path());
	try {
		const std::string		oDestinationName			(m_oDestination.getBaseName());
		File(m_oDestination).createDirectories();

		// see http://www.qnx.com/developers/docs/6.3.2/neutrino/utilities/m/mv.html
		// pipes non-error messages to dev/null
		std::ostringstream	oCommand;
		oCommand << "mv \"" << oFileOrigin << "/*\" \"" << m_oDestination.toString() << "\" > /dev/null";

//		const int 			oSysStatus	WEXITSTATUS( std::system(oCommand.str().c_str()) ); // WEXITSTATUS evaluates lower 8 bits
		int oRetValue = std::system(oCommand.str().c_str());
		const int 			oSysStatus		=	((oRetValue >> 8) & 0xff);
		if (oSysStatus != EXIT_SUCCESS) {
			wmLog(eDebug, "Failed command: %s\n", oCommand.str().c_str()); oMsg.str("");
			oMsg << "Call to 'mv' returned (WEXITSTATUS): " << oSysStatus << "\n";
			wmLogTr(eWarning, "QnxMsg.Vdr.FileCmdError", "File handling error at file '%s'. Action: '%s'. See log for details.\n", formatPath(m_oFile.path()).c_str(), "MoveUnixCmd");
			throw SystemException( oMsg.str() );
		} // if

		m_oFile.remove(false); // remove empty source folder

#ifndef NDEBUG
		oMsg << "Moved: " << oFileOrigin << "\n";
		wmLog(eDebug, oMsg.str()); oMsg.str("");
		oMsg << "to: " << m_oDestination.toString() << ".\n";
		wmLog(eDebug, oMsg.str());
#endif // #ifndef NDEBUG
	} // try
	catch(const Exception &p_rException) {
		oMsg.str(""); oMsg << "Not moved: " << oFileOrigin << "\n";
		wmLog(eDebug, oMsg.str()); oMsg.str("");
		oMsg << "to: " << m_oDestination.toString() << ".\n";
		wmLog(eDebug, oMsg.str());
		wmLogTr(eWarning, "QnxMsg.Vdr.FileCmdError", "File handling error at file '%s'. Action: '%s'. See log for details.\n", formatPath(m_oFile.path()).c_str(), "MoveUnixCmd");
		throw; // handle actual exception in caller 'CommandProcessor::work()'
	} // catch
} // execute



void WriteImageCmd::execute() {
	if (m_rIsRecordInterrupted == true) {
		return; // until queue is empty
	} // if
	std::ostringstream oMsg;

	using namespace vdr_image_type;
	const auto	oCurrentImg		=	std::get<eImage>		(m_oVdrImage);
	const auto	oImgNb			=	std::get<eImageNumber>	(m_oVdrImage);
	const auto	oHwRoiX			=	std::get<eHwRoiX>		(m_oVdrImage);
	const auto	oHwRoiY			=	std::get<eHwRoiY>		(m_oVdrImage);

	auto		oAdditionalData	=	std::vector<unsigned char>(add_data_indices::eNbBytes); // see bitmap.h for format

	*(reinterpret_cast<unsigned short*>(&oAdditionalData[add_data_indices::eVersion]))		=	0; // see bitmap.h for version
	*(reinterpret_cast<unsigned short*>(&oAdditionalData[add_data_indices::eHwRoiX]))		=	oHwRoiX;
	*(reinterpret_cast<unsigned short*>(&oAdditionalData[add_data_indices::eHwRoiY]))		=	oHwRoiY;
	*(reinterpret_cast<unsigned short*>(&oAdditionalData[add_data_indices::eImgNb]))		=	oImgNb;

	const std::string&	rPathWithFile			( m_oFile.path() );
	File				oPathWithoutFile 		( rPathWithFile.substr(0, rPathWithFile.find_last_of(Path::separator()) + 1) );

	if(oPathWithoutFile.exists() == false) {
		oPathWithoutFile.createDirectories();
		wmLog(eDebug, "Image directory does not exist. Directory created:\n");
		oMsg << "'" << oPathWithoutFile.path() << "'.\n";
		wmLog(eDebug, oMsg.str()); oMsg.str("");
		wmLogTr(eWarning, "QnxMsg.Vdr.FileCmdError", "File handling error at file '%s'. Action: '%s'. See log for details.\n", formatPath(m_oFile.path()).c_str(), "WriteImageCmd");
	} // if

	if (oImgNb % g_oCheckDuEveryNbImg == 0) { // do the check only every nth image - takes ~40us in debug
		const double oCurrentDiskUsage	= getDiskUsage("./");
		if (oCurrentDiskUsage > m_rParameter.m_oMaxDiskUsage.value()) {
			m_rParameter.isEnabled(false);
			m_rIsRecordInterrupted	= true;
			oMsg.str("");
			oMsg << std::setprecision(g_oPrecisionDiskUsage) << oCurrentDiskUsage * 100;
			wmLogTr(eWarning, "QnxMsg.Vdr.DiskUsage", "Current disk usage is %s%%.\n", oMsg.str().c_str()); // keep msg - oMsg.str("");
			wmLogTr(eWarning, "QnxMsg.Vdr.DiskUsageWarning", "The limit of %s%% has been reached. Recording disabled.\n", oMsg.str().c_str());
			return;
		} // if
	} // if

	if ( m_rGrabberStatusProxy.isImgNbInBuffer(std::get<eImageId>(m_oVdrImage)) == false ) {
		//m_rParameter.isEnabled(false);
		//m_rIsRecordInterrupted	= true;
		//wmLogTr(eWarning, "QnxMsg.Vdr.RecordingTooSlow", "Recording disabled, because too slow. Consider smaller ROI and or slower triggering.\n");
		wmLog(eDebug, "Harddisk overwhelmed, data rate too high. Consider smaller ROI and or slower triggering. Skipped writing image %d.\n",std::get<eImageId>(m_oVdrImage) );
		++m_rCounters.m_oNbImageWritesFailed;
		return;
	} // if
	fileio::Bitmap 	oBitmap		(rPathWithFile , oCurrentImg.size().width, oCurrentImg.size().height );
	const auto oSaveOk = oCurrentImg.isValid() ? oBitmap.save(oCurrentImg.begin(), oAdditionalData) : false;
	if (oSaveOk == true) {
		++m_rCounters.m_oNbImagesRecorded;
#ifndef NDEBUG
		oMsg << oImgNb << " written to '" << m_oFile.path() << "'.\n";
		wmLog(eDebug, oMsg.str()); oMsg.str("");
#endif // #ifndef NDEBUG
	} // if
	else {
		oMsg.str("");
		oMsg << "Writing '" << m_oFile.path() << "' to disk failed.\n";
		wmLog(eDebug, oMsg.str()); oMsg.str("");
		wmLogTr(eWarning, "QnxMsg.Vdr.FileCmdError", "File handling error at file '%s'. Action: '%s'. See log for details.\n", formatPath(m_oFile.path()).c_str(), "WriteImageCmd");
		++m_rCounters.m_oNbImageWritesFailed;
	} // else
} // execute



void WriteSampleCmd::execute() {
	if (m_rIsRecordInterrupted == true) {
		return; // until queue is empty
	} // if
	std::ostringstream oMsg;

	const auto	oCurrentSample	=	std::get<vdr_sample_type::eSample>		(m_oVdrSample);
	const auto	oTriggerNb		=	std::get<vdr_sample_type::eTriggerNb>	(m_oVdrSample);
	const auto	oSensorId		=	std::get<vdr_sample_type::eSensorId>	(m_oVdrSample);

	const std::string&	rPathWithFile			( m_oFile.path() );
	File				oPathWithoutFile 		( rPathWithFile.substr(0, rPathWithFile.find_last_of(Path::separator()) + 1) );

	if(oPathWithoutFile.exists() == false) {
		oPathWithoutFile.createDirectories();
		wmLog(eDebug, "Sample directory does not exist. Directory created:\n");
		oMsg << "'" << oPathWithoutFile.path() << "'.\n";
		wmLog(eDebug, oMsg.str()); oMsg.str("");
		wmLogTr(eWarning, "QnxMsg.Vdr.FileCmdError", "File handling error at file '%s'. Action: '%s'. See log for details.\n", formatPath(m_oFile.path()).c_str(), "WriteSampleCmd");
	} // if

	if (oTriggerNb % g_oCheckDuEveryNbImg == 0) { // do the check only every nth sample - takes ~40us in debug
		const double oCurrentDiskUsage	= getDiskUsage("./");
		if (oCurrentDiskUsage > m_rParameter.m_oMaxDiskUsage.value()) {
			m_rParameter.isEnabled(false);
			m_rIsRecordInterrupted	= true;
			oMsg.str("");
			oMsg << std::setprecision(g_oPrecisionDiskUsage) << oCurrentDiskUsage * 100;
			wmLogTr(eWarning, "QnxMsg.Vdr.DiskUsage", "Current disk usage is %s%%.\n", oMsg.str().c_str()); // keep msg - oMsg.str("");
			wmLogTr(eWarning, "QnxMsg.Vdr.DiskUsageWarning", "The limit of %s%% has been reached. Recording disabled.\n", oMsg.str().c_str());
			return;
		} // if
	} // if


	Sample 		oSample		{ rPathWithFile };
	const auto 	pRawData	=	reinterpret_cast<const char*>(oCurrentSample.data());
	const auto 	oDataSize	=	oCurrentSample.getSize();
	const auto 	oSaveOk 	= 	oSample.appendDataBlock(oSensorId, pRawData, oDataSize);

	if (oSaveOk == true) {
		++m_rCounters.m_oNbSamplesRecorded;
#ifndef NDEBUG
		oMsg << oTriggerNb << " written to '" << m_oFile.path() << "'.\n";
		wmLog(eDebug, oMsg.str()); oMsg.str("");
#endif // #ifndef NDEBUG
	} // if
	else {
		oMsg.str("");
		oMsg << "Writing '" << m_oFile.path() << "' to disk failed.\n";
		wmLog(eDebug, oMsg.str()); oMsg.str("");
		wmLogTr(eWarning, "QnxMsg.Vdr.FileCmdError", "File handling error at file '%s'. Action: '%s'. See log for details.\n", formatPath(m_oFile.path()).c_str(), "WriteSampleCmd");
		++m_rCounters.m_oNbSampleWritesFailed;
	} // else
} // execute



void ProcessRecordCmd::execute() {
	std::ostringstream oMsg;
	try {
		const auto	oAllSeamsDeleted		( deleteEmptyFolders() );
		if (oAllSeamsDeleted == true) {
			return; // if empty nothing remains that can be moved or cached
		} // if

		// move to final live mode folder if in live mode

		if (m_oIsLiveMode == true) {
			moveLiveModeRecord();
		} // if
        else
        {
            Poco::Path path{m_oFile.path()};
            const auto name = path.directory(path.depth());
            path.makeParent();
            path.pushDirectory(name.substr(1));
            m_oFile.renameTo(path.toString());

            m_rSchedulerEventsProxy.schedulerEventFunction(interface::SchedulerEvents::ProductInstanceVideoStored, {{"path", path.toString()}});
        }
		cachePath();
	} // try
	catch(const Exception &p_rException) {
		oMsg << "Record not processed: " << m_oFile.path() << "\n";
		wmLog(eDebug, oMsg.str()); oMsg.str("");
		wmLogTr(eWarning, "QnxMsg.Vdr.FileCmdError", "File handling error at file '%s'. Action: '%s'. See log for details.\n", formatPath(m_oFile.path()).c_str(), "ProcessRecordCmd");
		throw; // handle actual exception in caller 'CommandProcessor::work()'
	} // catch
} // execute



bool ProcessRecordCmd::deleteEmptyFolders() const {
	std::ostringstream oMsg;
	try {

		const DirectoryIterator	oEndIt;
		DirectoryIterator		oSeriesIt					( m_oFile ); // requires that the folder exists.
		int						oNbSeamFoldersNotDeleted	( 0 );
		int						oNbSeamFoldersDeleted		( 0 );
		File					oInstIdFile;
        File oMetaDataFile;

		while (oSeriesIt != oEndIt) { // all series folders within product
			if (oSeriesIt->isFile()) {
				//oMsg << "Skipped: '" << oSeriesIt.path().toString() << "'\n";
				//wmLog(eDebug, oMsg.str()); oMsg.str("");
                if (oSeriesIt.name() == "metadata.json")
                {
                    oMetaDataFile = File{ oSeriesIt.path() };
                }
                else
                {
                    oInstIdFile	= File{ oSeriesIt.path() };
                }
				++oSeriesIt;
				continue; // skip id file
			} // if

			DirectoryIterator oSeamIt	(oSeriesIt.path());

			bool oAllSeamsEmpty		(true);

			while (oSeamIt != oEndIt) { // all seam folders within product
				const auto	oImagePattern	=	oSeamIt->path() + Path::separator() + "*." + g_oImageExtension;
				const auto	oNbImages		=	getNbFilesMatchingPattern(oImagePattern);
				const auto	oSamplePattern	=	oSeamIt->path() + Path::separator() + "*." + g_oSampleExtension;
				const auto	oNbSamples		=	getNbFilesMatchingPattern(oSamplePattern);

				if (oNbImages + oNbSamples == 0) { // seam dir contains no bmp images and no smps
					try {
						(*oSeamIt).remove(true);
#ifndef NDEBUG
						oMsg << "Deleted: '" << oSeamIt->path() << "'.\n";
						wmLog(eDebug, oMsg.str()); oMsg.str("");
#endif // #ifndef NDEBUG
					} // try
					catch(const Exception &p_rException) {
						oMsg << "Not deleted: '" << oSeamIt->path() << "'.\n";
						wmLog(eDebug, oMsg.str()); oMsg.str("");
						oMsg  << p_rException.what() << " - " << p_rException.message() << "\n";
						wmLog(eDebug, oMsg.str()); oMsg.str("");
						wmLogTr(eWarning, "QnxMsg.Vdr.FileCmdError", "File handling error at file '%s'. Action: '%s'. See log for details.\n", formatPath(m_oFile.path()).c_str(), "deleteEmptyFolders");
					} // catch
					++oNbSeamFoldersDeleted;
				}
				else {
					++oNbSeamFoldersNotDeleted;
					oAllSeamsEmpty	= false; // theres is an not empty seam
				}
				++oSeamIt; // next seam folder
			} // while

			if (oAllSeamsEmpty == true) { // we could delete all empty seam folders. So delete this series folder.
				try {
					(*oSeriesIt).remove(false);
#ifndef NDEBUG
					oMsg << "Deleted: '" << oSeriesIt->path() << "'.\n";
					wmLog(eDebug, oMsg.str()); oMsg.str("");
#endif // #ifndef NDEBUG
				} // try
				catch(const Exception &p_rException) {
					oMsg << "Not deleted: '" << oSeriesIt->path() << "'.\n";
					wmLog(eDebug, oMsg.str()); oMsg.str("");
					oMsg  << p_rException.what() << " - " << p_rException.message() << "\n";
					wmLog(eDebug, oMsg.str()); oMsg.str("");
					wmLogTr(eWarning, "QnxMsg.Vdr.FileCmdError", "File handling error at file '%s'. Action: '%s'. See log for details.\n", formatPath(m_oFile.path()).c_str(), "deleteEmptyFolders");
				} // catch
			} // if
			else {
				//oAllSeriesEmpty	= false; // theres is an not empty seam
			}

			++oSeriesIt; // next seam series folder
		} // while

		if (oNbSeamFoldersNotDeleted != 0)
		{
			oMsg << oNbSeamFoldersNotDeleted << " seam(s) recorded, " << oNbSeamFoldersDeleted << " empty seam folder(s) deleted.\n";
			wmLog(eDebug, oMsg.str()); oMsg.str("");
		}

		// delete the product instance folder, if empty. After, it does not need to be inserted into cache.

		if (oNbSeamFoldersNotDeleted == 0) {
			try {
				if (!oInstIdFile.path().empty() && oInstIdFile.exists()) {
					//oMsg << "Delete : '" << oInstIdFile.path() << "'\n";
					//wmLog(eDebug, oMsg.str()); oMsg.str("");
					oInstIdFile.remove(false); // remove id file
				} // if
                if (!oMetaDataFile.path().empty() && oMetaDataFile.exists())
                {
                    oMetaDataFile.remove(false);
                }
				m_oFile.remove(false);
#ifndef NDEBUG
				oMsg << "Deleted: '" << m_oFile.path() << "'.\n";
				wmLog(eDebug, oMsg.str()); oMsg.str("");
#endif // #ifndef NDEBUG
			} // try
			catch(const Exception &p_rException) {
				oMsg << "Not deleted: '" << m_oFile.path() << "'.\n";
				wmLog(eDebug, oMsg.str()); oMsg.str("");
				oMsg  << p_rException.what() << " - " << p_rException.message() << "\n";
				wmLog(eDebug, oMsg.str()); oMsg.str("");
				wmLogTr(eWarning, "QnxMsg.Vdr.FileCmdError", "File handling error at file '%s'. Action: '%s'. See log for details.\n", formatPath(m_oFile.path()).c_str(), "deleteEmptyFolders");
			} // catch
			return true;
		} // if
		else { // not empty
			return false;
		} // else
	} // try
	catch(const Exception &p_rException) {
		oMsg << "Record not processed: " << m_oFile.path() << "\n";
		wmLog(eDebug, oMsg.str()); oMsg.str("");
		wmLogTr(eWarning, "QnxMsg.Vdr.FileCmdError", "File handling error at file '%s'. Action: '%s'. See log for details.\n", formatPath(m_oFile.path()).c_str(), "deleteEmptyFolders");
		throw; // handle actual exception in caller 'CommandProcessor::work()'
	} // catch
	return false; // possibly false in error case
} // deleteEmptyFolders



void ProcessRecordCmd::moveLiveModeRecord() const {
	std::ostringstream oMsg;

	const std::string&		oFileOrigin					( m_oFile.path() );
	const std::string&		oDestinationName			( m_oProductDirLiveFinal );

	try {
		File(oDestinationName).createDirectories();

		// see http://www.qnx.com/developers/docs/6.3.2/neutrino/utilities/m/mv.html
		// pipes non-error messages to dev/null
		std::ostringstream	oCommand;
		oCommand << "mv \"" << oFileOrigin << "/\"* \"" << oDestinationName << "\" > /dev/null";

//		const int 			oSysStatus	WEXITSTATUS( std::system(oCommand.str().c_str()) ); // WEXITSTATUS evaluates lower 8 bits
		int oRetValue = std::system(oCommand.str().c_str());
		const int 			oSysStatus		=	((oRetValue >> 8) & 0xff);
		if (oSysStatus != EXIT_SUCCESS) {
			wmLog(eDebug, "Failed command: %s\n", oCommand.str().c_str()); oMsg.str("");
			oMsg << "Call to 'mv' returned (WEXITSTATUS): " << oSysStatus << "\n";
			throw SystemException( oMsg.str() );
		} // if

		m_oFile.remove(false); // remove empty source folder

#ifndef NDEBUG
		oMsg << "Moved: " << oFileOrigin << "\n";
		wmLog(eDebug, oMsg.str()); oMsg.str("");
		oMsg << "to: " << oDestinationName << ".\n";
		wmLog(eDebug, oMsg.str());
#endif // #ifndef NDEBUG
	} // try
	catch(const Exception &p_rException) {
		oMsg.str(""); oMsg << "Not moved: " << oFileOrigin << "\n";
		wmLog(eDebug, oMsg.str()); oMsg.str("");
		oMsg << "to: " << oDestinationName << ".\n";
		wmLog(eDebug, oMsg.str()); oMsg.str("");
		oMsg  << p_rException.what() << " - " << p_rException.message() << "\n";
		wmLog(eDebug, oMsg.str());
		wmLogTr(eWarning, "QnxMsg.Vdr.FileCmdError", "File handling error at file '%s'. Action: '%s'. See log for details.\n", formatPath(m_oFile.path()).c_str(), "moveLiveModeRecord");
		throw; // handle actual exception in caller 'CommandProcessor::work()'
	} // catch
} // moveLiveModeRecord

namespace
{

std::deque<std::string> readCache(const std::string &cacheFilePath)
{
    std::ostringstream oMsg;
    std::ifstream oIfstream(cacheFilePath);	// open cache file for reading
    if (oIfstream.good() == false) {
        oMsg << __FUNCTION__ << ": Could not open existing cache file: '" << cacheFilePath << "'. Creating file.\n";
        wmLog(eDebug, oMsg.str()); oMsg.str("");
        oIfstream.close();
        oIfstream.open(cacheFilePath, std::ios_base::trunc);
    } // if
    // put file content in container

    std::deque<std::string>	oProdList;
    std::string oLine;
    while(std::getline(oIfstream, oLine)) {
        oProdList.push_back(oLine);
    } // while

#ifndef NDEBUG
    oMsg << __FUNCTION__ << ": Nb of old entries in cache file found: " << oProdList.size() << "\n";
    wmLog(eDebug, oMsg.str()); oMsg.str("");
#endif // #ifndef NDEBUG

    oIfstream.close(); // close cache file

    return oProdList;
}

void writeCache(const std::string &cacheFilePath, const std::deque<std::string> &cache)
{
    // Write to a temporary file first and then rename it, so operations look atomic on the file system.
    const std::string tmpPath = cacheFilePath + ".tmp";
    std::ofstream oOfstream(tmpPath); // open cache file for writing
    if (oOfstream.good() == false) { // should not happen
        std::ostringstream oMsg;
        oMsg << "Could not open cache file: '" << cacheFilePath << "'\n";
        wmLog(eDebug, oMsg.str()); oMsg.str("");
        wmLogTr(eWarning, "QnxMsg.Vdr.FileCmdError", "File handling error at file '%s'. Action: '%s'. See log for details.\n", cacheFilePath.c_str(), "cyclicDelete");
    }
    std::copy(std::begin(cache), std::end(cache), std::ostream_iterator<std::string>(oOfstream, "\n")); // write container to file
    std::filesystem::rename(tmpPath, cacheFilePath);
}

}

CachePath::CachePath(const std::string &cacheFilePath, const std::string &pathToAdd, std::size_t entriesToKeep, const std::string &pathToDeleteMustContain)
    : m_oCacheFilePath(cacheFilePath)
    , m_pathToAdd(pathToAdd)
    , m_oNbEntriesToKeep(entriesToKeep)
    , m_pathToDeleteMustContain(pathToDeleteMustContain)
{
}

void CachePath::operator()()
{
	std::ostringstream oMsg;
	try {
		std::deque<std::string>	oProdList{readCache(m_oCacheFilePath)};

		oProdList.push_back(m_pathToAdd); // enqueue new current entry
#ifndef NDEBUG
		oMsg << "cachePath" << ": Enqueued current product folder in cache:\n";
		wmLog(eDebug, oMsg.str()); oMsg.str("");
		oMsg << oProdList.back() << "\n";
		wmLog(eDebug, oMsg.str()); oMsg.str("");
#endif // #ifndef NDEBUG

		// one new instance, delete oldest instance if already enough instances to keep

		if (oProdList.size() > m_oNbEntriesToKeep)
		{
			const Path	oProductInstance		( oProdList.front() );

			oProdList.pop_front(); // remove element from container
			checkedDeleteRecordedInstance(oProductInstance, m_pathToDeleteMustContain);
		} // if

		writeCache(m_oCacheFilePath, oProdList);
	} // try
	catch(const Exception &p_rException) {
		oMsg << "Record not processed: " << m_pathToAdd << "\n";
		wmLog(eDebug, oMsg.str()); oMsg.str("");
		wmLogTr(eWarning, "QnxMsg.Vdr.FileCmdError", "File handling error at file '%s'. Action: '%s'. See log for details.\n", formatPath(m_pathToAdd).c_str(), "cachePath");
		throw; // handle actual exception in caller 'CommandProcessor::work()'
	} // catch
}

void ProcessRecordCmd::cachePath() const {
    CachePath functor{m_oCacheFilePath, m_oIsLiveMode ? m_oProductDirLiveFinal : m_oFile.path() + std::string{ Path::separator() }, m_oNbEntriesToKeep, g_oWmVideoDir};
    functor();
} // cachePath



UpdateCacheCmd::UpdateCacheCmd(const std::string &cacheFilePath, std::size_t p_oNbEntriesToKeep, const std::string &pathMustContain)
	:
	m_cacheFilePath			{ cacheFilePath },
	m_oNbEntriesToKeep		{ p_oNbEntriesToKeep },
    m_pathMustContain{pathMustContain}

{}



void UpdateCacheCmd::execute()
{
	std::ostringstream oMsg;

	try {
		// selecet cache file and open it
		std::deque<std::string>	oProdList{readCache(m_cacheFilePath)};

		const int	oNbProdInstancesToDelete	=	std::max(oProdList.size() - m_oNbEntriesToKeep, std::size_t(0));	// max: consider manual deletions

		oMsg << "UpdateCacheCmd" << ": Nb of entries to delete: " << oNbProdInstancesToDelete << ", " << m_oNbEntriesToKeep << " entries remaining.\n";
		wmLog(eDebug, oMsg.str()); oMsg.str("");

		poco_assert_dbg(m_oNbEntriesToKeep >= 1); // we want to keep at least one (current) entry

		if(oNbProdInstancesToDelete >= g_oNbProdInstDelWarning) {
			wmLogTr(eWarning, "QnxMsg.Vdr.LongupdateCache", "Automatic cleanup may take a while. Processing %i records.\n", oNbProdInstancesToDelete);
		} // if

		while (oProdList.size() > m_oNbEntriesToKeep) {	// delete old entries until desired number of entries reached
			const Path	oProductInstance		( oProdList.front() );

			oProdList.pop_front(); // remove element from container

			if (checkedDeleteRecordedInstance(oProductInstance, m_pathMustContain) == false)
			{
				break;
			}
		} // while

		writeCache(m_cacheFilePath, oProdList);
	} // try
	catch(const Exception &p_rException) {
		oMsg << "Could not update cache.\n";
		wmLog(eDebug, oMsg.str()); oMsg.str("");
		throw; // handle actual exception in caller 'CommandProcessor::work()'
	} // catch
} // execute

DeleteProductInstanceCmd::DeleteProductInstanceCmd(const std::string &instancePath, const std::string &cacheFilePath, const std::string &pathMustContain)
    : BaseCommand()
    , m_instancePath(instancePath)
    , m_cacheFilePath(cacheFilePath)
    , m_pathMustContain(pathMustContain)
{
}

void DeleteProductInstanceCmd::execute()
{
    try
    {
        if (!checkedDeleteRecordedInstance(m_instancePath, m_pathMustContain))
        {
            return;
        }
        std::deque<std::string> cache{readCache(m_cacheFilePath)};
        auto it = std::find(cache.begin(), cache.end(), m_instancePath);
        if (it == cache.end())
        {
            // not found, try with or without trailing slash
            std::string testString;
            if (m_instancePath.back() == '/')
            {
                testString = m_instancePath.substr(0, m_instancePath.size() - 1);
            } else
            {
                testString = m_instancePath + std::string{"/"};
            }
            it = std::find(cache.begin(), cache.end(), testString);
        }
        if (it != cache.end())
        {
            cache.erase(it);
            writeCache(m_cacheFilePath, cache);
        }
    } catch (const Exception &exception)
    {
        wmLog(eDebug, "Could not delete video.\n");
    }
}

/// free functions


bool checkedDeleteRecordedInstance(const Poco::Path& p_rInstanceToDelete, const std::string &pathMustContain)
{
	std::ostringstream oMsg;

	if (File(p_rInstanceToDelete).exists() == false)
	{
		return true; // file does not exist, maybe manual deletion
	}

	const int	oDesiredDirectoryDepth	( g_oMinDirDepthDelete ); // /wm_inst/video/station/livemode_tmp_78550bc or depth 5: /wm_inst/video/station/prod_type/prod_nb
	const int	oProductInstanceDepth	( p_rInstanceToDelete.depth() );

	if (oProductInstanceDepth < oDesiredDirectoryDepth) {
		oMsg << "Depth of product instance path '" << p_rInstanceToDelete.toString() << "'...\n";
		wmLog(eDebug, oMsg.str()); oMsg.str("");
		oMsg <<	".. is " << oProductInstanceDepth << " instead of " << oDesiredDirectoryDepth << ". Deletion skipped.\n";
		wmLog(eDebug, oMsg.str()); oMsg.str("");
		wmLogTr(eWarning, "QnxMsg.Vdr.FileCmdError", "File handling error at file '%s'. Action: '%s'. See log for details.\n", p_rInstanceToDelete.toString().c_str(), "checkedDeleteRecordedInstance");
		return false;
	} // if
	if (p_rInstanceToDelete.toString().find(pathMustContain) == std::string::npos) {
		oMsg << "Product instance path '" << p_rInstanceToDelete.toString() << "'...\n";
		wmLog(eDebug, oMsg.str()); oMsg.str("");
		oMsg << "... does not contain '" << pathMustContain << "'. Deletion skipped.\n";
		wmLog(eDebug, oMsg.str()); oMsg.str("");
		wmLogTr(eWarning, "QnxMsg.Vdr.FileCmdError", "File handling error at file '%s'. Action: '%s'. See log for details.\n", p_rInstanceToDelete.toString().c_str(), "checkedDeleteRecordedInstance");
		return false;
	} // if

#ifndef NDEBUG
	oMsg <<  "checkedDeleteRecordedInstance" << ": Deleting old product folder:\n";
	wmLog(eDebug, oMsg.str()); oMsg.str("");
	oMsg << "'" << p_rInstanceToDelete.toString() << "'.\n";
	wmLog(eDebug, oMsg.str()); oMsg.str("");
#endif // #ifndef NDEBUG

	try {
		File(p_rInstanceToDelete).remove(true);  // recursively delete whole product folder
	} // try
	catch(const Exception &p_rException) {
		oMsg  <<  "checkedDeleteRecordedInstance|" << p_rException.name() << ": " << p_rException.message() << std::endl;
		wmLog(eDebug, oMsg.str());
		wmLogTr(eWarning, "QnxMsg.Vdr.FileCmdError", "File handling error at file '%s'. Action: '%s'. See log for details.\n", formatPath(p_rInstanceToDelete.toString()).c_str(), "checkedDeleteRecordedInstance");

		return false;
	} // catch

	return true;
}

} // namespace vdr
} // namespace precitec
