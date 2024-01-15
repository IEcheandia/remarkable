#ifndef INSPECTION_SERVER_H_
#define INSPECTION_SERVER_H_

#include "event/inspection.interface.h"

namespace precitec
{
	using namespace  system;
	using namespace  message;

namespace interface
{

	template <>
	class TInspection<EventServer> : public TInspection<AbstractInterface>
	{
	public:
		TInspection(){}
		virtual ~TInspection() {}
	public:
		//Automatikbetrieb Start - Inspektion Bauteil aktiv, Grafen werden Produktspezifisch aufgebaut
		virtual void startAutomaticmode(uint32_t producttype, uint32_t productnumber, const std::string& p_rExtendedProductInfo) {}
		//Automatikbetrieb Stop
		virtual void stopAutomaticmode() {}
		// Inspection Start -> Naht aktiv
		virtual void start (int seamnumber ) {}
		// Inspection Ende -> Naht ! aktiv
		virtual void end ( int seamnumber ) {}
		// Nahtfolge uebernehmen
		virtual void info( int seamsequence ) {}
		// linelaser ein/aus
		virtual void linelaser (bool onoff) {}
		// Kalibration Start
		virtual void startCalibration() {}
		// Kalibration Ende
		virtual void stopCalibration() {}
		// Naht Vor-Start
		virtual void seamPreStart (int seamnumber ) {}
	};


} // interface
} // precitec



#endif /*INSPECTION_SERVER_H_*/
