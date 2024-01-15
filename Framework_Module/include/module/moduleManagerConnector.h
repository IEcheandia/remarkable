#pragma once

#include "message/module.handler.h"
#include "message/module.server.h"
#include "message/receptor.proxy.h"
#include "message/registrar.proxy.h"
#include "protocol/protocol.info.h"
#include "module/interfaces.h"

namespace precitec
{

namespace interface
{
class ConnectionConfiguration;
}

namespace framework
{
namespace module
{

class ModuleManagerConnector
{
public:
    ModuleManagerConnector(precitec::system::module::Modules moduleId);
    virtual ~ModuleManagerConnector();

    /** wird von Main aufgerufen um die Server aufzusetzen
     *  startet den Module-Server und dann die Std-Initialisierung
     */
    void startRegistrar(SmpProtocolInfo &moduleServerProtocolInfo);

    /// accessor fuer appId
    precitec::system::module::Modules getMyAppId() const;

    void registerPublication(precitec::interface::PMProxy proxy, precitec::system::module::Modules appId=AnyModule, PvString const& path=""); // importInterface
    void registerSubscription(precitec::interface::PMHandler handler, precitec::system::module::Modules appId=AnyModule, PvString const& path=""); // exportInterface

protected:
    void initModuleManager(precitec::interface::ConnectionConfiguration &config);

    /**
     * Default implementation returns an empty SmpProtocolInfo.
     **/
    virtual SmpProtocolInfo readConfig();

    precitec::interface::TRegistrar<precitec::interface::MsgProxy> &registrar() const;
    precitec::interface::TReceptor<precitec::interface::MsgProxy> &receptor() const;

    /// die Prozess Id -> eindeutiger Ident fuer alle module (innerhalb eines Rechners)
    int getModuleHandle() const;
    /// Pfad + AppId in Spec-Struktur packen
    virtual precitec::interface::ModuleSpec getMyModuleSpec() = 0;

    /// registriere Publication in der Liste
    bool publishAllInterfaces();
    void subscribeAllInterfaces();


    /// muss vor startModule aufgerufen werden, macht ein bereitgestelltes Interface bekannt
    void registerSubscription(precitec::interface::PEHandler handler, precitec::system::module::Modules appId=AnyModule, PvString const& path=""); // exportInterface
    /// muss vor startModule aufgerufen werden, macht ein verwendetes Interface bekannt
    void registerPublication(precitec::interface::PEProxy proxy, precitec::system::module::Modules appId=AnyModule, PvString const& path=""); // importInterface

    /// alternatives Interface
    void exportInterface(precitec::interface::PMHandler handler, precitec::system::module::Modules appId=AnyModule, PvString const& path=""); // exportInterface
    void exportInterface(precitec::interface::PEHandler handler, precitec::system::module::Modules appId=AnyModule, PvString const& path=""); // exportInterface
    /// muss vor startModule aufgerufen werden, macht ein verwendetes Interface bekannt
    void importInterface(precitec::interface::PMProxy proxy, precitec::system::module::Modules appId=AnyModule, PvString const& path=""); // importInterface
    void importInterface(precitec::interface::PEProxy proxy, precitec::system::module::Modules appId=AnyModule, PvString const& path=""); // importInterface

private:
    /// nimmt Kontakt mit Regitrar auf, startet dieses wenn noetig
    void startReceptor(precitec::interface::ConnectionConfiguration &config);

    precitec::interface::TRegistrar<precitec::interface::MsgProxy> *m_registrar = nullptr;
    /// der schickt Befehle an den MM
    precitec::interface::TReceptor<precitec::interface::MsgProxy>  *m_receptor = nullptr;
    /// Liste aller exportierten Interfaces
    std::vector<interface::AnalyzerEntry> mHandlerList_;
    precitec::interface::EventHandlerList eHandlerList_;
    /// Liste aller importierten Interfaces
    std::vector<interface::ProxyEntry> proxyList_;
    /// der fuehrt die Befehle des MM
    precitec::interface::TModule<precitec::interface::MsgServer> moduleServer_;
    /// der hoert auf die Befehle des MM und ruft metuldServer_ auf
    precitec::interface::TModule<precitec::interface::MsgHandler> moduleHandler_;
    /// die appId identifiziert das Modul eindeutig, aber es kann mehrfach im System vorhanden sein
    precitec::system::module::Modules modId_;

};

}
}
}
