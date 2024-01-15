#ifndef CONFIG_HANDLER_H_
#define CONFIG_HANDLER_H_

#include <iostream> 
#include <string> // std::string
#include <map>		// std::map

#include "server/handler.h"
#include  "message/messageReceiver.h"
#include "message/config.interface.h"

namespace precitec
{
namespace interface
{

	// Defaultserver hat keine Funktion
	template<>
	class TConfig<MsgServer> : public TConfig<AbstractInterface>
	{
	protected:
		void setParam(PvString key, DynamicAny value) {}
		void init(ParameterList const& params) {}
	};
	
	
	// Remotehandler empfangt die Message enpackt die Message und ruft den Implementation Server auf
	template <>
	class TConfig<MsgHandler> : public Server<MsgHandler>, public TConfig<AbstractInterface>
	{
	public:
		MSG_HANDLER( TConfig );
	public:
		void registerCallbacks() 
		{
			// die Message-Callbacks eintragen, kein Returntyp in Macros!!!
			REGISTER2(TConfig, setParam, PvString, DynamicAny);
			REGISTER1(TConfig, init, ParameterList);
		}

		void setParam(Receiver &receiver)
		{
			PvString  arg0; receiver.deMarshal(arg0);
			PvString 	arg1; receiver.deMarshal(arg1);
			server_->setParam(arg0, arg1);
		}
		
		void init(Receiver &receiver)
		{
			ParameterList params;
			int length; receiver.deMarshal(length);
			
			for(int i=0;i<length;i++)
			{
				PvString key; 	receiver.deMarshal(key);
				PvString value; 	receiver.deMarshal(value);
				params.insert(ParameterEntry(key, value));
			}
			server_->init(params);
		}
		

	};

} // namespace interface
} // namespace precitec


#endif /*CONFIG_HANDLER_H_*/
