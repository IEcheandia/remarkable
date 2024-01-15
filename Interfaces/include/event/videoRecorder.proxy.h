/**
 * 	@file
 *  @copyright		Precitec Vision GmbH & Co. KG
 *  @author			Simon Hilsenbeck (HS)
 *  @date			2012
 *  @brief			VideoRecorder Interface
 */

#ifndef VIDEORECORDER_PROXY_H_
#define VIDEORECORDER_PROXY_H_


#include "server/eventProxy.h"
#include "event/videoRecorder.interface.h"
#include "videoRecorder.h"


namespace precitec
{
namespace interface
{

	template <>
	class TVideoRecorder<EventProxy> : public Server<EventProxy>, public TVideoRecorder<AbstractInterface>, public TVideoRecorderMessageDefinition
	{
	public:
		/// Der CTor braucht die Klasse, die die Arbeit erledigt, also den eigentlichen Server
		TVideoRecorder() : EVENT_PROXY_CTOR(TVideoRecorder), TVideoRecorder<AbstractInterface>()
		{}

		/**
		 * @brief 	The function is called from the automaticStart handler. All product information from task cache is transferred and
		 * 			avaialable for creating a folder tree that serves as recording repository. A small delay before inspection start is expected.
		 * @param	p_rProductInstData		Aggregates product and seam data for a product instance.
		 * @return	void
		 */
		/*virtual*/ void startAutomaticMode(const ProductInstData &p_rProductInstData){
			INIT_EVENT(StartAutomaticMode);

			signaler().marshal(p_rProductInstData);
			signaler().send();
		}

		/**
		 * @brief The function is called if the server receives a single image from a sensor. The function calls the handleImage function in the VideoRecorder manager.
		 * @param	p_oSensorId			The ID of the sensor that produced the image.
		 * @param	p_rTriggerContext	TriggerContext that contains the image number.
		 * @param	p_rImage			The actual image.
		 * @param	p_oSeamData			Video recorder specific product information.
		 * @param	p_oNioReceived		If the image inspection has signaled a NIO.
		 * @return	void
		 */
		/*virtual*/ void data(int p_oSensorId, const TriggerContext &p_rTriggerContext, const image::BImage &p_rImage, SeamData p_oSeamData, bool p_oNioReceived){
			INIT_EVENT(Imagedata);

			signaler().marshal(p_oSensorId);
			signaler().marshal(p_rTriggerContext);
			signaler().marshal(p_rImage);
            signaler().marshal(p_oSeamData.m_oSeamSeries);
            signaler().marshal(p_oSeamData.m_oSeam);
            signaler().marshal(p_oSeamData.m_oTriggerDelta);
			signaler().marshal(p_oNioReceived);
			signaler().send();
		}

        void multiSampleData(const std::vector<SampleFrame> &samples, const SeamData &p_oSeamData, bool p_oNioReceived) override
        {
            INIT_EVENT(MultiSampleData);
            signaler().marshal(p_oSeamData.m_oSeamSeries);
            signaler().marshal(p_oSeamData.m_oSeam);
            signaler().marshal(p_oSeamData.m_oTriggerDelta);
            signaler().marshal(p_oNioReceived);
            signaler().marshal(samples.size());
            for (const auto &sample : samples)
            {
                signaler().marshal(sample.sensorId());
                signaler().marshal(sample.context());
                signaler().marshal(sample.data());
            }
            signaler().send();
        }

		/**
		 * @brief End of automatic cycle.
		 * @return	void
		 */
		/*virtual*/ void stopAutomaticMode() {
			INIT_EVENT(StopAutomaticMode);
			signaler().send();
		}

		/**
		 * @brief Signal seam start.
		 * @param	p_oSeamData			Seamseries and seam number.
		 * @return	void
		 */
		/*virtual*/ void seamStart(SeamData p_oSeamData) {
			INIT_EVENT(SeamStart);
            signaler().marshal(p_oSeamData.m_oSeamSeries);
            signaler().marshal(p_oSeamData.m_oSeam);
            signaler().marshal(p_oSeamData.m_oTriggerDelta);
            signaler().marshal(p_oSeamData.m_oSeamLabel);
			signaler().send();
		}

		/**
		 * @brief Signal seam end.
		 * @return	void
		 */
		/*virtual*/ void seamEnd() {
			INIT_EVENT(SeamEnd);
			signaler().send();
		}

		/**
		 * @brief Start of lice mode cycle.
		 * @param	p_rProductInstData		Aggregates product and seam data for a product instance.
		 * @return	void
		 */
		/*virtual*/ void startLiveMode(const ProductInstData& p_rProductInstData) {
			INIT_EVENT(StartLiveMode);
			signaler().marshal(p_rProductInstData);
			signaler().send();
		}	

		/**
		 * @brief End of lice mode cycle.
		 * @return	void
		 */
		/*virtual*/ void stopLiveMode() {
			INIT_EVENT(StopLiveMode);
			signaler().send();
		}


void deleteAutomaticProductInstances(const std::vector<std::string>& paths) override
{
    INIT_EVENT(DeleteAutomaticProductInstances);
    signaler().marshal(paths.size());
    for (const auto& path : paths)
    {
        signaler().marshal(path);
    }
    signaler().send();
}

void deleteLiveModeProductInstances(const std::vector<std::string>& paths) override
{
    INIT_EVENT(DeleteLiveModeProductInstances);
    signaler().marshal(paths.size());
    for (const auto& path : paths)
    {
        signaler().marshal(path);
    }
    signaler().send();
}

	};

} // interface
} // precitec


#endif /*VIDEORECORDER_PROXY_H_*/
