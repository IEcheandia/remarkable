#ifndef MODULE_PROXY_H_
#define MODULE_PROXY_H_

#include  "message/module.interface.h"
#include  "server/proxy.h"
#include  "Poco/SharedPtr.h"

namespace precitec
{
namespace interface
{

	/**
	 *  die Remote-Call-Spezialisierung ist recht simpel
	 * Fuer jede Member-Funktion des Basis-Servers gibt es eine
	 * Funktion mit gleicher Signatur.
	 * Nach initMessage wird jeder Parameter gemarshalled (in die Message verpackt)
	 * und dann mit send abgeschickt. Fuer nicht-void-funktionen wird der Returnparameter
	 * demarshalled (aus der Message entpackt) unr zurueckgegeben.
	 * Dass dies alles von Hand passiert hat den Vorteil, dass dieser Code
	 * ggf. Optimierungen bzw. Spezialcode fuer In-Out-Parameter enthalten kann.
	 * Dies den Kompiler automatisch machen zu lassen, ist vermutlich moeglich, wuerde aber
	 * sehr viel undurchsichtigen Template-Macro-Metaprogrammier-Code enthalten.
	 */
	template <>
	class TModule<MsgProxy> : public Server<MsgProxy>, public TModule<AbstractInterface>, public TModuleMessageDefinition {
	public:
		/// beide Basisklassen muessen geeignet initialisiert werden
		TModule() : PROXY_CTOR(TModule), TModule<AbstractInterface>() {
		}
		/// normalerweise wird das Protokoll gleich mitgeliefert
		TModule(SmpProtocolInfo & p) : PROXY_CTOR1(TModule,  p), TModule<AbstractInterface>()
		{
		}
		/// der DTor muss virtuell sein
		virtual ~TModule() {}
	public:
		// das gesamte virtuelle Interfae muss hier nachgebildet werden

		/// Server werden erst mit Bekanntwerden des Protokolls gestartet
		SmpProtocolInfo startServer(int interfaceId, SmpProtocolInfo & protocol, const std::string &path) override{
			// von der Message brauchen wir nur den index
			INIT_MESSAGE(StartServer);
			sender().marshal(interfaceId);
			sender().marshal(protocol);
            sender().marshal(path);
			sender().send();
			SmpProtocolInfo ret; sender().deMarshal(ret);
			return ret;
		}

		/// Server werden erst mit Bekanntwerden des Protokolls gestartet
		virtual void reStartServer(int interfaceId, SmpProtocolInfo &protocol)	{
			// von der Message brauchen wir nur den index
			INIT_MESSAGE(ReStartServer);
			sender().marshal(interfaceId);
			sender().marshal(protocol);
			sender().send();
		}

		/// Server werden erst mit Bekanntwerden des Protokolls gestartet
		virtual void stopServer(int interfaceId, SmpProtocolInfo &protocol)	{
			// von der Message brauchen wir nur den index
			INIT_MESSAGE(StopServer);
			sender().marshal(interfaceId);
			sender().marshal(protocol);
			sender().send();
		}

		/// die proxies erhalten das Protokoll bei der Aktivierung
		void startProxy(int interfaceId, SmpProtocolInfo & protocol, int subAppId, const std::string &path) override {
			// von der Message brauchen wir nur den index
			INIT_MESSAGE(StartProxy);
			sender().marshal(interfaceId);
			sender().marshal(protocol);
            sender().marshal(subAppId);
            sender().marshal(path);
			sender().send();
		}

		/// die proxies erhalten das Protokoll bei der Aktivierung
		virtual void stopProxy(int interfaceId, SmpProtocolInfo & protocol) {
			// von der Message brauchen wir nur den index
			INIT_MESSAGE(StopProxy);
			sender().marshal(interfaceId);
			sender().marshal(protocol);
			sender().send();
		}

		/**
		 * Programmende
		 */
		virtual void kill() {
			// von der Message brauchen wir nur den index
			INIT_MESSAGE(Kill);
			sender().send();
		}

	}; // TModule<MsgProxy>

} // namespace interface
} // namespace precitec

#endif /*MODULE_PROXY_H_*/
