#include "moduleEntry.h"
#include "message/module.proxy.h"
//#include "message/module.handler.h"
//#include "message/module.server.h"
//#include "message/registrar.proxy.h"
#include "message/registrar.handler.h"
#include "registrar.server.h"

namespace precitec
{
namespace interface
{

	ModuleEntry::ModuleEntry() : spec_(), info_(), module_(), server_(), handler_() {}

	/// der CTor fuer einen vollstaendigen Eintrag
	ModuleEntry::ModuleEntry(ModuleSpec const& spec, ModuleInfo const& info,
							MMAccessor &accessor, SmpProtocolInfo & proxyPInfo, SmpProtocolInfo & handlerPInfo)
	: spec_(spec), info_(info),
		module_(SmpModuleProxy(new TModule<MsgProxy>(proxyPInfo))),
		server_(new TRegistrar<MsgServer>(accessor)),
		handler_(new TRegistrar<MsgHandler>(server_))
	{
		//std::cout << "ModuleEntry CTor()" << (*this) << std::endl;
		// ab jetzt hoert der MM auf das neue Modul
		handler_->activate(handlerPInfo);
	}



	/// copy-CTor
	ModuleEntry::ModuleEntry(ModuleEntry const& rhs)
	: spec_(rhs.spec_), info_(rhs.info_),
		module_(rhs.module_),	server_(rhs.server_),	handler_(rhs.handler_) {}

	/**
	 *
	 * @param serverProtInfo das vom MM definierte Protokol
	 * @param interfaceId ein Modul hat fuer jedes Interface einen eigenen Server
	 * @return Client-Protokol, das vom Server-Protokol abhaengen kann
	 */
	SmpProtocolInfo	ModuleEntry::activateServer(int interfaceId, SmpProtocolInfo & serverProtInfo, const std::string &path) {
		//std::cout << "\t\tModuleEntry::activateServer: " << std::endl;
		return module_->startServer(interfaceId, serverProtInfo, path);
	}


	/**
	 *
	 * @param interfaceId ein Modul hat fuer jedes Interface einen eigenen Server
	 */
	void	ModuleEntry::reActivateServer(int interfaceId, SmpProtocolInfo & serverProtInfo) {
		//std::cout << "\t\tModuleEntry::reActivateServer: " << std::endl;
		return module_->reStartServer(interfaceId, serverProtInfo);
	}


	/**
	 *
	 * @param clientPInfo das Protokol
	 * @param interfaceId die verwendete Schnittstelle
	 * @param eId anders als der Server, koennen beim Client einzelne Events freigeschalten werden
	 */
	void ModuleEntry::activateClient(int interfaceId, SmpProtocolInfo & clientProtInfo, int /*eId=module::AllEvents*/, int subAppId, const std::string &path) {
		module_->startProxy(interfaceId, clientProtInfo, subAppId, path);
	}

	std::ostream& operator << (std::ostream&os, ModuleEntry const& me) {
		os << "ModuleEntry: <" << me.spec_  /*<< " . " << me.info_  */<< "> "; return os;
	}

} // namespace interface
} // namespace precitec
