#include "registrar.server.h"
#include "mmAccessor.h"
#include "publication.h"
#include "subscription.h"

#include "module/interfaces.h"

#include "protocol/protocol.info.h"
#include "protocol/protocol.udp.h"
#include "message/module.h"
#include  "module/interfaces.h"
#include "module/interfaces.h"	// wg NumApplications



namespace precitec
{
namespace interface
{

	using system::module::isMessageInterface;
	using system::module::Interfaces;
	using system::module::AllEvents;

	TRegistrar<MsgServer>::TRegistrar(MMAccessor	&ca)
		: mmAccessor_(ca) {}


	/// Am Anfang gibt es noch keine Module: numApps_ wird praeIncrementiert um die ModulNummer zu erzeugen ?????
	//int TRegistrar<MsgServer>::numApps__ = -1;




	/**	registerPublication = publish
	 * Achtung der MM selbst muss pruefen ob der Publisher einen subscriber braucht
	 * \return true
	 */
	bool TRegistrar<MsgServer>::publish(int moduleHandle, int interfaceId, int numEvents,
																			int pubAppId, int subAppId, PvString subPath)	{
		std::cout << "TRegistrar<MsgServer>::<" << ModuleName[pubAppId] << "> publishing " << InterfaceName[interfaceId] << std::endl;
		//std::cout << "TRegistrar<MsgServer>::Status Katalog" << std::endl;
		//mmAccessor_.listCatalogue(std::cout);
		//std::cout << "TRegistrar<MsgServer>::Status Subscriptions" << std::endl;
		//mmAccessor_.listSubscriptions(std::cout);
		// jede Publication wird einfach ans Ende angehaengt
		// 	mehrere apps duerfen die gleiche Publication haben

		// Erst eine Publication aus den vorhandenen Daten erstellen
		Publication publication(Interfaces(interfaceId), moduleHandle, numEvents, subAppId, subPath);
		// und nun via mmAccessor in die Listen eintragen
		PublicationIter pubIter = mmAccessor_.addPublication(moduleHandle, Interfaces(interfaceId), pubAppId, publication, numEvents, subAppId, subPath);

		// die Reihenfolge
		// nach jedem publish schauen wir, ob sich activierbare Paerchen gebildet haben
		//std::cout << "TRegistrar<MsgServer>::<" << ModuleName[pubAppId] << "> publish " << InterfaceName[interfaceId] << " end" << std::endl;
		// activateMatchingSubscriptions kann bei Messages ggf blocken. Dann wird es durch einen
		// passenden Subscriber (->true) oder durch das Programmende (->false) entblockt.

		//return mmAccessor_.activateMatchingSubscriptions(pubIter);
		bool notShutdown = mmAccessor_.activateMatchingSubscriptions(pubIter);
	/*	std::cout << "moduleList: " << std::endl; mmAccessor_.listModules(std::cout);
		std::cout << "connList: " << std::endl; mmAccessor_.listActivations(std::cout);
		std::cout << "waitList: " << std::endl; mmAccessor_.listWaiting(std::cout);*/
	//	std::cout << "TRegistrar<MsgServer>::<" << ModuleName[pubAppId] << "> published " << InterfaceName[interfaceId] << std::endl;
		return notShutdown;
	}

	void TRegistrar<MsgServer>::subscribe(int moduleHandle, int interfaceId, int numEvents,
																				int subAppId, int pubAppId, PvString path) {
	//	std::cout << "TRegistrar<MsgServer>::<" << ModuleName[subAppId] << ">subscribing " << InterfaceName[interfaceId] << std::endl;
		// jede Subscription wird einfach ans Ende angehaengt
		// 	mehrere apps duerfen die gleiche Subscription haben

		// find publication in catalogue_
		if ( isMessageInterface(Interfaces(interfaceId)) ) {
			Subscription subscription(moduleHandle, Interfaces(interfaceId), subAppId, AllEvents, path);
			//std::cout << subscription << " Status Katalog:" << std::endl;
			//mmAccessor_.listCatalogue(std::cout);
			//std::cout << "\tStatus Subscriptions" << std::endl;
			//mmAccessor_.listSubscriptions(std::cout);

			SubscriptionIter sub = mmAccessor_.addSubscription(subscription);
			// ggf haengende Msg-Clients aktivieren
			mmAccessor_.activateMatchingPublications(sub);
		} else {
			// events werden einzeln registriert da sie auch einzeln aktiviert werden koennen
			// 				z.Zt. werden sie vereinfacht wie Mesages behandelt (èn block)
			//std::cout << Subscription(moduleHandle, interfaceId, subAppId, AllEvents) << "	Status Katalog:" << std::endl;
			//mmAccessor_.listCatalogue(std::cout);
			//std::cout << "\tStatus Subscriptions" << std::endl;
			//mmAccessor_.listSubscriptions(std::cout);
			// vorerst alle Events en bloc
			Subscription subscription(moduleHandle, Interfaces(interfaceId), subAppId, AllEvents, path);
			mmAccessor_.addSubscription(subscription);
					/*
			for (int event=0; event<numEvents; ++event)	{
				Subscription subscription(moduleHandle, interfaceId, subAppId, event);
				mmAccessor_.addSubscription(subscription);
				// keine aktivierung
			}
			*/
		}
		//std::cout << "moduleList: " << std::endl; mmAccessor_.listModules(std::cout);
		//std::cout << "connList: " << std::endl; mmAccessor_.listActivations(std::cout);
		//std::cout << "waitList: " << std::endl; mmAccessor_.listWaiting(std::cout);
	//	std::cout << "TRegistrar<MsgServer>::<" << ModuleName[pubAppId] << ">subscribing ok " << InterfaceName[interfaceId] << std::endl;
//		std::cout << std::endl << std::endl << std::endl;
        wmLog(eInfo, "TRegistar: module: %s subscribe ok in %s \n", ModuleName[pubAppId], InterfaceName[interfaceId]);
	}

	void TRegistrar<MsgServer>::autoSubscribe(int moduleHandle, int interfaceId, int numEvents, int subAppId, int pubAppId, PvString path)
	{
	//	std::cout << "TRegistrar<MsgServer>::<" << ModuleName[subAppId] << ">autoSubscribing " << InterfaceName[interfaceId] << std::endl;
		wmLog(eInfo, "TRegistrar<MsgServer>::< %s > autosubscribing , interface: %s \n", ModuleName[subAppId], InterfaceName[interfaceId]);
        if ( isMessageInterface(Interfaces(interfaceId)) )
		{
			Subscription subscription(moduleHandle, Interfaces(interfaceId), subAppId, module::AllEvents, path);
			SubscriptionIter sub = mmAccessor_.addSubscription(subscription);
			mmAccessor_.activateMatchingPublications(sub);
		} else {
					//std::cout << Subscription(moduleHandle, interfaceId, subAppId, module::AllEvents) << std::endl;
					//std::cout << "	Status Katalog:" << std::endl;
					//mmAccessor_.listCatalogue(std::cout);
					//std::cout << "\t\tStatus Subscriptions" << std::endl;
					//mmAccessor_.listSubscriptions(std::cout);

			// vorerst alle Events en bloc
			Subscription subscription(moduleHandle, Interfaces(interfaceId), subAppId, module::AllEvents, path);
			SubscriptionIter sub = mmAccessor_.addSubscription(subscription);
			mmAccessor_.activateMatchingPublications(sub);
/*			for (int event=0; event<numEvents; ++event)	{
				Subscription subscription(moduleHandle, interfaceId, subAppId, event);
				SubscriptionIter sub = mmAccessor_.addSubscription(subscription);
				mmAccessor_.activateMatchingPublications(sub);
			}*/
		}
//		std::cout << "TRegistrar<MsgServer>::<" << ModuleName[subAppId] << ">autoSubscribing ok " << InterfaceName[interfaceId] << std::endl;
	}

	void TRegistrar<MsgServer>::kill() {
		mmAccessor_.kill();
	}




	//------------------------------------------------------------
	// Private Schnittstelle
	void TRegistrar<MsgServer>::activation(int handle, int eventId, bool activate) {
		// ActivatinoHandle ist eindeutiger Id fuer Subscription
		// finde Id in activeSubscriptionListe
	}

/*
	void TRegistrar<MsgServer>::deactivateSubscription(Subscription &subscription, int signalId)
	{
		SubscriptionIter sub = std::find(subscriptions_.begin(), subscriptions_.end(), subscription);
		if (sub==subscriptions_.end()) {
			// sollte niemlas passieren,
			// throw ExceptionXXX
			return;
		}
		// gibt es bereits publications fuer dieses Event-Interface?
		Publication tofind(subscription.interfaceId());
		PublicationIter pub = std::find(catalogue_.begin(), catalogue_.end(), tofind);
		while (pub!=catalogue_.end())
		{
			// alle publisher mit subscriber verbinden
			deactivate(pub, subscription, signalId);
			pub = std::find(++pub, catalogue_.end(), tofind);
		}
	}

		// called by Pub-/Sub-Dtor
	void TRegistrar<MsgServer>::unpublish(Publication &publication)
	{
		// for all activeSubs deactivate
		// for all subs privUnSubscribe
		// unpublish Pub
	}

	void TRegistrar<MsgServer>::unsubscribe(Subscription &subscription, int signalId)
	{
		// for all activeSubs deactivate
		// for all pubs privUnPublish
		// unSubscribe
	}
*/



/*
	void TRegistrar<MsgServer>::unPublish(PubicationListCIter &pub)
	{
	}
	*/
	void TRegistrar<MsgServer>::unSubscribe(SubscriptionCIter &sub, int signalId)
	{
	}




} // namespace interface
} // namespace precitec

