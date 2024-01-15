/**
 * 	@file
 * 	@copyright	Precitec Vision GmbH & Co. KG
 * 	@author		Simon Hilsenbeck (HS)
 * 	@date		2011
 * 	@brief		Receives the image and product data.
 */

#ifndef VIDEORECORDERSERVER_H_
#define VIDEORECORDERSERVER_H_

// project includes
#include "event/videoRecorder.interface.h"
#include "event/imageShMem.h"
#include "videoRecorder/videoRecorder.h"
#include "common/seamData.h"
// stl includes
#include <iostream>

namespace precitec {
namespace vdr {

/**
 * @ingroup VideoRecorder
 * @brief Server class, receiving the videoRecorder data packages (images).
 * @details This is the server of the videoRecorder interface in the VideoRecorder module. It receives the data from the videoRecorders, e.g. images, analog values, etc.
 */
class VideoRecorderServer : public interface::TVideoRecorder<AbstractInterface>
{
public:

	/**
	 * @brief CTor.
	 * @param p_rVideoRecorder Reference to VideoRecorder that is called whenever an image is received.
	 */
	VideoRecorderServer(VideoRecorder &p_rVideoRecorder) :
		m_rVideoRecorder( p_rVideoRecorder ),
		imageShMem_(precitec::greyImage::sharedMemoryHandle(), precitec::greyImage::sharedMemoryName(), precitec::system::SharedMem::StdServer, precitec::greyImage::sharedMemorySize())
	{} // VideoRecorderServer

	/**
	 * @brief 	The function is called from the automaticStart handler. All product information from task cache is transferred and
	 * 			avaialable for creating a folder tree that serves as recording repository. A small delay before inspection start is expected.
	 * @param	p_rProductInstData		Aggregates product and seam data for a product instance.
	 * @return	void
	 */
	/*virtual*/ void startAutomaticMode(const ProductInstData& p_rProductInstData) {
		//const system::ScopedTimer	oTimer(__FUNCTION__); wmLog(eDebug, " %s()\n", __FUNCTION__);
		m_rVideoRecorder.onAutomaticStart(p_rProductInstData);
	}

	/**
	 * @brief Erase empty folders. Called from automaticStop after an automatic cycle.
	 * @return	void
	 */
	/*virtual*/ void stopAutomaticMode() {
		//const system::ScopedTimer	oTimer(__FUNCTION__); wmLog(eDebug, " %s()\n", __FUNCTION__);
		m_rVideoRecorder.onAutomaticEnd();
	}

	/**
	 * @brief Start of lice mode cycle.
	 * @param	p_rProductInstId	Product instance id.
	 * @return	void
	 */
	/*virtual*/ void startLiveMode(const ProductInstData& p_rProductInstData) {
		//const system::ScopedTimer	oTimer(__FUNCTION__); wmLog(eDebug, " %s()\n", __FUNCTION__);
		m_rVideoRecorder.onLivemodeStart(p_rProductInstData);
	}

	/**
	 * @brief End of lice mode cycle.
	 * @return	void
	 */
	/*virtual*/ void stopLiveMode() {
		//const system::ScopedTimer	oTimer(__FUNCTION__); wmLog(eDebug, " %s()\n", __FUNCTION__);
		m_rVideoRecorder.onLivemodeEnd();
	}

	/**
	 * @brief The function is called if the server receives a single image from a sensor. The function calls the handleImage function in the VideoRecorder server.
	 * @param	p_oSensorId		The ID of the sensor that produced the image.
	 * @param	p_rTriggerContext	TriggerContext that contains the image number.
	 * @param	p_rImage			The actual image.
	 * @param	p_oSeamData			Video recorder specific product information.
	 * @param	p_oNioReceived		If the image inspection has signaled a NIO.
	 * @return	void
	 */
	/*virtual*/ void data(int p_oSensorId, const TriggerContext &p_rTriggerContext, const image::BImage &p_rImage, SeamData p_oSeamData, bool p_oNioReceived) {
		//const system::ScopedTimer	oTimer(__FUNCTION__); wmLog(eDebug, " %s()\n", __FUNCTION__);
		m_rVideoRecorder.handleImage( p_oSensorId, p_rTriggerContext, p_rImage, p_oSeamData, p_oNioReceived);
	}

    void multiSampleData(const std::vector<SampleFrame> &samples, const SeamData &p_oSeamData, bool p_oNioReceived) override
    {
        for (const auto &sample : samples)
        {
            m_rVideoRecorder.handleSample(sample.sensorId(), sample.context().imageNumber(), sample.data(), p_oSeamData, p_oNioReceived);
        }
    }

	/**
	 * @brief Signal seam start.
	 * @param	p_oSeamData			Seamseries and seam number.
	 * @return	void
	 */
	/*virtual*/ void seamStart(SeamData p_oSeamData){
		//const system::ScopedTimer	oTimer(__FUNCTION__); wmLog(eDebug, " %s()\n", __FUNCTION__);
		m_rVideoRecorder.onSeamStart(p_oSeamData);
	}

	/**
	 * @brief Signal seam end.
	 * @return	void
	 */
	/*virtual*/ void seamEnd() {
		//const system::ScopedTimer	oTimer(__FUNCTION__); wmLog(eDebug, " %s()\n", __FUNCTION__);
		m_rVideoRecorder.onSeamEnd();
	}

    void deleteAutomaticProductInstances(const std::vector<std::string>& paths) override
    {
        m_rVideoRecorder.deleteAutomaticProductInstances(paths);
    }

    void deleteLiveModeProductInstances(const std::vector<std::string>& paths) override
    {
        m_rVideoRecorder.deleteLiveModeProductInstances(paths);
    }

private:

	VideoRecorder&			m_rVideoRecorder; ///< Reference to videoRecorder manager that is called whenever a videoRecorder data element is received.
	SharedMem 				imageShMem_;

};

}	// vdr
}	// precitec

#endif /* VIDEORECORDERSENSORSERVER_H_ */

