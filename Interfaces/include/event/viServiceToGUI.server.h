#ifndef VISERVICETOGUI_PROXY_H_
#define VISERVICETOGUI_PROXY_H_


#include "event/viServiceToGUI.interface.h"


namespace precitec
{
	using namespace  system;
	using namespace  message;

namespace interface
{

	template <>
	class TviServiceToGUI<EventServer> : public TviServiceToGUI<AbstractInterface>
	{
	public:
		TviServiceToGUI(){}
		virtual ~TviServiceToGUI() {}
	public:

		virtual void ProcessImage(ProcessDataVector& input, ProcessDataVector& output){}
		virtual void SlaveInfoECAT(short count, SlaveInfo info){};
		virtual void ConfigInfo(std::string config){};

	};


} // interface
} // precitec



#endif /*VISERVICETOGUI_PROXY_H_*/
