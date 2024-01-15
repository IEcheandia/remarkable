/**
 * 	@file
 *  @copyright		Precitec Vision GmbH & Co. KG
 *  @author			Simon Hilsenbeck (HS)
 *  @date			2012
 *  @brief			VideoRecorder Interface
 */

#ifndef VIDEORECORDER_SERVER_H_
#define VIDEORECORDER_SERVER_H_


#include "event/videoRecorder.interface.h"
#include "videoRecorder.h"


namespace precitec
{
	using namespace  system;
	using namespace  message;

namespace interface
{

	template <>
	class TVideoRecorder<EventServer> : public TVideoRecorder<AbstractInterface>
	{
	public:
		TVideoRecorder(){}
		/**
		 * @brief 	The function is called from the automaticStart handler. All product information from task cache is transferred and
		 * 			avaialable for creating a folder tree that serves as recording repository. A small delay before inspection start is expected.
		 * @param	p_rProductInstData		Aggregates product and seam data for a product instance.
		 * @return	void
		 */
		/*virtual*/ void startAutomaticMode(const ProductInstData& p_rProductInstData){ std::cout << __FUNCTION__ << " called.\n"; }

		/**
		 * @brief End of automatic cycle.
		 * @return	void
		 */
		/*virtual*/ void stopAutomaticMode(){ std::cout << __FUNCTION__ << " called.\n"; }
		
		/**
		 * @brief Signal live mode start.
		 * @param	p_rProductInstData		Aggregates product and seam data for a product instance.
		 * @return	void
		 */		
		/*virtual*/ void startLiveMode(const ProductInstData& p_rProductInstData) { std::cout << __FUNCTION__ << " called.\n"; }

		/**
		 * @brief Signal live mode end.
		 * @return	void
		 */
		/*virtual*/ void stopLiveMode() { std::cout << __FUNCTION__ << " called.\n"; }

		/**
		 * @brief The function is called if the server receives a single image from a sensor. The function calls the handleImage function in the VideoRecorder manager.
		 * @param	p_oSensorId		The ID of the sensor that produced the image.
		 * @param	p_rTriggerContext	TriggerContext that contains the image number.
		 * @param	p_rImage			The actual image.
		 * @param	p_oSeamData			Video recorder specific product information.
		 * @param	p_oNioReceived		If the image inspection has signaled a NIO.
		 * @return	void
		 */
		/*virtual*/ void data(int p_oSensorId, const TriggerContext &p_rTriggerContext, const image::BImage &p_rImage, SeamData p_oSeamData, bool p_oNioReceived) { std::cout << __FUNCTION__ << " called.\n"; }

		/**
		 * @brief The function is called if the server receives a sample from a sensor. The function calls the handleSample function in the VideoRecorder server.
		 * @param	p_oSensorId			The ID of the sensor that produced the sample.
		 * @param	p_rTriggerContext	TriggerContext that contains the trigger number.
		 * @param	p_rSample			The actual sample.
		 * @param	p_oSeamData			Video recorder specific product information.
		 * @param	p_oNioReceived		If the sample inspection has signaled a NIO.
		 * @return	void
		 */
		/*virtual*/ void data(int p_oSensorId, const TriggerContext &p_rTriggerContext, const image::Sample &p_rSample, SeamData p_oSeamData, bool p_oNioReceived) { std::cout << __FUNCTION__ << " called.\n"; }

		/**
		 * @brief Signal seam start.
		 * @param	p_oSeamData			Seamseries and seam number.
		 * @return	void
		 */
		/*virtual*/ void seamStart(SeamData p_oSeamData){ std::cout << __FUNCTION__ << " called.\n"; }

		/**
		 * @brief Signal seam end.
		 * @return	void
		 */
		/*virtual*/ void seamEnd(){ std::cout << __FUNCTION__ << " called.\n"; }
	};


} // interface
} // precitec



#endif /*VIDEORECORDER_SERVER_H_*/
