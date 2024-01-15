#ifndef SENSOR_PROXY_H_
#define SENSOR_PROXY_H_

#include "server/eventProxy.h"
#include "event/sensor.interface.h"

namespace precitec
{
namespace interface
{

	template <>
	class TSensor<EventProxy> : public Server<EventProxy>, public TSensor<AbstractInterface>, public TSensorMessageDefinition
	{
	public:
		/// Der CTor braucht die Klasse, die die Arbeit erledigt, also den eigentlichen Server
		TSensor() : EVENT_PROXY_CTOR(TSensor), TSensor<AbstractInterface>()
		{
			//std::cout << "remote CTor::TRegistrar<Proxy> ohne Protokoll" << std::endl;
		}

		virtual ~TSensor() {}

	public:
		virtual void data(int sensorId, TriggerContext const& context, image::BImage const& data)
		{
			INIT_EVENT(Imagedata);
			marshal(sensorId);
			marshal(context);
			marshal(data);
			send();
		}

		virtual void data(int sensorId, TriggerContext const& context, image::Sample const& data)
		{
			INIT_EVENT(Sampledata);
			marshal(sensorId);
			marshal(context);
			marshal(data);
			send();
		}

        void simulationDataMissing(TriggerContext const& context) override
        {
            INIT_EVENT(SimulationDataMissing);
            marshal(context);
            send();
        }
	};

} // interface
} // precitec

#endif /*SENSOR_PROXY_H_*/
