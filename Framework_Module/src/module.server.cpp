#include  "system/types.h"
#include  "message/module.server.h"
#include "module/moduleLogger.h"

namespace precitec
{

	using namespace system::module;
	using namespace system::message;
namespace interface
{

	TModule<MsgServer>::TModule(std::vector<interface::AnalyzerEntry> &mh, EventHandlerList &eh, std::vector<interface::ProxyEntry> &p)
	: mHandlerList_(mh), eHandlerList_(eh), proxyList_(p),
		serversReadyMutex_(), // mutex locked
		//serversReady_(serversReadyMutex_), // condition signals stop of last server
		numRunningServers_(0)
	{
		serversReadyMutex_.lock();
	}

	// SubscriberInterface
	/// die Std-Initialisierung
	//void startModule(SmpProtocolInfo &moduleServerProtocolInfo);
	/// alle Server Runterfahren; Modul beenden
	void TModule<MsgServer>::kill() {

		for (auto i=mHandlerList_.begin(); i!=mHandlerList_.end(); ++i)
		{
			i->handler->stop(); // stoppt Server und joined den MsgLoop-Thread
			--numRunningServers_;
		}
		for (EventHandlerList::iterator i=eHandlerList_.begin(); i!=eHandlerList_.end(); ++i)
		{
			Server<EventHandler> &server = *i->second.handler;
			server.stop(); // stoppt Server fuer alle Protokolle, und joined den MsgLoop-Thread
			--numRunningServers_;
		}
		// ??????
		// if (!(numRunningServers_>0)) serversReady_.broadcast();
	}

	/// spezifischen Server starten
	SmpProtocolInfo TModule<MsgServer>::startServer(int interfaceId, SmpProtocolInfo &protocolInfo, const std::string &path) {
		/// jedes Modul muss alle seine Handler in diese Liste eintragen
		if (protocolInfo.isNull()) std::cout <<  "TModule<MsgServer>::startServer received empty protocol" << std::endl;
		if (isMessageInterface(Interfaces(interfaceId))) {
            for (auto it = mHandlerList_.begin(); it != mHandlerList_.end(); it++)
            {
                if (it->handler->info_.interfaceId != interfaceId || it->path != path)
                {
                    continue;
                }
                //std::cout <<  "TModule<MsgServer>::startServer activating message " << InterfaceName[interfaceId] << " with protocol " <<  *protocolInfo << std::endl;
                ++numRunningServers_;
                return it->handler->activate(protocolInfo);
            }
            return {};
		}	else {
			//std::cout <<  "TModule<MsgServer>::startServer activating Event " << InterfaceName[interfaceId] << " with protocol " <<  *protocolInfo << std::endl;
			Server<EventHandler> &server = *(eHandlerList_[interfaceId].handler);
			++numRunningServers_;
			return server.activate(protocolInfo);
		}
	}


	/**
	 * spezifischen Server 'neu' starten
	 * Es gibt zwei Faelle:
	 *  - ein EventServer kann deAktiviert werden
	 *  - Message und EventSevrer koennen mehrere Clients/Publisher bedienen
	 *    dabei verwenden sie das gleiche Protokoll und den gleichen realen Server
	 * In beiden Faellen existiert der Server ; er kann bereits laufen (beij MsgServer ist das immer
	 * 	  der Fall); falls nicht wird erwieder gestartet
	 * @param interfaceId die Schnittstelle (und damit der Server)
	 * @param uniqueId    hiermit wird das Protokoll identifiziert, da weder Protocol noch ProtcolInfo
	 *										sortierbar sind. \todo uniqueId in ProtocolInfo stecken ??? scheint getrennt einfacher
	 */

	void TModule<MsgServer>::reStartServer(int interfaceId, SmpProtocolInfo & protocolInfo) {
		/// jedes Modul muss alle seine Handler in diese Liste eintragen
		if (isMessageInterface(Interfaces(interfaceId))) {
            for (auto it = mHandlerList_.begin(); it != mHandlerList_.end(); it++)
            {
                if (it->handler->info_.interfaceId != interfaceId)
                {
                    continue;
                }
                std::stringstream oSt; oSt << "TModule<MsgServer>::startServer reActivating message " << InterfaceName[interfaceId] << " with protocol " <<  *protocolInfo << std::endl;
                wmLog( eDebug, oSt.str() );
                ++numRunningServers_;
                it->handler->start();
            }
		}	else {
			std::stringstream oSt; oSt <<  "TModule<MsgServer>::startServer reActivating Event " << InterfaceName[interfaceId] << " with protocol " <<  *protocolInfo << std::endl;
			wmLog( eDebug, oSt.str() );
			Server<EventHandler> &server = *(eHandlerList_[interfaceId].handler);
			++numRunningServers_;
			server.start(protocolInfo->type());
		}
	}

	/// spezifischen Server stoppen
	void TModule<MsgServer>::stopServer(int interfaceId, SmpProtocolInfo & protocolInfo) {
		/// jedes Modul muss alle seine Handler in diese Liste eintragen
		if (isMessageInterface(Interfaces(interfaceId))) {
            for (auto it = mHandlerList_.begin(); it != mHandlerList_.end(); it++)
            {
                if (it->handler->info_.interfaceId != interfaceId)
                {
                    continue;
                }
                it->handler->stop();
            }
		}	else {
			Server<EventHandler> &server = *(eHandlerList_[interfaceId].handler);
			server.stop(protocolInfo->type());
		}
	}

	/// spezifischen Server starten
	void TModule<MsgServer>::startProxy(int interfaceId, SmpProtocolInfo & protocolInfo, int subAppId, const std::string &path) {
		/// jedes Modul muss alle seine Handler in diese Liste eintragen
		if (isMessageInterface(Interfaces(interfaceId))) {
			//interface::Server<MsgProxy> &proxy = proxyList_[interfaceId].msgProxy();
			//std::cout << "TModule<MsgServer>::startProxy activating message " << InterfaceName[interfaceId] << " with protocol " << *protocolInfo << std::endl;
            for (auto it = proxyList_.begin(); it != proxyList_.end(); it++)
            {
                if (!it->isMessageProxy())
                {
                    continue;
                }
                if (it->msgProxy().info_.interfaceId != interfaceId || int(it->modId) != subAppId || it->path != path)
                {
                    continue;
                }
                it->msgProxy().activate(protocolInfo);
            }
		}	else {
            for (auto it = proxyList_.begin(); it != proxyList_.end(); it++)
            {
                if (!it->isEventProxy())
                {
                    continue;
                }
                if (it->eventProxy().info_.interfaceId != interfaceId || int(it->modId) != subAppId)
                {
                    continue;
                }
                it->eventProxy().activate(protocolInfo);
            }
		}
		//std::cout << "TModule<MsgServer>::startProxy ok" << std::endl;
	}


	/// spezifischen Server starten
	void TModule<MsgServer>::stopProxy(int interfaceId, SmpProtocolInfo & protocolInfo) {
		/// jedes Modul muss alle seine Handler in diese Liste eintragen
		if (isMessageInterface(Interfaces(interfaceId))) {
            for (auto it = proxyList_.begin(); it != proxyList_.end(); it++)
            {
                if (!it->isMessageProxy())
                {
                    continue;
                }
                if (it->msgProxy().info_.interfaceId != interfaceId)
                {
                    continue;
                }
                it->msgProxy().deActivate();
            }
		}	else {
            for (auto it = proxyList_.begin(); it != proxyList_.end(); it++)
            {
                if (!it->isEventProxy())
                {
                    continue;
                }
                if (it->eventProxy().info_.interfaceId != interfaceId)
                {
                    continue;
                }
                it->eventProxy().deActivate(protocolInfo);
            }
		}
	}


	void TModule<MsgServer>::waitForAllServers() {
		// first wait for shutdown-Message an TModule
		// ?????
		// if (numServersRunning_>0) serversReady_.wait();
		wmLog( eDebug, "TModule<MsgServer>::waitForAllServers().\n" );
		for (auto i=mHandlerList_.begin(); i!=mHandlerList_.end(); ++i)
		{
			Server<MsgHandler> &handler = *i->handler;
			handler.waitForServerEnd();
		}
		for (EventHandlerList::iterator i=eHandlerList_.begin(); i!=eHandlerList_.end(); ++i)
		{
			Server<EventHandler> &handler = *i->second.handler;
			handler.waitForServerEnd();
		}
	}

} // namespace interface
} // namespace precitec

