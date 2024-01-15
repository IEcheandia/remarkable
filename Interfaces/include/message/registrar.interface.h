#ifndef REGISTRAR_INTERFACE_H_
#define REGISTRAR_INTERFACE_H_

#include  "server/interface.h"
#include  "module/interfaces.h" // wg appId
#include  "protocol/protocol.info.h"
#include  "protocol/protocol.udp.h"

#include  "message/module.h"
#include  "message/registrar.iterators.h"
//#include  "message/registrar.activeSubscription.h"

namespace precitec
{
	using system::message::ProtocolInfo;
	using system::message::Message;
	using system::message::UdpProtocol;
	using system::module::AnyModule;
	using system::module::Registrar;

namespace interface
{

	template <int CallType>
	class TRegistrar;

	/**
	 *	AbstrakteBasisklasse des Module Servers
	 * 	Der Modul-Server kommuniziert mit dem Module-Manager,
	 * 		d.h. er wartet auf Befehler dessselben
	 */
	template <>
	class TRegistrar<AbstractInterface>
	{
	public:
		TRegistrar() {}
		virtual ~TRegistrar() {}
	public:
		// Interface zum Modul
		//	registerPublication = publish
		/// Client gibt Verwendung eines Interfaces frei; er kann eine ServerApp und den Pfad vorschreiben
		virtual bool publish(int moduleHandle, int interfaceId, int numEvents, int pubAppId, int subAppId=AnyModule, PvString subPath="") = 0;
		/// Server gibt die Behandlung eines Interfaces bekannt; er kann Client AppId und Pfad vorschreiben
		virtual void subscribe(int moduleHandle, int interfaceId, int numEvents, int subAppId, int pubAppId=AnyModule, PvString path="") = 0;
		/// wie subscribe, jedoch mit automatischer Aktivierung
		virtual void autoSubscribe(int moduleHandle, int interfaceId, int numEvents, int subAppId, int pubAppId=AnyModule, PvString path="") = 0;

		/// Client beeendet Freigaben
		//virtual void unpublish(int moduleHandle, int interfaceId, int numEvents, int pubAppId) = 0;
		/// Server gibt das Ende der Behandlung eines Interfaces bekannt
		//virtual void unsubscribe(int moduleHandle, int interfaceId, int numEvents, int subAppId) = 0;

		virtual void kill() = 0;
		/// clear tableEntries
		//virtual void clearApplication(int appId) = 0;
		/// clear tableEntries
		//virtual void clearInterface(int interfaceId) = 0;
	public:
		// Interface zum Konfigure-Interface
		/// setzt den aktivierungsStatus von Sub/Pub-Paerchen
		virtual void activation(int activationHandle, int eventId, bool activate) = 0;

		// trivialInterface fuer Listen (numElements; Element(index))
		//virtual int getNumPublications() = 0;
		//virtual Publication getPublications(int index) = 0;
		//virtual int getNumSubscriptions() = 0;
		//virtual Subscription getSubscription(int index) = 0;
//		virtual int getNumActivationPairs() = 0;
//		virtual ActivationPair getActivationPair(int index) = 0;
	};

    struct TRegistrarMessageDefinition
    {
		MESSAGE(bool, Publish, int, int, int, int, int, PvString);
		MESSAGE(void, Subscribe, int, int, int, int, int, PvString);
		MESSAGE(void, AutoSubscribe, int, int, int, int, int, PvString);
		MESSAGE(void, Activation, int, int, bool);
		MESSAGE(void, Kill, void);
		MESSAGE(void, Unpublish, int, int, int, int);
		MESSAGE(void, Unsubscribe, int, int, int, int);

		MESSAGE_LIST(
			Publish,
			Subscribe,
			AutoSubscribe,
			Activation,
			Kill,
			Unpublish,
			Unsubscribe
		);
    };

	template <>
	class TRegistrar<Messages> : public Server<Messages>, public TRegistrarMessageDefinition {
	public:
		TRegistrar() : info(Registrar, sendBufLen, replyBufLen, MessageList::NumMessages) {}//, handlers) {}
		MessageInfo info;
	private:
		/// Konstanten wg Lesbarkeit, diese koennten auch in der Basisklasse stehen, wuerden dann aber wohl kaum verwendet
		enum { Bytes=1, KBytes=1024, MBytes=1024*1024};
		enum { sendBufLen  = 500*Bytes, replyBufLen = 500*Bytes, handlers=6 };
	};

} // namespace interface
} // namespace precitec


#endif /*REGISTRAR_INTERFACE_H_*/
