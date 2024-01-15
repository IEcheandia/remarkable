#ifndef SENSOR_INTERFACE_H_
#define SENSOR_INTERFACE_H_

//#include <unistd.h> // sleep


#include "server/interface.h"
#include "module/interfaces.h" // wg appId
#include "protocol/protocol.info.h"
#include "event/sensor.h"


namespace precitec
{
namespace interface
{
	template <int CallType>
	class TSensor;


	template <>
	class TSensor<AbstractInterface>
	{
	public:
		TSensor(){}
		virtual ~TSensor() {}
	public:
		// liefert ein Bild
		virtual void data(int sensorId, TriggerContext const& context, image::BImage const& data) = 0;
		// liefert eine AnalogMessung/...
		virtual void data(int sensorId, TriggerContext const& context, image::Sample const& data) = 0;

        virtual void simulationDataMissing(TriggerContext const& context) = 0;
	private:
	};

    struct TSensorMessageDefinition
    {
		EVENT_MESSAGE(Imagedata, int, TriggerContext, image::BImage);
		EVENT_MESSAGE(Sampledata, int, TriggerContext, image::Sample);
        EVENT_MESSAGE(SimulationDataMissing, TriggerContext);

		MESSAGE_LIST(
			Imagedata,
			Sampledata,
            SimulationDataMissing
		);
    };

	template <>
	class TSensor<Messages> : public TSensorMessageDefinition
	{
	public:
		TSensor<Messages>() : info(system::module::Sensor, sendBufLen, replyBufLen, MessageList::NumMessages, NumBuffers) {}
		MessageInfo info;
	private:
		// Werte werden so festgelegt, dass alle Parameter und Ergebnisse Platz finden
		enum { Bytes=1, KBytes=1024, MBytes=1024*1024};
		enum { sendBufLen  = ((1*MBytes)+10*KBytes), replyBufLen = 500*Bytes, NumBuffers=64 };
	};


} // interface
} // precitec

#endif /*SENSOR_INTERFACE_H_*/
