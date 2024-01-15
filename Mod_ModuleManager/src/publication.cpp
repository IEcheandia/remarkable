#include "publication.h"
#include "module/interfaces.h"	// wg NumApplications

namespace precitec
{
	//using system::message::EventSignaler;

	using system::module::FirstMessage;
	using system::module::NumMessages;
	using system::module::FirstEvent;
	using system::module::LastEvent;
	using system::module::NumApplications;
	using system::module::InterfaceName;
	using system::module::ModuleName;

namespace interface
{
	//Publication::Publication(int interfaceId) : interfaceId_(interfaceId), moduleEntry_(NULL) {}
	// std-CTor es wird immer ein ganzes _Interface veroeffentlicht
	Publication::Publication(Interfaces interfaceId, int moduleHandle, int numEvents, int subAppId, const std::string &path/*, ModuleEntry&	entry*/)
	: interfaceId_(interfaceId), moduleHandle_(moduleHandle), numEvents_(numEvents), subAppId_(subAppId), path_(path)/*,	moduleEntry_(&entry)*/ {}


	std::ostream& operator << (std::ostream&os, Publication const&p) {
		os << "Pub: ";
		if (	(p.interfaceId_>=FirstMessage)
			 && (p.interfaceId_<NumMessages) ) {
			os << "M-"	<< InterfaceName[p.interfaceId_] << " (0.." << p.numEvents_ << ")";
			//if (p.moduleEntry_) {	os << " ->Proxy: "	<< *(p.moduleEntry_); }
		} else if (	 (p.interfaceId_>=FirstEvent)
			 			 	&& (p.interfaceId_<LastEvent) ) {
			os << "E-"	<< InterfaceName[p.interfaceId_] << " (0.." << p.numEvents_ << ")";
			//if (p.moduleEntry_) { os << " ->Proxy: " << *p.moduleEntry_; }
		} else {
			os << "Invalid Interface";
		}
		return os;
	}


} // namespace interface
} // namespace precitec
