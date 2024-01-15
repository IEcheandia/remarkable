#ifndef DEVICE_PROXY_H_
#define DEVICE_PROXY_H_

#include  "message/device.interface.h"
#include  "server/proxy.h"


namespace precitec
{
namespace interface
{
	/**
	 *	AbstrakteBasisklasse des Module Servers
	 * 	Der Modul-Server kommuniziert mit dem Module-Manager,
	 * 		d.h. er wartet auf Befehler dessselben
	 */
	using namespace  message;
	using namespace  system;

	// ?? using system::message::Sender;


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
	class TDevice<MsgProxy> : public Server<MsgProxy>, public TDevice<AbstractInterface>, public TDeviceMessageDefinition
	{
	public:
		// die Ctoren sind Standard und muessen nur umbenannt werden (inkl ResultArgsRangeServer<Messages>)
		/// beide Basisklassen muessen geeignet initialisiert werden
		TDevice() : PROXY_CTOR(TDevice), TDevice<AbstractInterface>()
		{
		}

		/// normalerweise wird das Protokoll gleich mitgeliefert
		TDevice(SmpProtocolInfo &p) : PROXY_CTOR1(TDevice,  p), TDevice<AbstractInterface>()
		{
		}
		/// der DTor muss virtuell sein
		virtual ~TDevice() {}
	public:
		// das gesamte virtuelle Interfae muss hier nachgebildet werden

		virtual int initialize(Configuration const &configuration, int subDevice=0)
		{
			// von der Message brauchen wir nur den index
			INIT_MESSAGE(Initialize);
			sender().marshal(configuration);
			sender().marshal(subDevice);
			sender().send();
			int ret = false;
            sender().deMarshal(ret);
			return ret;
		}

		virtual void uninitialize()
		{
			// von der Message brauchen wir nur den index
			INIT_MESSAGE(Uninitialize);
			sender().send();
		}

		virtual void reinitialize()
		{
			// von der Message brauchen wir nur den index
			INIT_MESSAGE(Reinitialize);
			sender().send();
		}

		virtual KeyHandle set(SmpKeyValue keyValue, int subDevice=0)
		{
			// von der Message brauchen wir nur den index
			INIT_MESSAGE(SetKeyValue);
			sender().marshal(keyValue);
			sender().marshal(subDevice);
			sender().send();
			KeyHandle ret; sender().deMarshal(ret);
			return ret;
		}

		virtual void  set(Configuration configuration, int subDevice=0)
		{
			// von der Message brauchen wir nur den index
			INIT_MESSAGE(SetConfiguration);
			sender().marshal(configuration);
			sender().marshal(subDevice);
			sender().send();
		}

		virtual SmpKeyValue get(Key key, int subDevice=0)
		{
			// von der Message brauchen wir nur den index
			INIT_MESSAGE(GetKey);
			sender().marshal(key);
			sender().marshal(subDevice);
			sender().send();
			SmpKeyValue ret; sender().deMarshal(ret);
			return ret;
		}

		virtual SmpKeyValue get(KeyHandle keyHandle, int subDevice=0)
		{
			// von der Message brauchen wir nur den index
			INIT_MESSAGE(GetHandle);
			sender().marshal(keyHandle);
			sender().marshal(subDevice);
			sender().send();
			SmpKeyValue ret; sender().deMarshal(ret);
			return ret;
		}

		virtual Configuration get(int subDevice=0)
		{
			// von der Message brauchen wir nur den index
			INIT_MESSAGE(GetConfiguration);
			sender().marshal(subDevice);
			sender().send();
			Configuration ret;
			sender().deMarshal(ret);
			return ret;
		}
	};

} // namespace interface
} // namespace precitec

#endif /*DEVICE_PROXY_H_*/
