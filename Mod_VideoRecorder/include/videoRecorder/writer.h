/**
 * 	@file
 * 	@copyright	Precitec Vision GmbH & Co. KG
 * 	@author		Simon Hilsenbeck (HS)
 * 	@date		2012
 * 	@brief		Manages and executes image sequence writing.
 */

#ifndef WRITER_H_20120214_INCLUDED
#define WRITER_H_20120214_INCLUDED

// stl includes
#include <string>
#include <map>
#include <atomic>
// Poco includes
#include "Poco/Path.h"
// project includes
#include "event/videoRecorder.h"
#include "message/grabberStatus.proxy.h"
#include "event/schedulerEvents.proxy.h"
#include "videoRecorder/parameter.h"
#include "videoRecorder/commandProcessor.h"
#include "videoRecorder/types.h"


namespace precitec {
namespace vdr {


typedef interface::TGrabberStatus<interface::AbstractInterface>		grabberStatusProxy_t;
typedef interface::TSchedulerEvents<interface::AbstractInterface>	schedulerEventsProxy_t;


/**
 * @brief	Manages and executes image sequence writing.
 */
class Writer {
public:
	/**
	 * @brief CTor.
	 */
	Writer(grabberStatusProxy_t &p_rGabberStatusProxy, schedulerEventsProxy_t &p_rSchedulerEventsProxy, Parameter &p_rParameter);

	/**
	 * @brief 			Parametrizes and starts the writing thread with 'record' method.
	 * @return	void
	 */
	void parametrize();

	/**
	 * @brief	Set ProductInstData. Called in live mode start or automatic start. Also sets the product folder.
	 * @param	p_rProductInstData		ProductInstData.
	 * @return	void
	 */
	void setProductInstData(const interface::ProductInstData &p_rProductInstData);

	/**
	 * @brief	Get ProductInstData. Called on automatic start.
	 * @param	p_rProductInstData		ProductInstData.
	 * @return	ProductInstData.
	 */
	const interface::ProductInstData& getProductInstData() const;

	/**
	 * @brief 	Sets a temporaty folder for live mode. Renamed and moved on live mode end. Also sets product instance id.
	 * @param	p_rProductInstId	Product instance id.
	 * @param	p_oTriggerDelta		Trigger delta / trigger interval [us].
	 * @return	void
	 */
	void setFinalProductInstDirLiveAndProdInstId(const Poco::UUID& p_rProductInstId, int p_oTriggerDelta);

	/**
	 * @brief	Setter for seam data. Also sets the seam directory. Called on seam start.
	 * @param	p_oSeamData		Seam series and seam number.
	 * @return	void
	 */
	void setSeamData(interface::SeamData p_oSeamData);

	/**
	 * @brief 	Depending on seam data information it composes folder names.
	 * 			Then it creates ther folder tree.
	 * 			These folders are used in a folder tree that serves as recording repository.
	 * @return	void
	 */
	void createDirectories();

	/**
	 * @brief 							Insert image in write queue. If the queue is full, a warning is emitted.
	 * @param	p_rVdrImage				Image and vdr context.
	 * @return	bool					If insertion succeeded.
	 */
	bool insertWriteQueue(const vdrImage_t &p_rVdrImage);

	/**
	 * @brief 							Insert sample in write queue. If the queue is full, a warning is emitted.
	 * @param	p_rVdrSample			Sample and vdr context.
	 * @return	bool					If insertion succeeded.
	 */
	bool insertWriteQueue(const vdrSample_t &p_rVdrSample);

	/**
	 * @brief 			Displays number of recorded, lost and skipped (write fail) images. Does nothing if recording is disabled.
	 * @return	void
	 */
	void printStatus() const;

	/**
	 * @brief 			Erases empty folders. May take up to 20 ms. Resets AutoFoldersCreated flag.
	 * @return	void
	 */
	void processRecord();

	/**
	 * @brief	Joins running threads, writes configuration, cleanup.
	 * @return void
	 */
	void uninitialize();

	/**
	 * @brief	Setter if mode is live mode. Live mode false means always automatic mode active.
	 * @param	p_oIsLiveMode		If mode is live mode.
	 * @return	void
	 */
	void setIsLiveMode(bool p_oIsLiveMode);

	/**
	 * @brief	Getter if mode is live mode.
	 * @return	bool				If mode is live mode.
	 */
	bool getIsLiveMode() const;

	/**
	 * @brief	Resets seam series counter.
	 * @return	void
	 */
	void resetSeriesCounter();

	/**
	 * @brief	Sets interruption flag of record thread.
	 * @return	void
	 */
	void setInterruption();

	/**
	 * @brief	Resets interruption flag of record thread.
	 * @return	void
	 */
	void resetInterruption();

	/**
	 * @return	void
	 */
	void writeSequenceInfo() const;

	/**
	 * @brief							Getter if automatic mode folders were created.
	 * @return	bool					If automatic mode folders were created.
	 */
	bool getAutoFoldersCreated() const;

	/**
	 * @brief							Setter if automatic mode folders were created.
	 * @param	p_oAutoFoldersCreated	If automatic mode folders were created.
	 * @return	void
	 */
	void setAutoFoldersCreated(bool p_oAutoFoldersCreated);

	/**
	 * @brief							Checks if the command queue is still busy.
	 * @return	bool					If the command queue is still busy.
	 */
	bool checkIfBusy() const;

	/**
	 * @brief 							Updates cache for cyclic deletion depending on settings.
	 * 									If number of sequences to keep was decreased, cache gets updated
	 * @param	p_oIsLiveMode			If live mode cache (true) or automatic mode cache (false) is updated
	 */
	void updateCache(bool p_oIsLiveMode) const;

    /**
     * Deletes the product instance at @p path and removes from cache
     * @param liveMode If live mode cache (@c true) or automatic mode cache (@c false) is updated
     **/
    void deleteProductInstance(bool liveMode, const std::string &path);

private:

	/**
	 * @brief 							Generate bmp file path (absolute directory, filename and extension) from seam directory and image number.
	 * @param	p_oImageNumber			Image number.
	 * @param	p_rExtension			File extension.
	 * @return	std::string				File path (absolute directory, filename and extension)
	 */
	std::string makeFilePath(unsigned int p_oImageNumber, const std::string& p_rExtension) const;

    std::string cacheFilePath(bool liveMode) const;

    void createDirectory(const Poco::Path &path);

	CommandProcessor::EnqueueMode parameterToEnqueueMode() const;

	typedef std::map<Poco::UUID, std::string>						uuid_string_map_t;
	typedef uuid_string_map_t::value_type							uuid_string_pair_t;

	const 	Poco::Path						m_oWmBaseDir;				///< Base directory of the weldmaster installation

	grabberStatusProxy_t 					&m_rGabberStatusProxy;  	///< send requests to grabber
	schedulerEventsProxy_t 					&m_rSchedulerEventsProxy;  	///< send events to a scheduler
	Parameter								&m_rParameter;				///< Recording parameter. Hold in the video recorder.
	Poco::Path								m_oConfigDir;				///< Directory of the xml configuration file.
	Poco::Path								m_oStationDir;				///< Station directory. Composed of wm base dir and station name.
	Poco::Path								m_oProductInstDirAuto;		///< Directory for automatic product. Composed of m_oStationDir, product name and number, timestamp
	Poco::Path								m_oProductInstDirLive;		///< Directory for live mode product. Composed of m_oStationDir, product name and number, timestamp
	Poco::Path								m_oProductInstDirLiveFinal;	///< Final folder for live mode.
	Poco::Path								m_oSeamDir;					///< Directory for images of current seam. Composed of m_oProductInstDir and m_oSeamData.
	bool									m_oIsLiveMode;				///< if live mode state is active
	bool									m_oAutoFoldersCreated;		///< if automatic mode folders were created
	mutable std::atomic<bool>				m_oIsRecordInterrupted;		///< if image number in write queue lies out of grabber buffer
	interface::SeamData						m_oSeamData;				///< Seam data for current inspection cycle, which is set on seam start.
	interface::ProductInstData				m_oProductInstData;			///< product and seam data for folder creation and xml
	interface::ProductInstData				m_oProductInstDataLive;		///< live mode product data
	Counters								m_oSeriesCounters;			///< Counters that are reset after each seam / recording cycle.
	std::chrono::time_point<std::chrono::system_clock> m_startTime;

	mutable CommandProcessor				m_oFileCmdProcessor;		///< processes file operations concurrently.
}; // Writer



/**
 * @brief 	Generates a directory name that consists of a label, a number and a timestamp of current time if label is product number.
 * @param	p_rLabel			Label to form the first part of a directory name.
 * @param	p_oNumber			Number to form the second part of a directory name.
 * @return	std::string
 */
std::string makeEnumeratedRepoDirName(const std::string &p_rLabel, int p_oNumber);



/**
 * @brief 	Generates product instance data for live mode instances using a random id.
 * @return	ProductInstData
 */
interface::ProductInstData makeLivemodeInstance();

} // namespace vdr
} // namespace precitec

#endif /* WRITER_H_20120214_INCLUDED */
