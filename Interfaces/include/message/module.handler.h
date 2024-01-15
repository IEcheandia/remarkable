#ifndef MODULE_HANDLER_H_
#define MODULE_HANDLER_H_

#include <iostream>
#include  "Poco/NamedMutex.h"
#include  "Poco/ScopedLock.h"
#include  "Poco/Process.h"

#include  "system/types.h"
#include  "server/handler.h"
#include  "message/module.interface.h"
#include  "module/moduleLogger.h"

namespace precitec
{
namespace interface
{


	template <>
	class TModule<MsgHandler> : public Server<MsgHandler>, public TModuleMessageDefinition	{
	public:
		MSG_HANDLER(TModule);
	public:

		void registerCallbacks() {
			// die Message-Callbacks eintragen, kein Returntyp in Macros!!!
			//REGISTER1(TModule, startModule, SmpProtocolInfo );
			REGISTER_MESSAGE(Kill, kill);
			REGISTER_MESSAGE(StartServer, startServer);
			REGISTER_MESSAGE(ReStartServer, reStartServer);
			REGISTER_MESSAGE(StopServer, stopServer);
			REGISTER_MESSAGE(StartProxy, startProxy);
			REGISTER_MESSAGE(StopProxy, stopProxy);
		}

		void kill(Receiver &receiver) {
			wmLog(eDebug, "RHandler::kill\n");
			// erst darf der Server aufraeumen
			server_->kill();
			// dann ist der Handler selbst dran
			stop(); // sollte redundant sein (vom Server bereits geschickt sein) ????
			// jestzt sollten alle Threads joinen und der Hanlder sich beenden
			// der Message-Reply mit dem Returnwert wird abgesetzt
			receiver.reply();
		}

		void startServer(Receiver &receiver) {
			int interfaceId; receiver.deMarshal(interfaceId);
			SmpProtocolInfo protocolInfo; receiver.deMarshal(protocolInfo);
			if (protocolInfo.isNull()) wmLog(eDebug, "TModule<MsgHandler>::startServer deMarshalled empty protocol\n");
            std::string path; receiver.deMarshal(path);
			receiver.marshal(server_->startServer(interfaceId, protocolInfo, path));
			// der Message-Reply mit dem Returnwert wird abgesetzt
			receiver.reply();
		}

		void reStartServer(Receiver &receiver) {
			int interfaceId; receiver.deMarshal(interfaceId);
			SmpProtocolInfo protocolInfo; receiver.deMarshal(protocolInfo);
			if (protocolInfo.isNull()) wmLog(eDebug, "TModule<MsgHandler>::startServer deMarshalled empty protocol\n");
			server_->reStartServer(interfaceId, protocolInfo);
			// der Message-Reply mit dem Returnwert wird abgesetzt
			receiver.reply();
		}


		void stopServer(Receiver &receiver) {
			int interfaceId; receiver.deMarshal(interfaceId);
			SmpProtocolInfo protocolInfo; receiver.deMarshal(protocolInfo);
			server_->stopServer(interfaceId, protocolInfo);
			// der Message-Reply mit dem Returnwert wird abgesetzt
			receiver.reply();
		}

		void startProxy(Receiver &receiver) {
			int interfaceId; receiver.deMarshal(interfaceId);
			std::cout <<  system::module::InterfaceName[interfaceId] << std::endl;
			SmpProtocolInfo protocolInfo; receiver.deMarshal(protocolInfo);
            int subAppId; receiver.deMarshal(subAppId);
            std::string path; receiver.deMarshal(path);
			server_->startProxy(interfaceId, protocolInfo, subAppId, path);
			// der Message-Reply mit dem Returnwert wird abgesetzt
			receiver.reply();
		}


		void stopProxy(Receiver &receiver) {
			int interfaceId; receiver.deMarshal(interfaceId);
			SmpProtocolInfo protocolInfo; receiver.deMarshal(protocolInfo);
			server_->stopProxy(interfaceId, protocolInfo);
			// der Message-Reply mit dem Returnwert wird abgesetzt
			receiver.reply();
		}

	};

} // namespace interface
} // namespace precitec

#endif /*MODULE_HANDLER_H_*/
