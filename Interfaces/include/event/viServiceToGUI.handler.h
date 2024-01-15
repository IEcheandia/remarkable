#ifndef VISERVICETOGUI_HANDLER_H_
#define VISERVICETOGUI_HANDLER_H_


#include "event/viService.h"
#include "event/viServiceToGUI.interface.h"
#include "server/eventHandler.h"


namespace precitec
{
namespace interface
{
	using namespace  message;

	template <>
	class TviServiceToGUI<EventHandler> : public Server<EventHandler>, public TviServiceToGUIMessageDefinition
	{
	public:
		EVENT_HANDLER( TviServiceToGUI );
	public:
		void registerCallbacks()
		{
			// die Message-Callbacks eintragen, kein Returntyp in Macros!!!
			REGISTER_EVENT(ProcessImageMsg, ProcessImage);
			REGISTER_EVENT(SlaveInfoECATMsg, SlaveInfoECAT);
			REGISTER_EVENT(ConfigInfoMsg, ConfigInfo);
		}

		void ProcessImage(Receiver &receiver)
		{
			ProcessDataVector input;
			receiver.deMarshal(input);

			ProcessDataVector output;
			receiver.deMarshal(output);

			getServer()->ProcessImage(input, output);
		}


		void SlaveInfoECAT(Receiver &receiver)
		{
			short count;
			receiver.deMarshal(count);
			SlaveInfo info(count);
			receiver.deMarshal(info);
			//std::cout << "TviService<EventHandler>::GetProcessImage("<<size<<")" << std::endl;
			getServer()->SlaveInfoECAT(count,info);
		}

		void ConfigInfo(Receiver &receiver)
		{
			std::string config;
			receiver.deMarshal(config);

			getServer()->ConfigInfo(config);
		}

		private:
				TviServiceToGUI<AbstractInterface> * getServer()
				{
					return server_;
				}


	};

} // namespace interface
} // namespace precitec



#endif /*VISERVICETOGUI_HANDLER_H_*/
