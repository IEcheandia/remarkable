#ifndef REGISTRAR_ITERATORS_H_
#define REGISTRAR_ITERATORS_H_

#include <list>
#include <vector>
#include <map>
#include <system/types.h> // wg PvString
//#include <system/typeTraits.h>
//#include "publication.h"
//#include "subscription.h"

namespace precitec
{
namespace interface
{
/*	using system::module::Subscription;
	using system::module::Publication;
	using system::module::ModuleEntry;
*/
	class Subscription;
	class Publication;
	class ModuleEntry;
	// die diversen Strukturen des Registrars halten diverse Listen
	// und Iteratoren darauf. Hier werden die Strukturen vorwaertsdeklariert.
	// Damit koennen die Iteratoren bedenkenlos eingesetzt werden.

	typedef std::list<Subscription>						SubscriptionList;
	typedef SubscriptionList::iterator 				SubscriptionIter;
	typedef SubscriptionList::const_iterator 	SubscriptionCIter;


	typedef std::list<Publication> 						PublicationList;
	typedef PublicationList::iterator 				PublicationIter;
	typedef PublicationList::const_iterator 	PublicationCIter;

//	class		ModuleSpec;
//	typedef std::map<int, ModuleSpec> 				PublisherList; // key = interfaceId
//	typedef PublisherList::iterator 					PublisherIter;
//	typedef PublisherList::const_iterator 		PublisherCIter;

	class		Proxyer;
	typedef std::list<Proxyer*> 							ProxyerList;
	typedef ProxyerList::iterator 						ProxyerIter;
	typedef ProxyerList::const_iterator 			ProxyerCIter;


} // namespace interface
} // namespace precitec

#endif /*REGISTRAR_ITERATORS_H_*/
