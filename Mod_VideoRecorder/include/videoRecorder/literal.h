/**
 * 	@file
 * 	@copyright	Precitec Vision GmbH & Co. KG
 * 	@author		Simon Hilsenbeck (HS)
 * 	@date		2012
 * 	@brief		Numeric and string literal definitions.
 */

#ifndef LITERAL_H_20120605_INCLUDED
#define LITERAL_H_20120605_INCLUDED

#include <string>

namespace precitec {
namespace vdr {

extern const std::string	g_oWmBaseDirEnv;			///< name of wm base dir environment variable

// parameter related literals

extern const std::string	g_oIsShutOff;				///< parameter name
extern const std::string	g_oIsEnabled;				///< parameter name
extern const std::string	skipIfFull;				///< parameter name
extern const std::string	g_oNioMode;					///< parameter name
extern const std::string	g_oProductTypeStart;		///< parameter name
extern const std::string	g_oProductTypeEnd;			///< parameter name
extern const std::string	g_oSeamSeriesStart;			///< parameter name
extern const std::string	g_oSeamSeriesEnd;			///< parameter name
extern const std::string	g_oSeamStart;				///< parameter name
extern const std::string	g_oSeamEnd;					///< parameter name
extern const std::string	g_oNbProductsToKeep;		///< parameter name
extern const std::string	g_oNbLiveModeToKeep;		///< parameter name
extern const std::string	g_oNbMaxImgLiveKey;			///< parameter name
extern const std::string	g_oMaxDiskUsage;			///< parameter name

extern const int			g_oLiveModeProdType;		///< live mode product type
extern const int			g_oLiveModeProdNumber;		///< live mode product number
extern const int			g_oSleepAfterNioSeam;		///< sleep time span [ms]
extern const int			g_oNbSleepAfterNioSeam;		///< maximum number of sleep cycles
extern const int			g_oJoinCmdProcTimespan;		///< timespan used in tryJoin [us]
extern const int			g_oJoinSeqTransferTimespan;	///< timespan used in tryJoin [us]
extern const int			g_oJoinUninitTimespan;		///< timespan used in tryJoin [us]
extern const int			g_oFillWidthDirName;		///< Fill width for number to string conversion in repo dir name generation.
extern const int			g_oFillWidthImgName;		///< Fill width for number to string conversion in image file name generation.
extern const int			g_oPrecisionDiskUsage;		///< precision of disk usage percentage.
extern const int			g_oCheckDuEveryNbImg;		///< check disk usage every nth image.
extern const int			g_oNbProdInstDelWarning;	///< Minimum number of product instances to be deleted, that triggers a warning.
extern const unsigned int	g_oTransferBlockSize;		///< Number of images that are transferred per transfer request.
extern const unsigned int	g_oPriorityVdr;				///< Default video recorder thread priority. Lies just below default priority, to not interfer with inspection.
extern const unsigned int	g_oPrioSequenceTransfer;	///< Video recorder thread priority for sequence info and transfer. Lower than vdr prio to not interfer with recording.
extern const unsigned int	g_oMaxQueueSize;			///< Maximum size of the command queue. Depends on the DMA cache size, because the image command references img pointers.
extern const unsigned int	g_oDirDepthProd;			///< Directory depth of a product foler.
extern const unsigned int	g_oDirDepthProdInst;		///< Directory depth of a product instance foler.
extern const unsigned int	g_oMinDirDepthDelete;		///< Minimal directory depth of a product foler to be deleted.
extern const unsigned int	g_oMaxNbRecordingsToSearch;	///< Maximal number of recordings to be searched in repository.

extern const std::string	g_oVrMountFileName;			///< name of batch file wo extension
extern const std::string	g_oConfigFileName;			///< name of xml configuration file wo extension
extern const std::string	g_oProductCacheFile;		///< hidden cache file name for auto deletion of product instance folders
extern const std::string	g_oLiveModeCacheFile;		///< hidden cache file name for auto deletion of product instance folders
extern const std::string	g_oLiveModeDefaultDirName;	///< folder name where live mode writes images. Renamed after recording and recreated.
extern const std::string	g_oSequenceInfoFile;		///< intialization of class 'Writer'
extern const std::string	g_oWinVideoRepo;			///< path of video repository on win side
extern const std::string	g_oWmBaseDirFallback;		///< intialization of class 'Writer'
extern const std::string	g_oStationNameKey;			///< key of station name entry in Connect.config
extern const std::string	g_oStationNameDefault;		///< default station name
extern const std::string	g_oLiveModeName;			///< live mode product name
extern const std::string	g_oWmBatchDir;				///< intialization of class 'Writer'
extern const std::string	g_oWmConfigDir;				///< intialization of class 'Writer'
extern const std::string	g_oWmVideoDir;				///< intialization of class 'Writer'
extern const std::string	g_oStationDir;				///< name of station directory within video repo
extern const std::string	g_oWmDataDir;				///< intialization of class 'Writer'
extern const std::string	g_oWmConfigFileName;		///< intialization of class 'Writer'
extern const std::string	g_oSequenceTransferThread;	///< thread name
extern const std::string	g_oCommandProcessorThread;	///< thread name

extern const std::string	g_oQnxScriptExtension;		///< qnx shell script extension (format)
extern const std::string	g_oImageExtension;			///< image extension (format)
extern const std::string	g_oSampleExtension;			///< sample extension (format). See common/sample.h
extern const std::string	g_oXmlExtension;			///< xml extension (format)
extern const std::string	g_oIdExtension;				///< id extension (custom: uuid is saved as filename in an empty .id file)
extern const std::string	g_oDateTimeFormatLong;		///< date time format according to poco date time formatter  http://pocoproject.org/docs/Poco.DateTimeFormatter.html
extern const std::string	g_oTagProductName;			///< tag for xml file
extern const std::string	g_oTagProductInst;			///< tag for xml file
extern const std::string	g_oTagDateTime;				///< tag for xml file
extern const std::string	g_oTagIsLiveMode;			///< tag for xml file
extern const std::string	g_oTagProductType;			///< tag for xml file
extern const std::string	g_oTagProductUUID;			///< tag for xml file
extern const std::string	g_oTagProductNumber;		///< tag for xml file
extern const std::string	g_oTagExtendedProductInfo;	///< tag for xml file
extern const std::string	g_oTagSeamSeries;			///< tag for xml file
extern const std::string	g_oTagSeam;					///< tag for xml file
extern const std::string	g_oTagTriggerDelta;			///< tag for xml file
extern const std::string	g_oTagNbImages;				///< tag for xml file
extern const std::string	g_oTagNbSamples;			///< tag for xml file
extern const std::string	g_oDummyFileName;			///< name of dummy file that is temporally created for write testing
extern const std::string	g_oProdTypeDirName;			///< directory label of product type directory
extern const std::string	g_oProdNbDirName;			///< directory label of product number directory
extern const std::string	g_oSeamSeriesDirName;		///< directory label of seam series directory
extern const std::string	g_oSeamDirName;				///< directory label of seam directory
} // vdr
} // precitec

#endif /* LITERAL_H_20120605_INCLUDED */
