#ifndef RECORDER_INTEFACE_H_
#define RECORDER_INTEFACE_H_

#include "server/interface.h"
#include "module/interfaces.h" // wg appId
#include "protocol/protocol.info.h"
#include "overlay/overlayCanvas.h"
#include "event/recorder.h"


namespace precitec
{
namespace interface
{
	template <int CallType>
	class TRecorder;
	

	template <>
	class TRecorder<AbstractInterface> 
	{
	public:
		TRecorder(){}
		virtual ~TRecorder() {}
	public:
		// liefert ein Bild inkl. Context und Canvasdaten
		virtual void data(int sensorId, ImageContext const& context, image::BImage const& data, image::OverlayCanvas const& canvas) = 0;
		// liefert eine AnalogMessung/...
		virtual void data(int sensorId, ImageContext const& context, image::Sample const& data, image::OverlayCanvas const& canvas) = 0;

        virtual void multiSampleData(const std::vector<precitec::interface::SampleFrame> &samples, image::OverlayCanvas const& canvas) = 0;

        virtual void simulationDataMissing(ImageContext const& context) = 0;
	private:
	};

    struct TRecorderMessageDefinition
    {
		EVENT_MESSAGE(Imagedata, int, ImageContext, image::BImage, image::OverlayCanvas);
		EVENT_MESSAGE(Sampledata, int, ImageContext, image::Sample, image::OverlayCanvas);
        EVENT_MESSAGE(MultiSampledata, std::vector<precitec::interface::SampleFrame>, image::OverlayCanvas);
        EVENT_MESSAGE(SimulationDataMissing, ImageContext);
		MESSAGE_LIST(
            Imagedata,
            Sampledata,
            MultiSampledata,
            SimulationDataMissing
		);
    };

			
	template <>
	class TRecorder<Messages> : public TRecorderMessageDefinition
	{
	public:
		TRecorder<Messages>() : info(system::module::Recorder, sendBufLen, replyBufLen, MessageList::NumMessages, NumBuffers) {}
		MessageInfo info;
	private:	
		// Werte werden so festgelegt, dass alle Parameter und Ergebnisse Platz finden
		enum { Bytes=1, KBytes=1024, MBytes=1024*1024};
		enum { sendBufLen  = 2*MBytes, replyBufLen = 1500*KBytes, NumBuffers=8 };
	};

			
} // interface
} // precitec

#endif /*RECORDER_INTEFACE_H_*/
