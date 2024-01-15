/**
 * 	@file
 * 	@copyright	Precitec Vision GmbH & Co. KG
 * 	@author		Simon Hilsenbeck (HS)
 * 	@date		2012
 * 	@brief		Numeric and string literal definitions.
 */

#include "videoRecorder/literal.h"
#include "event/imageShMem.h"

namespace precitec {
namespace vdr {

const std::string	g_oWmBaseDirEnv				("WM_BASE_DIR");

const std::string	g_oIsShutOff				("IsShutOff");
const std::string	g_oIsEnabled				("IsEnabled");
const std::string	skipIfFull  				("SkipIfFull");
const std::string	g_oNioMode					("NioMode");
const std::string	g_oProductTypeStart			("ProductTypeStart");
const std::string	g_oProductTypeEnd			("ProductTypeEnd");
const std::string	g_oSeamSeriesStart			("SeamSeriesStart");
const std::string	g_oSeamSeriesEnd			("SeamSeriesEnd");
const std::string	g_oSeamStart				("SeamStart");
const std::string	g_oSeamEnd					("SeamEnd");
const std::string	g_oNbProductsToKeep			("NbProductsToKeep");
const std::string	g_oNbLiveModeToKeep			("NbLiveModeToKeep");
const std::string	g_oNbMaxImgLiveKey			("NbMaxImgLive");
const std::string	g_oMaxDiskUsage				("MaxDiskUsage");

const int			g_oLiveModeProdType			(0);
const int			g_oLiveModeProdNumber		(0);
const int			g_oSleepAfterNioSeam		(10);
const int			g_oNbSleepAfterNioSeam		(50);
const int			g_oJoinCmdProcTimespan		(10000);
const int			g_oJoinSeqTransferTimespan	(5 * 60 * 1000); // 5 minutes
const int			g_oJoinUninitTimespan		(5000);
const int			g_oFillWidthDirName			(4);
const int			g_oFillWidthImgName			(5);
const int			g_oPrecisionDiskUsage		(3);
const int			g_oCheckDuEveryNbImg		(100);
const int			g_oNbProdInstDelWarning     (50); // 50 takes already around 5 s
const unsigned int  g_oTransferBlockSize		(10);
#ifdef __QNX__
const unsigned int  g_oPriorityVdr				(9);
const unsigned int  g_oPrioSequenceTransfer		(8);
#elif (defined __linux__)
const unsigned int  g_oPriorityVdr				(0);
const unsigned int  g_oPrioSequenceTransfer		(0);
#endif
// should match nb buffer dma full frame, however too big does not harm, so use a 'small' image size to obtain an upper bound
const unsigned int  g_oMaxQueueSize				(greyImage::ImageShMemSize / (128*128) + 10); // at least 10, prevents 0 or 1
const unsigned int  g_oDirDepthProd				(3); // /wm_inst/video/station/product/
const unsigned int  g_oDirDepthProdInst			(4); // /wm_inst/video/station/product_name/product_instance
const unsigned int  g_oMinDirDepthDelete		(g_oDirDepthProdInst); // /wm_inst/video/station/product_name/product_instance
const unsigned int  g_oMaxNbRecordingsToSearch	(1000); // depends on m_oNbProductsToKeep and m_oNbLiveModeToKeep, should be max 500 + 100

const std::string	g_oVrMountFileName			("mount_video_repo");
const std::string	g_oConfigFileName			("video_recorder");
const std::string	g_oProductCacheFile			(".video_recorder_product_cache");
const std::string	g_oLiveModeCacheFile		(".video_recorder_livemode_cache");
const std::string	g_oLiveModeDefaultDirName	("livemode_tmp_");
const std::string	g_oSequenceInfoFile			("sequence_info");
const std::string	g_oWmBaseDirFallback		("/tmp/precitec/");
const std::string	g_oStationNameKey			("Station.Name");
const std::string	g_oStationNameDefault		("DefaultStationName");
const std::string	g_oLiveModeName				("Live Mode");
const std::string	g_oWmBatchDir				("batch");
const std::string	g_oWmConfigDir				("config");
const std::string	g_oWmVideoDir				("video");
const std::string	g_oStationDir				("WM-QNX-PC");
const std::string	g_oWmDataDir				("data");
const std::string	g_oWmConfigFileName			("video_recorder.xml");
const std::string	g_oSequenceTransferThread	("SequenceTransfer");
const std::string	g_oCommandProcessorThread	("CommandProcessor");

const std::string	g_oQnxScriptExtension		("sh");
const std::string	g_oImageExtension			("bmp");
const std::string	g_oSampleExtension			("smp");
const std::string	g_oXmlExtension				("xml");
const std::string	g_oIdExtension				("id");
const std::string	g_oDateTimeFormatLong		("%y%m%d-%H%M%S%i"); // "061022-185505123"
const std::string	g_oTagProductName			("product_name");
const std::string	g_oTagProductInst			("product_instance");
const std::string	g_oTagDateTime				("date_time");
const std::string	g_oTagIsLiveMode			("is_livemode");
const std::string	g_oTagProductType			("product_type");
const std::string	g_oTagProductUUID			("product_UUID");
const std::string	g_oTagProductNumber			("product_number");
const std::string	g_oTagExtendedProductInfo	("extended_product_info");
const std::string	g_oTagSeamSeries			("seam_series");
const std::string	g_oTagSeam					("seam");
const std::string	g_oTagTriggerDelta			("trigger_delta");
const std::string	g_oTagNbImages				("nb_images");
const std::string	g_oTagNbSamples				("nb_samples");
const std::string	g_oDummyFileName			(".dummy");
const std::string	g_oProdTypeDirName			("product_type");
const std::string	g_oProdNbDirName			("product_number");
const std::string	g_oSeamSeriesDirName		("seam_series");
const std::string	g_oSeamDirName				("seam");
} // vdr
} // precitec

