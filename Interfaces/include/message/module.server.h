#ifndef MODULE_SERVER_H_
#define MODULE_SERVER_H_

#include  "Poco/ScopedLock.h"
#include  "Poco/Process.h"
#include  "Poco/Path.h"
#include  "Poco/Mutex.h"

#include  "module/interfaces.h"
#include  "message/module.h"
#include  "message/module.interface.h"
//#include  "message/module.handler.h"
#include  "message/registrar.proxy.h"
#include  "server/handler.h"
#include  "server/eventHandler.h"
#include  "protocol/protocol.info.h"
#include  "system/sharedMem.h"
#include  "system/types.h"


namespace precitec
{
	using system::module::isMessageInterface;
	using system::module::Interfaces;
	using system::message::ProtocolInfo;
	using system::message::SmpProtocolInfo;
namespace interface
{

	template <>
	class TModule<MsgServer> : public TModule<AbstractInterface> 	{

	public:
		/// Dfault CTor
		TModule(std::vector<interface::AnalyzerEntry> &mh, EventHandlerList &eh, std::vector<interface::ProxyEntry> &p);
		virtual ~TModule() {}
	public:
		// SubscriberInterface
		/// die Std-Initialisierung
		//virtual void startModule(SmpProtocolInfo moduleServerProtocolInfo);
		/// alle Server Runterfahren; Modul beenden
		virtual void kill();
		/// spezifischen Server starten
		SmpProtocolInfo startServer(int interfaceId, SmpProtocolInfo & protocolInfo, const std::string &path) override;
		/// spezifischen Server neu starten
		virtual void reStartServer(int interfaceId, SmpProtocolInfo & protocolInfo);
		/// spezifischen Server stoppen
		virtual void stopServer(int interfaceId, SmpProtocolInfo & protocolInfo);
		/// spezifischen Server starten
		void startProxy(int interfaceId, SmpProtocolInfo & protocolInfo, int subAppId, const std::string &path) override;
		/// spezifischen Server starten
		virtual void stopProxy(int interfaceId, SmpProtocolInfo & protocolInfo);
		void waitForAllServers();

	private:
		//die eigentlichen Lsiten sind in den abgeleiteten Klassen
		/// Liste aller exportierten Inerfaces
		std::vector<interface::AnalyzerEntry>	&mHandlerList_;
		EventHandlerList		&eHandlerList_;
		/// die Liste importierten Interfaces
		std::vector<interface::ProxyEntry>						&proxyList_;
		/// zaehlt die Zahl der laufenden Server
		//Poco::FastMutex			 shutdownSync_;
		Poco::FastMutex			serversReadyMutex_;
		//Poco::Condition			serversReady_;
		int									numRunningServers_;
		/// shMem fuer QnxEvents (MessageBuffer fuer Qnx-Pulse)
		SharedMem						bufferMemory_;
		/// name desselben
		PvString						shMemName_;

	}; // Module



} // namespace interface
} // namespace precitec

#endif /*MODULE_SERVER_H_*/
