/*!
 *  @copyright		Precitec Vision GmbH & Co. KG
 *  @author			Simon Hilsenbeck (HS)
 *  @date			2011
 *  @file
 *  @brief			Helper, debug, utility code.
 */


#ifndef TOOLS_H
#define TOOLS_H


// local includes
#include "SystemManifest.h"				///< dll export
#include "module/logType.h"
// poco includes
#include "Poco/Path.h"
#include "Poco/File.h"
#include "Poco/UUID.h"
// stl includes
#include <string>						///< string
#include <exception>

#include <unistd.h>

namespace precitec {
namespace system {


/**
 * @brief	Get number of files matching pattern.
 * @param	p_rPattern	Unix file path pattern including wildcards like '*'.
 * @return	Number of files matching pattern.
 * @sa		http://www.qnx.com/developers/docs/6.3.0SP3/neutrino/lib_ref/g/glob.html?lang=de
 */
SYSTEM_API std::size_t getNbFilesMatchingPattern(const std::string &p_rPattern);

/**
 * @brief	Provides relative disk usage for the file system that contains the file named by the path argument.
 * @param	p_rPath	Unix file path.
 * @return	double	Value between 0.0 and 1.0 that represents the relative amount of used disk space.
 * @sa		http://www.qnx.com/developers/docs/6.3.0SP3/neutrino/lib_ref/s/statvfs.html
 */
SYSTEM_API double getDiskUsage(const std::string &p_rPath);

/**
 * @brief	Provides size of folder content. SLOW.
 * @param	p_rPath		Path to directory.
 * @return	std::size_t	Amount of used disk space in bytes.
 */
SYSTEM_API Poco::File::FileSize getDirSizePoco(const std::string &p_rPath);

/**
 * @brief	Provides size of folder content.
 * @param	p_rPath		Path to directory.
 * @return	std::size_t	Amount of used disk space in bytes.
 */
SYSTEM_API std::size_t getDirSizeSys(const std::string &p_rPath);

/**
 * @brief	Copies a file to a directory. Creates directories if they do not exist.
 * @param	p_rSource			Source path.
 * @param	p_rDestination		Destination path.
 * @return	error status. -1 on failure.
 */
SYSTEM_API int cp(const Poco::Path &p_rSource, const Poco::Path &p_rDestination);

/**
 * @brief   Read xml file
 */
SYSTEM_API bool readXMLConfigToString(std::string &p_rFileContent, const std::string pFileName); 

/**
 * @brief  Uses OS's command "pwd" to read current directory. pwd works under linux, qnx, OS X, win 7.
 */
SYSTEM_API std::string wmBaseDir();

/**
 * @brief	Debug format helper. Shortens a Path to a given directory depth and converts it to string.
 * @param   p_rPath					Path to be formatted. 
 * @param   p_oDirDepthEnd          Directory depth the path is shortened from the end. 
 * @return  Shortened path.
 */
SYSTEM_API std::string formatPath(const Poco::Path p_rPath, unsigned int p_oDirDepthEnd = 2);

/**
 * @brief	Tries to convert a string to a uuid.
 * @param	p_rIdIn		String to parse.
 * @return	UUID if string could be converted.
 */
SYSTEM_API Poco::UUID string2Uuid(const std::string& p_rIdIn);


/**
 * @brief	Logs different kind of exceptions.
 * @param	p_rFunction		Name of caller.
 * @param	p_oExceptionPtr	Exception pointer.
 */
SYSTEM_API void logExcpetion(const std::string& p_rCallerName, std::exception_ptr p_oExceptionPtr);

/**
 * @brief	Logs a message and splits it up into several log messages if needed.
 * @param	p_oLogType		Type of log message.
 * @param	p_rMsg			Message to be logged.
 */
SYSTEM_API void splitLog(LogType p_oLogType, const std::string& p_rMsg);
} // namespace system
} // namespace precitec


#endif // #ifndef TOOLS_H
