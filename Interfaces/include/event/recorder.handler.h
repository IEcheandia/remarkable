#ifndef RECORDER_HANDLER_H_
#define RECORDER_HANDLER_H_

#include "event/recorder.interface.h"
#include "server/eventHandler.h"


namespace precitec
{
	using namespace image;

namespace interface
{
	using image::Sample;

	template <>
	class TRecorder<EventHandler> : public Server<EventHandler>, public TRecorderMessageDefinition
	{
	public:
		EVENT_HANDLER( TRecorder );
	public:
		void registerCallbacks()
		{
			// die Message-Callbacks eintragen, kein Returntyp in Macros!!!
            REGISTER_EVENT(Imagedata, data<image::BImage>);
            REGISTER_EVENT(Sampledata, data<image::Sample>);
            REGISTER_EVENT(MultiSampledata, multiSampleData);
            REGISTER_EVENT(SimulationDataMissing, simulationDataMissing);
		}

		template <typename T>
		void data(Receiver &receiver)
		{
			int sensorId; receiver.deMarshal(sensorId);
			ImageContext context; receiver.deMarshal(context);
			T data; receiver.deMarshal(data);
			OverlayCanvas canvas; receiver.deMarshal(canvas);
			getServer()->data(sensorId, context, data, canvas);
		}

        void multiSampleData(Receiver &receiver)
        {
            OverlayCanvas canvas; receiver.deMarshal(canvas);
            std::size_t size; receiver.deMarshal(size);
            std::vector<SampleFrame> samples;
            samples.reserve(size);
            for (std::size_t i = 0; i < size; i++)
            {
                int sensorId; receiver.deMarshal(sensorId);
                ImageContext context; receiver.deMarshal(context);
                image::Sample sample; receiver.deMarshal(sample);
                samples.emplace_back(sensorId, context, sample);
            }
            getServer()->multiSampleData(samples, canvas);
        }

        void simulationDataMissing(Receiver &receiver)
        {
            ImageContext context;
            receiver.deMarshal(context);
            getServer()->simulationDataMissing(context);
        }

	private:
		TRecorder<AbstractInterface> * getServer()
		{
			return server_;
		}

	};

} // namespace interface
} // namespace precitec

#endif /*RECORDER_HANDLER_H_*/
