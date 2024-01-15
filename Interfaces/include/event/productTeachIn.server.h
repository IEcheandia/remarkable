#ifndef PRODUCTTEACHIN_SERVER_H_
#define PRODUCTTEACHIN_SERVER_H_

#include "event/productTeachIn.interface.h"

namespace precitec
{
	using namespace  system;
	using namespace  message;

namespace interface
{

	template <>
	class TProductTeachIn<EventServer> : public TProductTeachIn<AbstractInterface>
	{
	public:
		TProductTeachIn(){}
		virtual ~TProductTeachIn() {}
	public:
		// Inspection Start -> Naht aktiv
		virtual void start (int seamSeries, int seam ) {}
		// Inspection Ende -> Naht ! aktiv
		virtual void end ( system::Timer::Time duration) {}
		// startAuto
		virtual void startAutomatic(int code) {}
		// stopAuto
		virtual void stopAutomatic() {}
	};


} // interface
} // precitec



#endif /*PRODUCTTEACHIN_SERVER_H_*/
