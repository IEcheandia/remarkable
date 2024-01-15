#ifndef VISERVICEFROMGUI_PROXY_H_
#define VISERVICEFROMGUI_PROXY_H_


#include "event/viServiceFromGUI.interface.h"


namespace precitec
{
	using namespace  system;
	using namespace  message;

namespace interface
{

	template <>
	class TviServiceFromGUI<EventServer> : public TviServiceFromGUI<AbstractInterface>
	{
	public:
		TviServiceFromGUI(){}
		virtual ~TviServiceFromGUI() {}
	public:

		virtual void SetTransferMode(bool onOff){}
		virtual void OutputProcessData(short physAddr, ProcessData& data, ProcessData& mask, short type){};

	};


} // interface
} // precitec



#endif /*VISERVICEFROMGUI_PROXY_H_*/
