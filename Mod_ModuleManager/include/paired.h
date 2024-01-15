#ifndef PAIRED_H_
#define PAIRED_H_

#include <list>
#include <iostream>
#include "message/registrar.iterators.h"
#include "module/interfaces.h"
#include "protocol/protocol.info.h"

namespace precitec
{
namespace interface
{
	//using interface::PublicationIter;
	//using interface::SubscriptionIter;
	using system::module::AllEvents;
	using system::message::SmpProtocolInfo;

	class ActivationPair {
	public:
		/// leer Subscription (wg container)
		ActivationPair();
		/// copy CTor
		ActivationPair(ActivationPair const& as);
		/// std-CTor mit Daten
		ActivationPair(PublicationIter p, SubscriptionIter s, SmpProtocolInfo & pInfo);
		~ActivationPair() {}
	public:
		/// accessor
		PublicationIter  pub() const { return pub_; }
		/// accessor
		SubscriptionIter sub() const { return sub_; }

		bool	activate(int eId=AllEvents);
		//bool operator < (ActivationPair const& rhs);
		bool operator == (ActivationPair const& rhs);
		/// Accessor
		SmpProtocolInfo		clientProtocol() const { return clientProtInfo_; }

		/// std-Ausgabe
		friend std::ostream &operator << (std::ostream &os, ActivationPair const& p);
	private:
		///
		int interfaceId() const { return interfaceId_; }
		/// bitmaske mit jedem Bit gesetzt
		static int mask(int numEvents) { return activeBit(numEvents) -1; }
		/// gibt Bit in Maske fuer Event zurueck
		static int activeBit(int eventNum) { return (1L << eventNum); }
		/// git die Maske als StringMuster aus (*=active, -=inactive)
		static void maskToString(std::ostream &os, int mask, int numEvents);

	private:
		int								interfaceId_; /// \todo Ist das einfach redundant wg sub->interfaceId() oder effizient ???
		PublicationIter  	pub_;
		SubscriptionIter 	sub_;
		/// pInfo wird hier gespeichert, weil eine Pub je Server/Subscription ein Protokol hat
		SmpProtocolInfo		clientProtInfo_;
		/// Bitmap aller (in)aktiven events
		int								active_;
		/// Identifier nach aussen, hierueber werden die Subscriptions erfasst
		int								microGuid_;
	private:
		/// Generator fuer die microGuids
		static int				counter__;
	};
	/// der Registrar verwendet diese Liste, alle elemente koennen getrost, kopiert werden -> std::vector ok
	//typedef std::vector<ActivationPair> ActivationPairList;
	typedef std::list<ActivationPair>			PairedList;
	// und die Iteratoren
	typedef PairedList::iterator 				PairedIter;
	typedef PairedList::const_iterator 	PairedCIter;


} // namespace interface
} // namespace precitec


#endif /*PAIRED_H_*/
