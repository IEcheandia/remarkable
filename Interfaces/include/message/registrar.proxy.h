#ifndef REGISTRAR_PROXY_H_
#define REGISTRAR_PROXY_H_

#include  "Poco/TypeList.h"
#include  "message/registrar.interface.h"
#include  "server/proxy.h"
#include  "system/types.h"
#include  "module/moduleLogger.h"

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
	class TRegistrar<MsgProxy> : public Server<MsgProxy>, TRegistrar<AbstractInterface>, public TRegistrarMessageDefinition
	{
	public:
		/// beide Basisklassen muessen geeignet initialisiert werden
		TRegistrar() : PROXY_CTOR(TRegistrar), TRegistrar<AbstractInterface>() {
		}
		/// normalerweise wird das Protokoll gleich mitgeliefert
		TRegistrar(SmpProtocolInfo & p) : PROXY_CTOR1(TRegistrar,  p), TRegistrar<AbstractInterface>() {
		}
		/// der DTor muss virtuell sein
		virtual ~TRegistrar() {}
	public:
		// das gesamte virtuelle Interfae muss hier nachgebildet werden

#define PPRINT(name) wmLog(eDebug, #name\n);

#define PRINT(name) PPRINT(name)

		//	registerPublication = publish
		/// Achtung der MM selbst muss pruefen ob der Publisher einen subscriber braucht
		virtual bool publish(int moduleHandle, int interfaceId, int numEvents, int pubAppId, int subAppId=AnyModule, PvString subPath="") {
			// von der Message brauchen wir nur den index
			//typedef TRegistrar<Messages>::publish_int_int_int_int_int_PvString Msg1;
			INIT_MESSAGE(Publish);
			sender().marshal(moduleHandle);
			sender().marshal(interfaceId);
			sender().marshal(numEvents);
			sender().marshal(pubAppId);
			sender().marshal(subAppId);
			sender().marshal(subPath);
			sender().send();
			bool notShuttingDown = false;
            sender().deMarshal(notShuttingDown);
			return notShuttingDown;
		}

		virtual void subscribe(int moduleHandle, int interfaceId, int numEvents, int subAppId, int pubAppId=AnyModule, PvString pubPath="") {
			// von der Message brauchen wir nur den index
			INIT_MESSAGE(Subscribe);
			sender().marshal(moduleHandle);
			sender().marshal(interfaceId);
			sender().marshal(numEvents);
			sender().marshal(subAppId);
			sender().marshal(pubAppId);
			sender().marshal(pubPath);
			sender().send();
		}

		virtual void autoSubscribe(int moduleHandle, int interfaceId, int numEvents, int subAppId, int pubAppId=AnyModule, PvString pubPath="") {
			// von der Message brauchen wir nur den index
			INIT_MESSAGE(AutoSubscribe);
			sender().marshal(moduleHandle);
			sender().marshal(interfaceId);
			sender().marshal(numEvents);
			sender().marshal(subAppId);
			sender().marshal(pubAppId);
			sender().marshal(pubPath);
			sender().send();
		}

		virtual void activation(int handle, int eventId, bool activate) {
			// von der Message brauchen wir nur den index
			INIT_MESSAGE(Activation);
			sender().marshal(handle);
			sender().marshal(eventId);
			sender().marshal(activate);
			sender().send();
		}

		virtual void kill()
		{
			// von der Message brauchen wir nur den index
			INIT_MESSAGE(Kill);
			sender().send();
		}

		virtual void unpublish(int moduleHandle, int interfaceId, int numEvents, int pubAppId, int subAppId=AnyModule, PvString subPath="")	{
			INIT_MESSAGE(Unpublish);
			sender().marshal(moduleHandle);
			sender().marshal(interfaceId);
			sender().marshal(numEvents);
			sender().marshal(pubAppId);
			sender().send();
		}

		virtual void unsubscribe(int moduleHandle, int interfaceId, int numEvents, int subAppId, int pubAppId=AnyModule, PvString pubPath="") {
			INIT_MESSAGE(Unsubscribe);
			sender().marshal(moduleHandle);
			sender().marshal(interfaceId);
			sender().marshal(numEvents);
			sender().marshal(subAppId);
			sender().send();
		}


	}; // TRegistrar<Proxy>

} // namespace interface
} // namespace precitec

#endif /*REGISTRAR_PROXY_H_*/
