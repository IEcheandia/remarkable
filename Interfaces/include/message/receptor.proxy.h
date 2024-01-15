#ifndef RECEPTOR_PROXY_H_
#define RECEPTOR_PROXY_H_

#include  "message/receptor.interface.h"
#include  "server/proxy.h"
#include  "system/types.h"

namespace precitec
{
namespace interface
{
	/**
	 *	AbstrakteBasisklasse des Module Servers
	 * 	Der Modul-Server kommuniziert mit dem Module-Manager,
	 * 		d.h. er wartet auf Befehler dessselben
	 */
	//using namespace  message;
	//using message::UdpProtocol;


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
	class TReceptor<MsgProxy> : public Server<MsgProxy>, TReceptor<AbstractInterface>, public TReceptorMessageDefinition {
	public:
		/// beide Basisklassen muessen geeignet initialisiert werden
		TReceptor() : PROXY_CTOR(TReceptor), TReceptor<AbstractInterface>()	{
		}
		/// normalerweise wird das Protokoll gleich mitgeliefert
		TReceptor(SmpProtocolInfo & p)
		: PROXY_CTOR1(TReceptor,  p), TReceptor<AbstractInterface>() {
		}
		/// der DTor muss virtuell sein
		virtual ~TReceptor() {}
	public:
		// das gesamte virtuelle Interfae muss hier nachgebildet werden

		/// Server werden erst mit Bekanntwerden des Protokolls gestartet
		virtual void registerModuleManager(int interfaceId, SmpProtocolInfo & protocol)	{
			// von der Message brauchen wir nur den index
			INIT_MESSAGE(RegisterModuleManager);
			sender().marshal(interfaceId);
			sender().marshal(protocol);
			sender().send();
		}

		virtual SmpProtocolInfo registerModule(int moduleHandle, ModuleSpec spec, ModuleType type) {
			// von der Message brauchen wir nur den index
			INIT_MESSAGE(RegisterModule);
			sender().marshal(moduleHandle);
			sender().marshal(spec);
			sender().marshal(type);
			sender().send();
			SmpProtocolInfo ret; sender().deMarshal(ret);
			return ret;
		}

		virtual void unregisterModule(int moduleHandle) {
			// von der Message brauchen wir nur den index
			INIT_MESSAGE(UnregisterModule);
			sender().send();;
		}

	}; // TReceptor<Proxy>

} // namespace interface
} // namespace precitec

#endif /*RECEPTOR_PROXY_H_*/
