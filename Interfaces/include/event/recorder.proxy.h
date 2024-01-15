#ifndef RECORDER_PROXY_H_
#define RECORDER_PROXY_H_

#include "server/eventProxy.h"
#include "event/recorder.interface.h"


namespace precitec
{
	using namespace image;

namespace interface
{
	template <>
	class TRecorder<EventProxy> : public Server<EventProxy>, public TRecorder<AbstractInterface>, public TRecorderMessageDefinition
	{
	public:
		/// Der CTor braucht die Klasse, die die Arbeit erledigt, also den eigentlichen Server
		TRecorder() : EVENT_PROXY_CTOR(TRecorder), TRecorder<AbstractInterface>()
		{
			//std::cout << "remote CTor::TRegistrar<Proxy> ohne Protokoll" << std::endl;
		}

		virtual ~TRecorder() {}

	public:
		virtual void data(int sensorId, ImageContext const& context, image::BImage const& data, image::OverlayCanvas const& canvas) {
			INIT_EVENT(Imagedata);
			signaler().marshal(sensorId);
			signaler().marshal(context);
			signaler().marshal(data);
			signaler().marshal(canvas);
			signaler().send();
		}


		virtual void data(int sensorId, ImageContext const& context, image::Sample const& data, image::OverlayCanvas const& canvas) {
			INIT_EVENT(Sampledata);
			signaler().marshal(sensorId);
			signaler().marshal(context);
			signaler().marshal(data);
			signaler().marshal(canvas);
			signaler().send();
		}

        void multiSampleData(const std::vector<precitec::interface::SampleFrame> &samples, image::OverlayCanvas const& canvas) override
        {
            INIT_EVENT(MultiSampledata)
            signaler().marshal(canvas);
            signaler().marshal(samples.size());
            for (const auto &sample : samples)
            {
                signaler().marshal(sample.sensorId());
                signaler().marshal(sample.context());
                signaler().marshal(sample.data());
            }
            signaler().send();
        }

        void simulationDataMissing(ImageContext const& context) override
        {
            INIT_EVENT(SimulationDataMissing);
            signaler().marshal(context);
            signaler().send();
        }

	};

} // interface
} // precitec

#endif /*RECORDER_PROXY_H_*/
