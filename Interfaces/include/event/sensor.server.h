#ifndef SENSOR_SERVER_H_
#define SENSOR_SERVER_H_


#include <unistd.h> // sleep
#include "event/sensor.interface.h"

namespace precitec
{
	using namespace  system;
	using namespace  message;

namespace interface
{

	template <>
	class TSensor<EventServer> : public TSensor<AbstractInterface> 
	{
	public:
		TSensor(){}
		virtual ~TSensor() {}
	public:
		// liefert ein Bild
		virtual void data(TriggerContext const& context, image::BImage const& frame) {}
		// liefert eine AnalogMessung
		virtual void data(TriggerContext const& context, image::Sample const& frame) {}
	private:
	};

	class SisoMe4 : public TSensor<AbstractInterface> 
	{
	public:
		SisoMe4(){}
		virtual ~SisoMe4() {}
	public:
		// liefert ein Bild
		virtual void data(TriggerContext const& context, image::BImage const& frame) {}
		// liefert eine AnalogMessung
		virtual void data(TriggerContext const& context, image::Sample const& frame) {}
	private:
	};



} // interface
} // precitec


#endif /*SENSOR_SERVER_H_*/
