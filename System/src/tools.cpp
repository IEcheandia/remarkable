/*!
 *  @copyright		Precitec Vision GmbH & Co. KG
 *  @author			Simon Hilsenbeck (HS)
 *  @date			2011
 *  @file
 *  @brief			Helper, debug, utility code.
 */

#include "system/tools.h"

// stl
#include <iostream>
#include <string>
#include <sstream>
#include <fstream>
#include <cstring>
// local includes
#include "module/moduleLogger.h"
#include "common/logMessage.h"
#include "system/templates.h"
#include "system/exception.h"
#include "system/timer.h"
#include "Poco/DirectoryIterator.h"
// os includes
#if defined __QNX__ || defined __linux__
	#include <sys/statvfs.h>			///< Get filesystem information, given a path
	#include <ftw.h>
	#include <fcntl.h>
	#include <glob.h>	
	#include <unistd.h>
	#include <string.h>					///< strerror
#endif // defined(__QNX__||__linux__)

using namespace Poco;
using Poco::UUID;

namespace precitec {
namespace system {

std::size_t getNbFilesMatchingPattern(const std::string &p_rPattern) {
#if defined __QNX__ || defined __linux__
	glob_t oGlob;
	std::size_t oNbFilesMatching = 0;
	const int oRetVal = glob(p_rPattern.c_str(), GLOB_NOSORT, NULL, &oGlob);
	if (oRetVal != 0 && oRetVal != 3/*pattern not found*/) {
		wmLog(eError, "%s: 'glob' returned %i.\n", __FUNCTION__, oRetVal);
		wmLog(eDebug, "%s: 'glob' arg: '%s'.\n", __FUNCTION__, formatPath(Path{ p_rPattern }).c_str());
		return oNbFilesMatching;
	} // if
	oNbFilesMatching	=	oGlob.gl_pathc;
	globfree(&oGlob);
	return oNbFilesMatching;
#else // NOT __QNX__||__linux__
	throw Poco::Exception("Only QNX platform supported.");
	return 0;
#endif // __QNX__||__linux__
} // getNbFilesMatchingPattern



double getDiskUsage(const std::string &p_rPath) {
#if defined __QNX__ || defined __linux__
	struct statvfs64 oBuffer;
	const int oRetVal = statvfs64( p_rPath.c_str(), &oBuffer );
	if (oRetVal != 0) {
		wmLog(eError, "%s: 'statvfs64' returned %i.\n", __FUNCTION__, oRetVal);
		return 0;
	} // if
//	std::cout << "filesystem block size: " << oBuffer.f_bsize << ".\n";
//	std::cout << "fundamental filesystem block size (f_frsize): " << oBuffer.f_frsize << ".\n";
//	std::cout << "total number of blocks on file system in units of f_frsize / f_frsize: " << oBuffer.f_blocks / oBuffer.f_frsize << ".\n";
//	std::cout << "total number of free blocks: " << oBuffer.f_bfree << ".\n";
//	std::cout << "number of free blocks available to non-privileged process: " << oBuffer.f_bavail << ".\n";

	poco_assert_dbg(oBuffer.f_blocks != 0);
	const double oDu	= double(oBuffer.f_blocks - oBuffer.f_bavail) / double(oBuffer.f_blocks);
	poco_assert_dbg(0.0 <= oDu && oDu <= 1.0);
	return oDu;
#else // NOT __QNX__||__linux__
	throw Poco::Exception("Only QNX platform supported.");
	return 0;
#endif // __QNX__||__linux__
} // getDiskUsage

File::FileSize getDirSizePoco(const std::string &p_rPath) {
	File::FileSize			oTotalSize	(0);
	DirectoryIterator		oIt			(p_rPath);
	const DirectoryIterator	oEndIt;
	while (oIt != oEndIt) { // all files and folders within folder
		if (oIt->isFile() ) {
			oTotalSize	+= oIt->getSize(); // byte 
		}
		else {
			oTotalSize	+= getDirSizePoco( oIt->path() );
		}
		++oIt; // next file / folder
	} // while

	return	oTotalSize;
} // getDirSizePoco


static unsigned int g_oTotalSize		( 0 );
int sumFileSize(const char *p_pFilePath, const struct stat64 *p_pSBuffer, int p_oTypeflag) {
#if defined __QNX__ || defined __linux__
	g_oTotalSize += p_pSBuffer->st_size;
    return 0;

#else // NOT __QNX__||__linux__
	throw NotSupportedException();
	return 0;
#endif // __QNX__||__linux__
} // sumFileSize



std::size_t getDirSizeSys(const std::string &p_rPath) {
#if defined __QNX__ || defined __linux__
	g_oTotalSize	= 0;
	const int			oRetVal		( ftw64(p_rPath.c_str(), &sumFileSize, 8/*TO TEST*/) );
	if (oRetVal != EXIT_SUCCESS) {
		wmLog(eError, "%s: 'ftw64' returned %i.\n", __FUNCTION__, oRetVal);
		return 0;
	} // if
	return g_oTotalSize; //

#else // NOT __QNX__||__linux__
	throw NotSupportedException();
	return 0;
#endif // __QNX__||__linux__
} // getDirSizeSys



int cp(const Poco::Path &p_rSource, const Poco::Path &p_rDestination) {
#if defined __QNX__ || defined __linux__

	try { // executed within thread, all exceptions must be caught
		if ( File( p_rSource ).exists() == false) {
			std::ostringstream oMsg;
			oMsg << __FUNCTION__ << ": Source file / directory does not exist:" << "\n";
			wmLog(eDebug, oMsg.str()); oMsg.str(""); // calling side will send a general, localized warning to user
			oMsg << "'" << p_rSource.toString()  << "'\n";
			wmLog(eDebug, oMsg.str());

			return EXIT_FAILURE;
		} // if

		File	oDestDir	( p_rDestination );
		if (p_rDestination.isDirectory() == true) {
			// ok
		} else {
			std::ostringstream oMsg;
			oMsg << __FUNCTION__ << ": Not a directory: '" << p_rDestination.toString() << "'\n";
			throw Poco::Exception(oMsg.str());

		} // else

		if ( oDestDir.exists() == false) {
			oDestDir.createDirectories();
			std::ostringstream oMsg;
			oMsg << __FUNCTION__ << ": Directory created:" << "\n";
			wmLog(eDebug, oMsg.str()); oMsg.str("");
			oMsg << "'" << oDestDir.path()  << "'\n";
			wmLog(eDebug, oMsg.str());
		} // if

		Path	oDestWithSourceFile	( p_rDestination );
		oDestWithSourceFile.setFileName( p_rSource.getBaseName() );
		oDestWithSourceFile.setExtension( p_rSource.getExtension() );

		const std::string	&rSource	( p_rSource.toString() );
		const std::string	&rDest		( oDestWithSourceFile.toString() );
		const char *		from		( rSource.c_str() );
		const char *		to			( rDest.c_str() );

		int fd_to, fd_from;
		char buf[4096]; // depends on L1 cache size, see http://stackoverflow.com/questions/7463689/most-efficient-way-to-copy-a-file-in-linux?lq=1
		ssize_t nread;

		fd_from = open(from, O_RDONLY);
		if (fd_from < 0) {
			std::ostringstream oMsg;
			oMsg << __FUNCTION__ << ": '" << from << "'.\n";
			wmLog(eDebug, "%s\n", oMsg.str().c_str()); oMsg.str("");
			oMsg << __FUNCTION__ << ": 'errno " << errno << "': " << std::strerror(errno) << "'.\n";
			wmLog(eDebug, "%s\n", oMsg.str().c_str());
			throw Poco::Exception(oMsg.str());
		}

		fd_to = open(to, O_WRONLY | O_CREAT | O_TRUNC, 0666); // 0666 - rw, see chmod docu
		if (fd_to < 0) {
			close(fd_from);
			if (fd_to >= 0) {
				close(fd_to);
			} // if

			std::ostringstream oMsg;
			oMsg << __FUNCTION__ << ": '" << to << "'.\n";
			wmLog(eDebug, "%s\n", oMsg.str().c_str()); oMsg.str("");
			oMsg << __FUNCTION__ << ": 'errno " << errno << "': " << std::strerror(errno) << "'.\n";
			wmLog(eDebug, "%s\n", oMsg.str().c_str());
			throw Poco::Exception(oMsg.str());
		}

		while (nread = read(fd_from, buf, sizeof buf), nread > 0) {
			char *out_ptr = buf;
			ssize_t nwritten	(0);

			do {
				nwritten = write(fd_to, out_ptr, nread);

				if (nwritten >= 0) {
					nread -= nwritten;
					out_ptr += nwritten;
				}
				else if (errno != EINTR) {
					if (close(fd_to) < 0) {
						fd_to = -1;
					}
					close(fd_from);
					std::ostringstream oMsg;
					oMsg << __FUNCTION__ << ": 'errno " << errno << "': " << std::strerror(errno) << "'.\n";
					throw Poco::Exception(oMsg.str());
				}
			} while (nread > 0);
		} // while

		if (nread == 0) {
			if (close(fd_to) >= 0) {
				close(fd_to);
			}
			close(fd_from);
			return EXIT_SUCCESS;
		}

		close(fd_from);
		if (fd_to >= 0) {
			close(fd_to);
		} // if

		std::ostringstream oMsg;
		oMsg << __FUNCTION__ << ": 'nread' is '" << nread << "' instead of 0.\n";
		throw Poco::Exception(oMsg.str());

	} catch(const Poco::Exception &p_rException) {
		std::ostringstream oMsg;
		oMsg  << __FUNCTION__ << " - " << p_rException.name() << "': " << p_rException.what() << " - " << p_rException.message();
		wmLog(eDebug, oMsg.str());
		return EXIT_FAILURE;
	} // catch
	catch(...) { // swallow exception
		std::ostringstream oMsg;
		oMsg  << __FUNCTION__ << ": Unknow exception caught.\n";
		wmLog(eDebug, oMsg.str());
		return EXIT_FAILURE;
	} // catch

	return EXIT_FAILURE; // non void fun warning
#else // NOT __QNX__||__linux__
	throw NotSupportedException();
	return 0;
#endif // __QNX__||__linux__
} // cp

std::string wmBaseDir() {
	return getenv("WM_BASE_DIR") ? std::string(getenv("WM_BASE_DIR")) : "";
}

/// Returns file as XML file, p_rFileStr will hold the name of the file on successful read and be empty otherwise.
bool readXMLConfigToString(std::string &p_rFileStr, const std::string oTheFile)
{
	// returns filename or emtpy string

	std::string oLine("");
	p_rFileStr = "";

	std::ifstream oFle(oTheFile, std::ios::in);
	if ( oFle.is_open() )
	{
		while ( std::getline(oFle, oLine) )
		{
			p_rFileStr += oLine;
		}
		oFle.close();
	} else
	{
		wmLogTr(eError, "QnxMsg.Calib.NoCalibFile", "Cannot access calibration file %s!", oTheFile.c_str());
		return false;
	}
	return true;
}



std::string formatPath(const Path p_rPath, unsigned int p_oDirDepthEnd) {
	if (static_cast<unsigned int>(p_rPath.depth()) <= p_oDirDepthEnd) {
		return p_rPath.toString();
	} // if
	const auto	oNbDirsToPopFront	= p_rPath.depth() - p_oDirDepthEnd;
	auto	oShortPath	=	p_rPath;
	for (auto oI = 0u; oI < oNbDirsToPopFront; ++oI) {
		oShortPath.popFrontDirectory();
	} // for
	oShortPath.setDevice(""); // remove drive lettter
	return std::string{ "..." }.append(oShortPath.toString());
} // formatPath



UUID string2Uuid(const std::string& p_rIdIn) {
	UUID			oIdOut;
	try {
		oIdOut	= UUID( p_rIdIn );
	} // try
	catch(const Exception &p_rException) {
		wmLog(eDebug, "%s: '%s' - %s\n", __FUNCTION__, p_rException.name(), p_rException.message().c_str());
		wmLogTr(eWarning, "QnxMsg.Vdr.ProcNonCritical", "A non-critcal  error occurred in the procedure '%s'. Message: '%s'. See log file.\n",
				__FUNCTION__, p_rException.message().c_str());
	} // catch
	return oIdOut;
} // string2Uuid



void logExcpetion(const std::string& p_rCallerName, std::exception_ptr p_oExceptionPtr) {
    try {		
        if(p_oExceptionPtr == std::exception_ptr{}) {
        	wmLog(eError, "Exception caught in '%s'. Exception pointer invalid. A thread boundary might have been crossed.\n", p_rCallerName.c_str());
        	return;
        }
        std::rethrow_exception(p_oExceptionPtr);
    } // try
	catch(Poco::Exception& p_rException) { // fliplib exceptions like the GraphBuilderException are all poco exceptions
		wmLog(eError, "Poco exception caught in '%s':\n", p_rCallerName.c_str());
		if (p_rException.displayText().empty() == false) {
			splitLog( eError, p_rException.displayText() );
		} // if
		if (p_rException.nested()) {
			const auto& rNestedExcep	=	*p_rException.nested();
			if (rNestedExcep.displayText().empty() == false) {
				splitLog( eError, rNestedExcep.displayText() );
			} // if
		} // if
	} // catch	
	catch (std::exception& p_rException) {
		wmLog(eError, "Standard exception caught in '%s':\n", p_rCallerName.c_str());
		std::string oExceptionMessage(p_rException.what());
		std::cerr << "std::exception: " << oExceptionMessage << "\n"; // DEBUG
		splitLog(eError, oExceptionMessage);
	} // catch	
	catch (...) {
		wmLog(eError, "Unknown exception caught in '%s':\n", p_rCallerName.c_str());
		std::cerr << "Unknown exception caught in '" << p_rCallerName << "'\n"; // DEBUG
	} // catch
} // logExcpetion



void splitLog(LogType p_oLogType, const std::string& p_rMsg) {
	const auto	oChunkLenTotal	=	LogMessageLength - 26/*mod name, timestamp, ...*/;	// from Interfaces/logMessage.h
	const auto	oMakePrefix		=	[](std::size_t p_Nb, std::size_t p_TotalNb) { 
		return std::string{ "(part " + std::to_string(p_Nb) + " / " +  std::to_string(p_TotalNb) + ") " };
	};
	const auto	oChunkLen		=	oChunkLenTotal - oMakePrefix(1, 1).length();
	const auto	oNbChunks		=	(p_rMsg.length() - 1 + oChunkLen) / oChunkLen;

	if (oNbChunks == 1)
	{
		wmLog(p_oLogType, p_rMsg + "\n");
		return;
	}

	unsigned oMaxChar = std::min(p_rMsg.length(), 5 * oChunkLen); //do not flood the logger
	for ( unsigned i = 0; i < oMaxChar; i += oChunkLen )
	{
		wmLog(p_oLogType, oMakePrefix((i + oChunkLen) / oChunkLen, oNbChunks) + p_rMsg.substr(i, oChunkLen) + "\n");
	} // for
} // splitLog

} // namespace system
} // namespace precitec

