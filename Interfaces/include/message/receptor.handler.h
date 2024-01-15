#ifndef RECEPTOR_HANDLER_H_
#define RECEPTOR_HANDLER_H_

#include  "message/receptor.interface.h"
#include  "server/handler.h"
#include  "module/moduleLogger.h"

namespace precitec
{
namespace interface
{


	template <>
	class TReceptor<MsgHandler> : public Server<MsgHandler>, public TReceptorMessageDefinition {
	public:
		MSG_HANDLER(TReceptor);

		void registerCallbacks() { 
			// die Message-Callbacks eintragen, kein Returntyp in Macros!!
			REGISTER_MESSAGE(RegisterModuleManager, registerModuleManager);
			REGISTER_MESSAGE(RegisterModule, registerModule);
			REGISTER_MESSAGE(UnregisterModule, unregisterModule);
		}
		
		void registerModuleManager(Receiver &receiver) {
			//TestServer_Messages::MESSAGE_NAME4(compare, int, int, float, float) msg(sendBuffer, replyBuffer);
			int 							interfaceId; 	receiver.deMarshal(interfaceId);
			SmpProtocolInfo  	protocol; 		receiver.deMarshal(protocol);
			server_->registerModuleManager(interfaceId, protocol);
			wmLog(eDebug, "registerModuleManager interfaceId %i\n", interfaceId);
			// der Message-Reply mit dem Returnwert wird abgesetzt
			receiver.reply();
		}
		
		void registerModule(Receiver &receiver) {
			int 		moduleHandle; 	receiver.deMarshal(moduleHandle);
			ModuleSpec 	spec; 	receiver.deMarshal(spec);
			int			type; 	receiver.deMarshal(type);
			SmpProtocolInfo si = server_->registerModule(moduleHandle, spec, ModuleType(type));
			receiver.marshal(si);
			//receiver.marshal(server_->registerModule(spec, moduleHandle, ModuleType(type), clear);
			//receiver.dumpReplyBuffer();
			receiver.reply();
		}


		void unregisterModule(Receiver &receiver) {
			ModuleSpec 	spec; 	receiver.deMarshal(spec);
			int 		moduleHandle; 	receiver.deMarshal(moduleHandle);
			server_->unregisterModule(moduleHandle);
			receiver.reply();
		}
	};

} // namespace interface
} // namespace precitec
	
#endif /*RECEPTOR_LOCAL_H_*/
