#ifndef REGISTRAR_SERVER_H_
#define REGISTRAR_SERVER_H_

#include  "message/registrar.interface.h"
#include  "message/registrar.iterators.h"
#include  "paired.h"
#include  "message/module.interface.h"

#include  "message/messageSender.h"
#include  "protocol/protocol.info.h"

namespace precitec
{
namespace interface
{


//	using interface::TRegistrar;
	using system::message::NumActiveProtocols;

	class MMAccessor;
	/**
	 * Hier wird nun der Registrar-Server implementiert
	 */
	template <>
	class TRegistrar<MsgServer> : public TRegistrar<AbstractInterface> {
	public:
		TRegistrar(MMAccessor	&ca);

		virtual ~TRegistrar() {}
	public:
		//	registerPublication = publish; Achtung !!! publish und subscribe sind proSignal, nicht pro Interface
		/// Achtung der MM selbst muss pruefen ob der Publisher einen subscriber braucht
		virtual bool publish(int moduleHandle, int interfaceId, int numEvents, int pubAppId, int subAppId=AnyModule, PvString subPath="");
		/// Achtung !!! publish ist pro Signal, nicht pro Interface ???? per Implementierung noch nicht
		virtual void subscribe(int moduleHandle, int interfaceId, int numEvents, int subAppId, int pubAppId=AnyModule, PvString pubPath="");
		/// wie subscribe, jedoch mit automatischer Aktivierung
		virtual void autoSubscribe(int moduleHandle, int interfaceId, int numEvents, int subAppId, int pubAppId=AnyModule, PvString pubPath="");
		/// Achtung !!! Aktivierungen sind pro Signal, nicht pro Interface
		virtual void activation(int handle, int eventId, bool activate);
		/// !!!! Achtung sollte eigentlich nicht in diesem Interface sei ????
		virtual	void kill();
	public:
		// MM-Interface
		void activateSubscription(Subscription &subscription, int eventId=module::AllEvents);

		void deactivateSubscription(Subscription &subscription, int eventId=module::AllEvents);

	private:
		/// called by Pub-Dtor
		void unpublish(Publication &publication);
		/// called by Sub-Dtor
		void unsubscribe(Subscription &subscription);

		/// Interface von Publisher steht nicht mehr zur Verfuegung
		void unPublish(PublicationCIter &pub);

		/// Interface kann nicht mehr aktiviert werden
		void unSubscribe(SubscriptionCIter &sub, int signalId);

		virtual void clearApplication(int appId)
		{
		}

		virtual void clearInterface(int interfaceId)
		{
		}


		//PublicationIter startDefaultPublisher(int InterfaceId);

		void loadModule(MessageHandlerList &mList, EventHandlerList &eList);


	private:
		/// per Konfiguration gesetzte Lister der Default Publisher-Apps
		//PublicationIter	defaultPublishers_;
		MMAccessor			&mmAccessor_;

		// die global-eindeutige Applikations-Nummer
		int 					appId_;
	private:

		static int 		numApps__;
	}; // TRegistrar


} // namespace interface
} // namespace precitec

#endif /*REGISTRAR_SERVER_H_*/
