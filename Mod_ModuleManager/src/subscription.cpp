#include <iostream>
#include "system/templates.h"
#include "subscription.h"
#include "module/interfaces.h"	// wg NumApplications

namespace precitec
{

	using system::module::FirstMessage;
	using system::module::NumMessages;
	using system::module::FirstEvent;
	using system::module::LastEvent;
	using system::module::NumApplications;
	using system::module::InterfaceName;
	using system::module::Interfaces;
	using system::module::ModuleName;

namespace interface
{

	//Subscription::Subscription(int moduleHandle, int interfaceId)
	//: moduleHandle_(moduleHandle), interfaceId_(interfaceId) {}


	Subscription::Subscription(int moduleHandle, int interfaceId, int appId, int event, const std::string &path)
	: moduleHandle_(moduleHandle), interfaceId_(Interfaces(interfaceId)), appId_(appId), eventId_(event), path_(path) {}


	std::ostream& operator << (std::ostream&os, Subscription const&s) {
		os << "Sub: ";
		if (	(s.interfaceId_>=FirstMessage) && (s.interfaceId_<NumMessages) ) {
			os << "M-" << InterfaceName[s.interfaceId_]; // << "(" << s.interfaceId_ << "#" << (s.eventId_>=0?toString(s.eventId_):"all") << ")";
			if ((s.appId_>0)&&(s.appId_<NumApplications))	os << " mod: <"	<< ModuleName[s.appId_] << ">";
		} else if ((s.interfaceId_>=FirstEvent) && (s.interfaceId_<LastEvent)) {
			os << "E-" << InterfaceName[s.interfaceId_]; // << "(" << s.interfaceId_ << "#" << (s.eventId_>=0?toString(s.eventId_):"all") << ")";
			if ((s.appId_>0)&&(s.appId_<NumApplications))	os << " mod: <"	<< ModuleName[s.appId_] << ">";
		} else {
			os << "<Invalid Interface>";
		}
		return os;
	}

} // namespace interface
} // namespace precitec


