#ifndef RECORDER_SERVER_H_
#define RECORDER_SERVER_H_

#include "event/recorder.interface.h"

/*
 * Hier werden die abstrakten Basisklassen, sowie die
 * Messages definiert. Dieser Header wird sowohl fuer
 * den lokalen (Implementation, MsgHandler) sowie den
 * remote-Teil (Proxyer) benoetigt.
 */
namespace precitec
{
	using namespace image;
	
namespace interface
{
	
	template <>
	class TRecorder<EventServer> : public TRecorder<AbstractInterface> 
	{
	public:
		TRecorder(){}
		virtual ~TRecorder() {}
	public:
		// liefert ein Bild inkl. Context und Canvasdaten
		void data(int sensorId, ImageContext const &context, BImage const& data, OverlayCanvas const& canvas) {}
		// liefert eine AnalogMessung/...
		void data(int sensorId, ImageContext const &context, Sample const& data, OverlayCanvas const& canvas) {}
		
	private:
	};


} // namespace interface
} // namespace precitec

#endif /*RECORDER_SERVER_H_*/
