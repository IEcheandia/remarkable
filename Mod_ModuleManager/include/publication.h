#ifndef PUBLICATION_H_
#define PUBLICATION_H_

#include <list>

#include "Poco/SharedPtr.h"
#include "message/messageSender.h"
#include "message/eventSignaler.h"
#include "system/policies.h"
#include "module/interfaces.h"

namespace precitec
{
	//using system::message::EventSignaler;
namespace interface
{

	class Subscription;
	class ModuleEntry;
	typedef std::list<ModuleEntry> 	ModuleList;
	typedef ModuleList::iterator ModulIter;
	using system::module::Interfaces;

	/**
	 * Publication ist das von einem Modul veroeffentlichte Interface.
	 * Die Publication verbindet Interface mit Applikation und dem konkreten
	 * EventSignaler.
	 * Der Registrar haelt eine Liste von den Publications aller Module.
	 * Ein Server veroeffentlicht maximal ein Interface. Aber ein Modul kann
	 * von mehreren Server ableiten und damit meherere Interfaces veroeffentlichen.
	 * Der eventSignaler gehoert zum Server, die AppId gehoert zum Modul.
	 */
	class Publication {
	public:
		//typedef Poco::SharedPtr<EventSignaler, Poco::ReferenceCounter, Poco::ReleasePolicy<EventSignaler> > SmpEventSignaler;
	public:
		/// leerer CTor wg std::container
		Publication() {}
		// std-CTor es wird immer ein ganzes _Interface veroeffentlicht
		//Publication(int interfaceId);
		// std-CTor es wird immer ein ganzes _Interface veroeffentlicht, eizelne Events aktiviert
		Publication(Interfaces interfaceId, int moduleHandle, int numEvents, int subAppId, const std::string &path/*, ModuleEntry&	entry*/);

		~Publication() {}
	public:
		// das eigentliche Client-Interface, Schnittstellen koennen pro Signal oder in toto abboniert werden
		void 	addSubscription(Subscription const& s, int signalId);
		void 	removeSubscription(Subscription const& s, int signalId);
	public:
		// das Registrar Interface, ggf: privat machen + Registrar friend??

		/// der Registrar traegt die ID der subskribierenden Applikaiton ein
		void 				setModuleHandle(int h) { moduleHandle_ = h; }

		/// wird vom Registrar eingetragen
		//void				setModuleEntry(ModulIter entry) { moduleEntry_ = entry; }

		///	wird fuer Benachrichtigungen/Fehlerbehebungen benoetigt
		int					handle() const { return moduleHandle_; }
		int					numEvents() const { return numEvents_; }
		Interfaces			interfaceId() const { return interfaceId_; }
		//void 				setModuleId(int a) { interfaceId_ = a; }
		//ModulIter		moduleEntry() const { return moduleEntry_; }
		int subAppId() const { return subAppId_; }

		const std::string &path() const { return path_; }

		// die Vergleichsoperatoren erlaben das Einfuegen in alle Std-Container
		bool operator < (Publication const& rhs)	{	return (interfaceId_ < rhs.interfaceId_ && subAppId_ < rhs.subAppId() ); }
		bool operator == (Publication const& rhs) {	return interfaceId_ == rhs.interfaceId_ && subAppId_ == rhs.subAppId();	}

		friend std::ostream& operator << (std::ostream&os, Publication const&p);

	private:
		/// das Interface
		Interfaces			interfaceId_;
		/// die Subscriber-Applikation (ProcessId)
		int					moduleHandle_;
		/// Anzahl der Events: noetig, da die einzelnen Signale ueber den Publisher (de)aktiviert werden
		int					numEvents_;
		///	Iter des Modul-Eintrags -> Proxy des Moduls
		//ModulIter		moduleEntry_;
        int subAppId_;
        std::string path_;
	};


} // namespace interface
} // namespace precitec

#endif /*PUBLICATION_H_*/
