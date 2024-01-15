#ifndef VISERVICEFROMGUI_HANDLER_H_
#define VISERVICEFROMGUI_HANDLER_H_


#include "event/viService.h"
#include "event/viServiceFromGUI.interface.h"
#include "server/eventHandler.h"


namespace precitec
{
namespace interface
{
	using namespace  message;

	template <>
	class TviServiceFromGUI<EventHandler> : public Server<EventHandler>, public TviServiceFromGUIMessageDefinition
	{
	public:
		EVENT_HANDLER( TviServiceFromGUI );
	public:
		void registerCallbacks()
		{
			// die Message-Callbacks eintragen, kein Returntyp in Macros!!!
			REGISTER_EVENT(SetTransferModeMsg, SetTransferMode);
			REGISTER_EVENT(OutputProcessDataMsg, OutputProcessData);
			REGISTER_EVENT(RequestSlaveInfo, requestSlaveInfo);
		}

		void SetTransferMode(Receiver &receiver)
		{
			bool onOff;
			receiver.deMarshal(onOff);
			getServer()->SetTransferMode(onOff);
		}

		void OutputProcessData(Receiver &receiver)
		{
			short physAddr;
			receiver.deMarshal(physAddr);

			ProcessData data;
			receiver.deMarshal(data);

			ProcessData mask;
			receiver.deMarshal(mask);

			short type;
			receiver.deMarshal(type);

			getServer()->OutputProcessData(physAddr, data, mask, type);
		}

		void requestSlaveInfo(Receiver &receiver)
		{
			getServer()->requestSlaveInfo();
		}



		private:
				TviServiceFromGUI<AbstractInterface> * getServer()
				{
					return server_;
				}


	};

} // namespace interface
} // namespace precitec



#endif /*VISERVICEFROMGUI_HANDLER_H_*/
