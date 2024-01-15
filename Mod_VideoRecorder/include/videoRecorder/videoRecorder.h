/**
 * 	@file
 * 	@copyright	Precitec Vision GmbH & Co. KG
 * 	@author		Simon Hilsenbeck (HS)
 * 	@date		2011
 * 	@brief		Manages and executes video recording.
 */

#ifndef VIDEORECORDER_H_
#define VIDEORECORDER_H_

#include <cstdint>
// stl includes
#include <string>
// Poco includes
#include "Poco/Mutex.h"
// project includes
#include "Mod_VideoRecorder.h"
#include "image/image.h"
#include "event/videoRecorder.h"
#include "common/seamData.h"
#include "message/device.h"
#include "message/grabberStatus.interface.h"
#include "event/schedulerEvents.proxy.h"
#include "image/image.h"
#include "image/ipSignal.h"
#include "videoRecorder/parameter.h"
#include "videoRecorder/writer.h"


namespace precitec {


/**
 * @brief In this namespace all the videoRecorder related classes are organized - the VideoRecorder is the main class that controls and executes the video recording.
 */
namespace vdr {


/**
 * @ingroup VideoRecorder
 * @brief Executes and provides a basic framework for image recording.
 *
 * @details The VideoRecorder class is the central instance in the VideoRecorder module. A single VideoRecorder object is created in App_VideoRecorder.
 * VideoRecorder writes image sequences in BMP-format to a folder in '($WM_BASE_DIR)/video/'. Depending on product information a file hirarchy is created.
 * The name of the sequence folder is composed of current station name, product type and number, a date time string, seamseries and  seam number.
 * videoRecorderServer.h implements the 'TVideoRecorder' interface as source for images with recorder specific product information.
 * All images to be recorded are referenced in a FIFO queue. The disk writing of these images is executed in a separate low-priority thread located in the 'Writer' class.
 */
class MOD_VIDEORECORDER_API VideoRecorder {

public:
	typedef interface::TGrabberStatus<interface::AbstractInterface>				grabberStatusProxy_t;
	typedef interface::TSchedulerEvents<interface::AbstractInterface>			schedulerEventsProxy_t;

	/**
	 * @brief	CTor.
	 * @param	p_rGabberStatusProxy			Proxy, which sends requests to grabber.
	 * @param	p_rPublishRecordStatusProxy		Proxy, which signals product record to windows.
	 * @param	p_rSchedulerEventsProxy			Proxy, which sends events to a scheduler.
	 */
	VideoRecorder(grabberStatusProxy_t &p_rGabberStatusProxy, schedulerEventsProxy_t &p_rSchedulerEventsProxy);

	/**
	 * @brief 	The function is called from the InspectionCmd handler.
	 * @param	p_rProductInstId	Product instance id.
	 * @return	void
	 */
	void onLivemodeStart(const interface::ProductInstData& p_rProductInstData);

	/**
	 * @brief 	The function is called from the automaticStart handler. All product information from task cache is transferred and
	 * 			avaialable for creating a folder tree that serves as recording repository. A small delay before inspection start is expected.
	 * @param	p_rProductInstData		Aggregates product and seam data for a product instance.
	 * @return	void
	 */
	void onAutomaticStart(const interface::ProductInstData& p_rProductInstData);

	/**
	 * @brief 			Hanlde automatic cycle end.
	 * @return	void
	 */
	void onAutomaticEnd();

	/**
	 * @brief 			Hanlde live mode cycle end.
	 * @return	void
	 */
	void onLivemodeEnd();

	/**
	 * @brief Handle seam start signal.
	 * @param	p_rSeamData			Seam series and seam number
	 * @return	void
	 */
	void onSeamStart(interface::SeamData p_oSeamData);

	/**
	 * @brief Handle seam end signal.
	 * @return	void
	 */
	void onSeamEnd();

	/**
	 * @brief	set a value
	 * @param	p_oSmpKeyValue	key and value
	 * @return	KeyHandle		handle (token)
	 */
	interface::KeyHandle set(interface::SmpKeyValue p_oSmpKeyValue);

	/**
	 * @brief	get a value by key
	 * @param	p_oKey			key
	 * @return	SmpKeyValue		key and value
	 */
	interface::SmpKeyValue get(interface::Key p_oKey) const;

	/**
	 * @brief	get the configuration
	 * @return	Configuration	key value container
	 */
	interface::Configuration getConfig() const;

	/**
	 * @brief	Handle a single image and vdr information - is called by the videoRecorderServer.
	 * @param	p_oSensorId			Sensor ID of the sensor that sends the data.
	 * @param	p_rTriggerContext	Reference to triggerContext coming from the sensor.
	 * @param	p_rData				Reference to BImage object with the actual image.
	 * @param	p_oSeamData			Video recorder specific product information.
	 * @param	p_oNioReceived		If the image inspection has signaled a NIO.
	 * @return void
	 */
	void handleImage(int p_oSensorId, const interface::TriggerContext &p_rTriggerContext, const image::BImage &p_rData, interface::SeamData p_oSeamData, bool p_oNioReceived);

	/**
	 * @brief	Handle a sample and vdr information - is called by the videoRecorderServer.
	 * @param	p_oSensorId			Sensor ID of the sensor that sends the data.
	 * @param	imageNumber			The image number as set by the sensor.
	 * @param	p_rData				Reference to Sample object with the actual data array.
	 * @param	p_oSeamData			Video recorder specific product information.
	 * @param	p_oNioReceived		If the image inspection has signaled a NIO.
	 * @return void
	 */
	void handleSample(int p_oSensorId, int imageNumber, const image::Sample &p_rData, interface::SeamData p_oSeamData, bool p_oNioReceived);

	/**
	 * @brief	Joins running threads, writes configuration, cleanup.
	 * @return void
	 */
	void uninitialize();

	/**
	 * @brief	Get number of recorded raw data files available for that product instance.
	 * @param	p_rProductId		ID of a product.
	 * @param	p_rProductInstId	ID of a product instance.
	 * @return	Number of recorded raw data files available for that product instance.
	 */
	unsigned int getNbRawDataFilesAvailable(const Poco::UUID& p_rProductId, const Poco::UUID& p_rProductInstId);


	/**
	 * @brief	Get number of recorded raw data files available for that seam.
	 * @param	p_rProductId		ID of a product.
	 * @param	p_rProductInstId	ID of a product instance.
	 * @param	seamseries seamseries nr
	 * @param	seam seam nr
	 * @return	Number of recorded raw data files available for that seam.
	 */
	unsigned int getNbOfSamplesInSeam(const Poco::UUID& p_rProductId,
			const Poco::UUID& p_rProductInstId, uint32_t seamseries, uint32_t seam);


    void deleteAutomaticProductInstances(const std::vector<std::string>& paths);
    void deleteLiveModeProductInstances(const std::vector<std::string>& paths);


private:

	/**
	 * @brief 							Insert image in write queue. If the queue is full, a warning is emitted. Proxy for writer insertion.
	 * @param	p_rVdrImgage			Image and vdr context.
	 * @return	bool					If insertion succeeded.
	 */
	bool insertWriteQueue(const vdrImage_t &p_rVdrImgage);

	/**
	 * @brief 							Insert sample in write queue. If the queue is full, a warning is emitted. Proxy for writer insertion.
	 * @param	p_rVdrSample			Sample and vdr context.
	 * @return	bool					If insertion succeeded.
	 */
	bool insertWriteQueue(const vdrSample_t &p_rVdrSample);

	mutable Poco::FastMutex 			m_oWriterMutex;				///< Mutex to protect all data receive functions (event handlers) which call the writer.
	mutable Poco::FastMutex 			m_oReaderMutex;				///< Mutex to protect all data request functions (event handlers) which call the reader.

	grabberStatusProxy_t&				m_rGabberStatusProxy;  		///< send requests to grabber
	schedulerEventsProxy_t&				m_rSchedulerEventsProxy;  	///< send events to a scheduler
	Parameter							m_oParameter;				///< recording parameter
	uint32_t							m_oProductType;				///< product type number. Set in 'createDirectories(...)'.
	vdrImgage_queue_t 					m_oImageWriteCache;			///< Internal image write cache. Needed for nio seam writing.
	vdrSample_queue_t 					m_oSampleWriteCache;		///< Internal sample write cache. Needed for nio seam writing.
	Writer								m_oWriter;					///< Manages and executes image sequence writing.
	bool								m_oNioSeamOccured;			///< If during a seam one or more nios occured.
	bool								m_oIsRecordCycleActive;		///< if a livemode or an automatic cycle is active. If no cycle is active, no data is accepted.
	bool								m_oProductRecordSignaled;	///< if for current product recording was signaled to win
};

} // namespace vdr
} // namespace precitec

#endif /* VIDEORECORDER_H_ */
