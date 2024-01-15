/**
 * 	@file
 *  @copyright		Precitec Vision GmbH & Co. KG
 *  @author			Simon Hilsenbeck (HS)
 *  @date			2012
 *  @brief			VideoRecorder Interface
 */

#ifndef VIDEORECORDER_INTERFACE_H_
#define VIDEORECORDER_INTERFACE_H_

#include <vector>

#include "common/frame.h"
#include "server/interface.h"
#include "module/interfaces.h" // wg appId
#include "protocol/protocol.info.h"
#include "videoRecorder.h"

/*
 * Hier werden die abstrakten Basisklassen, sowie die
 * Messages definiert. Dieser Header wird sowohl fuer
 * den lokalen (Implementation, MsgHandler) sowie den
 * remote-Teil (Proxyer) benoetigt.
 */
namespace precitec
{
namespace interface
{
	using namespace  system;
	using namespace  message;

	//----------------------------------------------------------
	// Hier folgen die Template-Definitionen fuer die verschiedenen
	// Spezialisierungen  <Implementation> <MsgHandler> <Proxyer> <Messages>

	template <int mode>
	class TVideoRecorder;

	//----------------------------------------------------------
	// Abstrakte Basis Klassen  = Interface
	// Die <Implementation> und <Proxy> Spezialisierungen leiten von diesen
	// Basisklassen ab.

	/**
	 * Signaler zeigt auf primitive Weise den Systemzustand an.
	 * Der State-Enum bietet drei Zustaende an. Verschiedene
	 * Handler koennen diese Zustaende unterschiedlich darstellen.
	 */
	template<>
	class TVideoRecorder<AbstractInterface>
	{
	public:
		TVideoRecorder() {}
		virtual ~TVideoRecorder() {}
	public:
		/**
		 * @brief 	The function is called from the automaticStart handler. All product information from task cache is transferred and
		 * 			avaialable for creating a folder tree that serves as recording repository. A small delay before inspection start is expected.
		 * @param	p_rProductInstData		Aggregates product and seam data for a product instance.
		 * @return	void
		 */
		virtual void startAutomaticMode(const ProductInstData &p_rProductInstData) = 0;

		/**
		 * @brief End of automatic cycle.
		 * @return	void
		 */
		virtual void stopAutomaticMode() = 0;

		/**
		 * @brief Start of lice mode cycle.
		 * @param	p_rProductInstData		Aggregates product and seam data for a product instance.
		 * @return	void
		 */
		virtual void startLiveMode(const ProductInstData &p_rProductInstData) = 0;	

		/**
		 * @brief End of lice mode cycle.
		 * @return	void
		 */
		virtual void stopLiveMode() = 0;	

		/**
		 * @brief The function is called if the server receives a single image from a sensor. The function calls the handleImage function in the VideoRecorder server.
		 * @param	p_oSensorId			The ID of the sensor that produced the image.
		 * @param	p_rTriggerContext	TriggerContext that contains the image number.
		 * @param	p_rImage			The actual image.
		 * @param	p_oSeamData			Video recorder specific product information.
		 * @param	p_oNioReceived		If the image inspection has signaled a NIO.
		 * @return	void
		 */
		virtual void data(int p_oSensorId, const TriggerContext& p_rTriggerContext, const image::BImage& p_rImage, SeamData p_oSeamData, bool p_oNioReceived) = 0;

        /**
         * @brief The function is called if the server receives samples from sensors.  he function calls the handleSample function in the VideoRecorder server for each sample.
         * @param samples All the samples
         * @param p_oSeamData Video recorder specific product information.
         * @param p_oNioReceived If the sample inspection has signaled a NIO.
         **/
        virtual void multiSampleData(const std::vector<SampleFrame> &samples, const SeamData &p_oSeamData, bool p_oNioReceived) = 0;

		/**
		 * @brief Signal seam start.
		 * @param	p_oSeamData			Seamseries and seam number.
		 * @return	void
		 */
		virtual void seamStart(SeamData p_oSeamData) = 0;

		/**
		 * @brief Signal seam end.
		 * @return	void
		 */
		virtual void seamEnd() = 0;

        virtual void deleteAutomaticProductInstances(const std::vector<std::string>& paths) = 0;
        virtual void deleteLiveModeProductInstances(const std::vector<std::string>& paths) = 0;
	};

    struct TVideoRecorderMessageDefinition
    {
		EVENT_MESSAGE(StartAutomaticMode, ProductInstData);
		EVENT_MESSAGE(StopAutomaticMode, void);
		EVENT_MESSAGE(StartLiveMode, ProductInstData);
		EVENT_MESSAGE(StopLiveMode, void);
		EVENT_MESSAGE(Imagedata, int, TriggerContext, image::BImage, SeamData, bool);
		EVENT_MESSAGE(SeamStart, SeamData);
		EVENT_MESSAGE(SeamEnd, void);
		EVENT_MESSAGE(DeleteAutomaticProductInstances, std::vector<std::string>);
		EVENT_MESSAGE(DeleteLiveModeProductInstances, std::vector<std::string>);
        EVENT_MESSAGE(MultiSampleData, std::vector<SampleFrame>, SeamData, bool);

		MESSAGE_LIST(
			StartAutomaticMode,
			StopAutomaticMode,
			StartLiveMode,
			StopLiveMode,
			Imagedata,
			SeamStart,
			SeamEnd,
			DeleteAutomaticProductInstances,
			DeleteLiveModeProductInstances,
            MultiSampleData
		);
    };

	//----------------------------------------------------------
	template <>
	class TVideoRecorder<Messages> : public Server<Messages>, public TVideoRecorderMessageDefinition
	{
	public:
		TVideoRecorder() : info(system::module::VideoRecorder, sendBufLen, replyBufLen, MessageList::NumMessages, NumBuffers) {}
		MessageInfo info;
	private:
		/// Kontanten wg Lesbarkeit, diese koennten auch in der Basisklasse stehen, wuerden dann aber wohl kaum verwendet
		enum { Bytes=1, KBytes=1024, MBytes=1024*1024};
		enum { sendBufLen  = ((1*MBytes)+10*KBytes)/*TODO VM grabber*/, replyBufLen = 100*Bytes, NumBuffers=64 };
	};


} // namespace interface
} // namespace precitec


#endif /*VIDEORECORDER_INTERFACE_H_*/
