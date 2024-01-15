#ifndef MODULE_INTERFACE_H_
#define MODULE_INTERFACE_H_

#include  "server/interface.h"
#include  "module/interfaces.h" // wg appId
#include  "protocol/protocol.info.h"


namespace precitec
{
	using namespace  system;
	using namespace  message;

namespace interface
{

	template <int CallType>
	class TModule;

	/**
	 *	AbstrakteBasisklasse des Module Servers
	 * 	Der Modul-Server kommuniziert mit dem Module-Manager,
	 * 		d.h. er wartet auf Befehler dessselben
	 */
	template <>
	class TModule<AbstractInterface> {
	public:
		TModule() {}
		virtual ~TModule() {}
	public:
		// SubscriberInterface

		// die member muessen alle entweder genullt werden oder eine {}-Implementierung bekommen
		// sonst mostert der Linker
		/// startet den Module-Server und dann die Std-Initialisierung
		//virtual void startModule(SmpProtocolInfo moduleServerProtocolInfo) = 0;
		/// alle Server Runterfahren; Modul beenden
		virtual void kill() = 0;
		/// spezifischen Server starten, wg QNX-Protokol Rueckgabe des erzeugten ProtokolInfos
		virtual SmpProtocolInfo startServer(int interfaceId, SmpProtocolInfo & protocolInfo, const std::string &path) =0;
		/// spezifischen Server (ggf. fuer neuen Cleint) neu starten
		virtual void reStartServer(int interfaceId, SmpProtocolInfo & protocolInfo) =0;
		/// spezifischen Server starten/stoppen
		virtual void stopServer(int interfaceId, SmpProtocolInfo & protocolInfo)=0;
		/// spezifischen Proxy starten/stoppen
		virtual void startProxy(int interfaceId, SmpProtocolInfo & protocolInfo, int subAppId, const std::string &path)=0;
		virtual void stopProxy(int interfaceId, SmpProtocolInfo & protocolInfo)=0;
	};

    struct TModuleMessageDefinition
    {
		MESSAGE(SmpProtocolInfo, StartServer, int, SmpProtocolInfo, std::string);
		MESSAGE(void, ReStartServer, int, SmpProtocolInfo);
		MESSAGE(void, StopServer, int, SmpProtocolInfo);
		MESSAGE(void, StartProxy, int, SmpProtocolInfo, int, std::string);
		MESSAGE(void, StopProxy, int, SmpProtocolInfo);
		MESSAGE(void, Kill, void);

		MESSAGE_LIST(
			Kill,
			StartServer,
			ReStartServer,
			StopServer,
			StartProxy,
			StopProxy
		);
    };

	template <>
	class TModule<Messages> : public Server<Messages>, public TModuleMessageDefinition {
	public:
		TModule() : info(system::module::Module, sendBufLen, replyBufLen, MessageList::NumMessages) {} //, NumHandlers) {}
		MessageInfo info;
	private:
		/// Konstanten wg Lesbarkeit, diese koennten auch in der Basisklasse stehen, wuerden dann aber wohl kaum verwendet
		enum { Bytes=1, KBytes=1024, MBytes=1024*1024};
		enum { sendBufLen  = 1*KBytes, replyBufLen = 100*Bytes, NumHandlers=2 };
	};

} // namespace interface
} // namespace precitec


#endif /*MODULE_INTERFACE_H_*/
