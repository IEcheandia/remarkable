#ifndef SUBSCRIPTION_H_
#define SUBSCRIPTION_H_


#include <vector>

#include "Poco/SharedPtr.h"
#include "system/policies.h"
#include "module/interfaces.h"	// wg NumApplications
#include "protocol/protocol.info.h"

namespace precitec
{
	using system::message::SmpProtocolInfo;
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
	/**
	 * EventInterface ist eine EventListe mit ProtocollInfo und dem Id der
	 * konkreten Applikation (die erst vom MM/Registrar erzeugt wird)
	 * Eine Subscription ist erst eine Voranmeldung. Erst mit der Aktivierung
	 * werden Signale verschickt. Eine Subscription umfasst immer ein ganzes
	 * Interrface, waehrend die Aktivierung auch/typischer auf einzelne Signale
	 * angewandt wird.
	 */
	class Subscription {
	public:
		Subscription() {}
		// subscriber CTor
		Subscription(int moduleHandle, int interfaceId, int appId, int event, const std::string &path);

		~Subscription() {}
	public:
		/// eine Subscription kommt vom Client, erst der Registrar fuegt die appId hinzu
		void 				setAppId(int a) { appId_ = a; }
		int					appId() const { return appId_; }
		/// erst der Registrar fuegt das Protokoll hinzu
		void 				setProtocolInfo(SmpProtocolInfo const& p) { protocolInfo_ = p; }
		SmpProtocolInfo	&protocolInfo()  { return protocolInfo_; }

		Interfaces	interfaceId() const { return interfaceId_; }
		//void					setEvent() { eventId_ = event; }
		int					event() const { return eventId_;}
		int					handle() const { return moduleHandle_;}

		const std::string &path() const { return path_; }

		friend std::ostream& operator << (std::ostream&os, Subscription const&s);
	private:
		/// Pid des Server-Prozesses/Threads definiert Subscription eindeutig
		int						moduleHandle_;
		///
		Interfaces		interfaceId_;
		int						appId_;
		int						eventId_;
		SmpProtocolInfo		protocolInfo_; // muss das das Protocoll sein oder geht hier auch ProtocollInfo??????
		std::string path_;
	};


} // namespace interface
} // namespace precitec


#endif /*SUBSCRIPTION_H_*/
