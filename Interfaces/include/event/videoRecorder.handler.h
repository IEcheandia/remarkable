/**
 * 	@file
 *  @copyright		Precitec Vision GmbH & Co. KG
 *  @author			Simon Hilsenbeck (HS)
 *  @date			2012
 *  @brief			VideoRecorder Interface
 */

#ifndef VIDEORECORDER_HANDLER_H_
#define VIDEORECORDER_HANDLER_H_


#include "event/videoRecorder.interface.h"
#include "server/eventHandler.h"
#include "videoRecorder.h"


namespace precitec
{
namespace interface
{
	using namespace  message;

	template <>
	class TVideoRecorder<EventHandler> : public Server<EventHandler>, public TVideoRecorderMessageDefinition
	{
	public:
		EVENT_HANDLER( TVideoRecorder );
	public:
		void registerCallbacks()
		{
			// die Message-Callbacks eintragen, kein Returntyp in Macros!!!
			REGISTER_EVENT(StartAutomaticMode, startAutomaticMode);
            REGISTER_EVENT(Imagedata, data<image::BImage>);
			REGISTER_EVENT(StopAutomaticMode, stopAutomaticMode);
			REGISTER_EVENT(SeamStart, seamStart);
			REGISTER_EVENT(SeamEnd, seamEnd);
			REGISTER_EVENT(StartLiveMode, startLiveMode);
			REGISTER_EVENT(StopLiveMode, stopLiveMode);
			REGISTER_EVENT(DeleteAutomaticProductInstances, deleteAutomaticProductInstances);
			REGISTER_EVENT(DeleteLiveModeProductInstances, deleteLiveModeProductInstances);
            REGISTER_EVENT(MultiSampleData, multiSampleData);
		}

		void startAutomaticMode(Receiver& p_rReceiver)
		{
			ProductInstData 			oProductInstData;		p_rReceiver.deMarshal(oProductInstData);

			server_->startAutomaticMode(oProductInstData);
		}

		void stopAutomaticMode(Receiver& p_rReceiver)
		{
			server_->stopAutomaticMode();
		}

		void startLiveMode(Receiver& p_rReceiver)
		{
			ProductInstData 			oProductInstData;		p_rReceiver.deMarshal(oProductInstData);
			server_->startLiveMode(oProductInstData);
		}

		void stopLiveMode(Receiver& p_rReceiver)
		{
			server_->stopLiveMode();
		}

        template <typename T>
		void data(Receiver& p_rReceiver)
		{
			int 			oSensorId = 0;		p_rReceiver.deMarshal(oSensorId);
			TriggerContext	oTriggerContext;	p_rReceiver.deMarshal(oTriggerContext);
			T	oImage;				p_rReceiver.deMarshal(oImage);
			SeamData 		oSeamData;
            p_rReceiver.deMarshal(oSeamData.m_oSeamSeries);
            p_rReceiver.deMarshal(oSeamData.m_oSeam);
            p_rReceiver.deMarshal(oSeamData.m_oTriggerDelta);
			bool 			oNioReceived;		p_rReceiver.deMarshal(oNioReceived);

			server_->data(oSensorId, oTriggerContext, oImage, oSeamData, oNioReceived);
		}

        void multiSampleData(Receiver &p_rReceiver)
        {
            SeamData oSeamData;
            p_rReceiver.deMarshal(oSeamData.m_oSeamSeries);
            p_rReceiver.deMarshal(oSeamData.m_oSeam);
            p_rReceiver.deMarshal(oSeamData.m_oTriggerDelta);
            bool oNioReceived; p_rReceiver.deMarshal(oNioReceived);
            std::size_t size; p_rReceiver.deMarshal(size);
            std::vector<SampleFrame> samples;
            samples.reserve(size);
            for (std::size_t i = 0; i < size; i++)
            {
                int sensorId; p_rReceiver.deMarshal(sensorId);
                ImageContext context; p_rReceiver.deMarshal(context);
                image::Sample sample; p_rReceiver.deMarshal(sample);
                samples.emplace_back(sensorId, context, sample);
            }

            server_->multiSampleData(samples, oSeamData, oNioReceived);
        }

		void seamStart(Receiver& p_rReceiver)
		{
			SeamData 		oSeamData;
            p_rReceiver.deMarshal(oSeamData.m_oSeamSeries);
            p_rReceiver.deMarshal(oSeamData.m_oSeam);
            p_rReceiver.deMarshal(oSeamData.m_oTriggerDelta);
            p_rReceiver.deMarshal(oSeamData.m_oSeamLabel);
			server_->seamStart(oSeamData);
		}

		void seamEnd(Receiver& p_rReceiver)
		{
			server_->seamEnd();
		}

void deleteAutomaticProductInstances(Receiver& p_rReceiver)
{
    std::size_t size{0u};
    std::vector<std::string> paths;
    p_rReceiver.deMarshal(size);
    paths.reserve(size);
    for (std::size_t i{0u}; i < size; i++)
    {
        std::string path;
        p_rReceiver.deMarshal(path);
        paths.emplace_back(std::move(path));
    }
    server_->deleteAutomaticProductInstances(paths);
}

void deleteLiveModeProductInstances(Receiver& p_rReceiver)
{
    std::size_t size{0u};
    std::vector<std::string> paths;
    p_rReceiver.deMarshal(size);
    paths.reserve(size);
    for (std::size_t i{0u}; i < size; i++)
    {
        std::string path;
        p_rReceiver.deMarshal(path);
        paths.emplace_back(std::move(path));
    }
    server_->deleteLiveModeProductInstances(paths);
}

	};

} // namespace interface
} // namespace precitec



#endif /*VIDEORECORDER_HANDLER_H_*/
