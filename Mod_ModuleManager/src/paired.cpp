#include "paired.h"
#include "publication.h"
#include "subscription.h"

namespace precitec
{
namespace interface
{
	using system::module::AllEvents;
	using system::message::SmpProtocolInfo;

		/// Subscription-MicroGuid-Generator
		int ActivationPair::counter__ = 0;


		/// leer Subscription (wg container)
		ActivationPair::ActivationPair()
		: interfaceId_(), pub_(), sub_(), active_(0), microGuid_(counter__++) {}

		/// copy CTor
		ActivationPair::ActivationPair(ActivationPair const& as)
		: interfaceId_(as.interfaceId_), pub_(as.pub_), sub_(as.sub_), active_(as.active_),microGuid_(counter__++) {}

		/**
		 * std-CTor mit Daten
		 * das Interface haengt am subscriber
		 */
		ActivationPair::ActivationPair(PublicationIter p, SubscriptionIter s, SmpProtocolInfo &pInfo)
		: interfaceId_(s->interfaceId()),
			pub_(p),
			sub_(s),
			clientProtInfo_(pInfo),
			active_(0),
			microGuid_(counter__++) {
			// \todo sub/pub auf Konsistez wg interface pruefen
		}

		/// Eintrag in Litenelement aktualisieren; -> true wenn tatsaechlich Aenderung erfolgt ist
		bool	ActivationPair::activate(int eId) {
		//	std::cout << "ActivationPair::activate: " << std::endl;
			// Maske der zulaessigen Bits
			int fullMask = mask(pub_->numEvents());
			// Bit maskieren / bzw. alle legalen Bits setzen wenn alle gesetzt werden soolen
			int activeMask = (eId==AllEvents) ? fullMask : fullMask & activeBit(eId);
			// wenn das Bit wegmaskiert wurde, war es illegal
			if (activeMask==0) return false;
			// wenn es schon gesetzt wurde muessen wir nichts tun
			if (active_&activeMask) return false;
			// das gewuenschte Bit setzen
			active_ |= activeMask;
			return true;
		}


	/*
		bool ActivationPair::operator < (ActivationPair const& rhs)
		{
			return 		 (    &*pub_ <  &*rhs.pub_ )
							|| (	 (&*pub_ == &*rhs.pub_ )
									&& (  (  &*sub_ <  &*rhs.sub_ )
										 || ( (&*sub_ == &*rhs.sub_ )
										  		&& (eventId_ < rhs.eventId_ ) ) ) );
		}
		*/
		bool ActivationPair::operator == (ActivationPair const& rhs)
		{
			return 		 (pub_ == rhs.pub_ )
									&& (sub_ == rhs.sub_ );
		}

		std::ostream &operator << (std::ostream &os, ActivationPair const& p) {
			os 	<< "PairNum: " << p.microGuid_ << " serving " << InterfaceName[p.interfaceId_] << " {" << *p.pub() << " --- " << *p.sub() << ": " ; //os.flush();
			ActivationPair::maskToString(os, p.active_, p.pub()->numEvents());
			//os.flush();
			os	<< "}";
			return os;
		}

		void ActivationPair::maskToString(std::ostream &os, int mask, int numEvents){
			for (int i=0; i<numEvents; ++i) {
				os << ((activeBit(numEvents) & mask) ? "*" :"-");
			}
		}

} // namespace interface
} // namespace precitec

