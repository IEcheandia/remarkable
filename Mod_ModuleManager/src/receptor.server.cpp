#include "receptor.server.h"
#include "mmAccessor.h"

#include "common/connectionConfiguration.h"
#include "module/interfaces.h"
#include "moduleEntry.h"
#include "message/module.proxy.h"
#include "message/module.h"
#include "paired.h"

#include "protocol/protocol.info.h"
#include "protocol/protocol.udp.h"


namespace precitec
{

namespace interface
{
	using system::module::isMessageInterface;
	using system::module::Interfaces;

	TReceptor<MsgServer>::TReceptor(MMAccessor	&ca)
	: mmAccessor_(ca),
		baseMMProtocol_(new SocketInfo(Udp, "127.0.0.1", ConnectionConfiguration::instance().getString("MMProtocol.Port", "49910"))),
		mmProtocolCursor_(0)	{
			//std::cout << "TReceptor::CTor" << std::endl;
		// der Registrar generiert Protokolle, die sich aus diesen 'Basisprotokollen' ableiten
	}

	std::ostream & operator << ( std::ostream &os, TReceptor<MsgServer>  const& r ) {
		r.mmAccessor_.listModules(os);
		return os;
	}


	/**
	 * Die MM-Kommunikation findet immer mit dem UdpProtokoll statt, da es immer funktioniert
	 * Explizit das QNX-Protokoll funktioniert (nach bisherigem Kenntnisstand nicht),
	 * da es verlangt, dass erst der Server steht, und daraus das Client-Protokoll erzeugt wird
	 * (Server erzeugt Channel,zu dem Cleint connectAttach macht). Hier wiederum
	 * wird der Client sofort angelegt und das Server-Protooll zurueckgegeben.
	 */
	SmpProtocolInfo TReceptor<MsgServer>::registerModule(int moduleHandle, ModuleSpec spec,
																											 ModuleType type) {
	//	std::cout << "TReceptor<MsgServer>::registerModule " << spec << std::endl;
		//std::cout << "TReceptor<MsgServer>::Status" << std::endl;
		// debug-Ausgabe
	//	std::cout << "moduleList:" << std::endl;
		mmAccessor_.listModules(std::cout);

		// Alle Module bedienen das Modul-Interface mit den UDP-Protokol! s.o.
		// Auf der Gegenseite (Modulmanager) hoert der Registrar auf ale Module -> 2 neue Protokolle
		// die Reihenfolge ist ntscheidend (s. Rueckgabe)
		SmpProtocolInfo modulePInfo			= baseMMProtocol_->generateDerived(++mmProtocolCursor_);
		SmpProtocolInfo registrarPInfo	= baseMMProtocol_->generateDerived(++mmProtocolCursor_);

		// ueber  den mmAccessor greifen wir auf gemeinsame Datenstukturen des MM zu. Der Registrar ist der
		// fuer das Modul dezidierte MsgServer, ,der auf Meldungen des Moduls hoert
		mmAccessor_.addModule(spec, moduleHandle, type, modulePInfo, registrarPInfo);

		// die Rueckgabe geht an das Modul, das natuerlich auch diese Angabe braucht
		// hier wird ein uebler Trick verwendet: der Einfachheit halber wird nur ein Protokol
		// zurueckgegeben, das andere systematisch daraus erzeugt -> das erst erzeugte Protokoll
		// wird zurueckgegeben. Dieser Hack muss natuerlcih irgendwann einmal entfernt werden!!!
		//std::cout << "TReceptor<MsgServer>::registerModule ok " << spec << std::endl;
		return  modulePInfo;
	}

	void TReceptor<MsgServer>::unregisterModule(int moduleHandle) {
	//	std::cout << "TReceptor<MsgServer>::unregisterModule " << std::endl;
	//	std::cout << "TReceptor<MsgServer>::Status" << std::endl;
		mmAccessor_.listModules(std::cout);

		// es gibt zwei Faelle:
		//		das Modul existiert noch, dann sollten wir es killen
		//		es existiert nicht mehr, dann sollten wir es in Fieden lassen
		// -> killen in try-catch-Block
		try {
			mmAccessor_.unsubscribeAll(moduleHandle);
			mmAccessor_.unpublishAll(moduleHandle);
		} catch (...) {
			// Module bereits tot, ist ok
		}
		//std::cout << "TReceptor<MsgServer>::removeModule" << std::endl;
		mmAccessor_.removeModule(moduleHandle);
	//	std::cout << "TReceptor<MsgServer>::unregisterModule ok" << std::endl;


	}


} // namespace interface
} // namespace precitec

