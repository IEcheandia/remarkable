#include "module/moduleManagerConnector.h"
#include "common/connectionConfiguration.h"

#include <Poco/Process.h>

using namespace precitec::interface;
using precitec::system::message::SmpProtocolInfo;
using precitec::system::message::SocketInfo;

namespace precitec
{
namespace framework
{
namespace module
{

ModuleManagerConnector::ModuleManagerConnector(Modules moduleId)
    : moduleServer_(mHandlerList_, eHandlerList_, proxyList_)
    , moduleHandler_(&moduleServer_)
    , modId_(moduleId)
{
}

ModuleManagerConnector::~ModuleManagerConnector() = default;

SmpProtocolInfo ModuleManagerConnector::readConfig()
{
    return SmpProtocolInfo();
}

TReceptor<MsgProxy> &ModuleManagerConnector::receptor() const
{
    return *m_receptor;
}

TRegistrar<MsgProxy> &ModuleManagerConnector::registrar() const
{
    return *m_registrar;
}

Modules ModuleManagerConnector::getMyAppId() const
{
    return modId_;
}

/**
 * Der Receptor_Proxy wird gestartet
 * Das ist einfach wenn der MM bereits laeuft (die Verbindung laeuft ueber ein Std-Interface=
 * Der Registrar ist Server, kann also verschiedene Module ueber eine Schnitstelle bedienen,
 * da er nur antwortet.
 * Falls der MM noch nicht existiert, muss er gestartet werden. Dann muss man warten bis er
 * bereit ist. Die Synchronisation geschieht ueber Named Mutexe
 */
void ModuleManagerConnector::startReceptor(ConnectionConfiguration &config)
{
    // der Receptor erledigt nur minimale Aufgaben, blockiert nie und
    // kann es sich daher erlauben, auf einem Protokoll/Port alle Module zu bedienen
    // dieses PRotokoll ist hartcodiert und via createReceptorProtocol() erhaeltlich.
    SmpProtocolInfo receptorInfo(new SocketInfo(system::message::Udp, "127.0.0.1", config.getString("Receptor.Port", "49900")));
    m_receptor = new TReceptor<MsgProxy>(receptorInfo);
    //std::cout << "Receptor: " << receptor() << std::endl;
    //std::cout << "BaseModule::establish_registrar is running" << std::endl;
}

/**
 * Gehoert nicht zum Message-Interface, wird von Subsystem verwendet
 * Wird nur aufgerufen, wenn Module nicht vom MM gestartet wird
 * Baut die Verbidung zu MM-Registrar auf
 * startet den Module-Server
 * ueber initModule wird dann der gemeinsame Teil der Initialisierung aufgerufen
 * danach laufen alle Server!
 */
void ModuleManagerConnector::startRegistrar(SmpProtocolInfo &moduleServerProtocolInfo)
{
    if (moduleServerProtocolInfo.isNull()) {
        // wir stellen ueber den Registrar-Proxy die Verbindung zum Module-Manager her
        // module wird getrennt von den anderen Servern initialisiert, da er allen Modulen
        // gemeinsam ist und daher hier in der Basisklasse erledigt werden kann
        //std::cout << "BaseModule::startModule trying to register Module" << std::endl;
        //interface::TwoProtocols infos;
        moduleServerProtocolInfo = receptor().registerModule(getModuleHandle(), getMyModuleSpec(), interface::ProcessModule);

        if ( moduleServerProtocolInfo.isNull() ) {
            wmLog( eError, "BaseModule::startModule Anmeldung an Registrar fehlgeschlagen.\n");
            return;
        }

    } else {
        // der else-Zweig tritt ein, wenn der MM-das Modul startet
        // dieser Zweig ist schon lange nicht mehr getrstet worden!!! ??????
        // tun sollte er eigentlich nichts, aber inzwischen hat sich einiges geaendert:
        //  der MM muss von sich aus erst die Protokolle erzeugen und dann das Modul laden!!!!
    }
    // jetzt laeuft die Verbindung zum MM bereits

    // das zurueckgegebene Protokoll ist erstmal fuer den Modul-Server
    // aber direkt daraus abgeleitet wird das Protokoll fuer den Registrar-Client
    SocketInfo registrarInfo(*dynamic_cast<SocketInfo const*>(moduleServerProtocolInfo.get()));
    // ... und diese Ableitung erfolgt hier (analog im Registrar). Das ist natuerlich ein temp. Hack!!!
    SmpProtocolInfo protInfo(registrarInfo.generateNextDerived());
    m_registrar = new TRegistrar<MsgProxy>(protInfo);
    //std::cout << "Registrar: " << registrar() << std::endl;

    // mit dem Protokoll vom Registar wird der moduleHandler gestartet
    // dies geht (wg Reihenfolge) im QNX-Protokoll nicht, daher ist Udp hardkodiert
    moduleHandler_.activate(moduleServerProtocolInfo);

    // die Server werden erst auf Initiative des MM gestartet, da nur der die noetigen Protokolle
    // ermitteln kann
}

int ModuleManagerConnector::getModuleHandle() const
{
    return Poco::Process::id();
}

void ModuleManagerConnector::initModuleManager(ConnectionConfiguration &config)
{
    //static bool firstTime(true);
    //std::cout << "BaseModule::initialize " << ModuleName[getMyAppId()] << (firstTime ? "first time" : "must not be called twice!!!!!!!") <<std::endl;
    //initSubsystems();
    //loadConfiguration(); // load default configuration files, if present

    SmpProtocolInfo moduleServerProtocolInfo = readConfig();

    // MM starten, und den ModuleHanlder natuerlich auch, dann kann
    // schamlos a Schwaetzle zwischen den Beiden gehalten werden

    // der Registrar-Proxy wird initialisiert; ModuleManager wird zur Not gestartet
    startReceptor(config);
    startRegistrar(moduleServerProtocolInfo);

    // alle Interfaces werden einzeln gepublished und die Server gestartet
    // diese Funktion muss natuerlich von den abgeleiteten Klassen ueberschrieben werden

    //std::cout << "BaseModule::initialize end" << std::endl;
}

/**
 * After all interfaces have been 'collected' by the publish/subscribe/export/import-functions,
 * these Lists are now worked off and individually sent to the ModuleManager
 */
bool ModuleManagerConnector::publishAllInterfaces()
{
    //std::cout << "publishAllInterfaces" << std::endl;

    // notShuttingDown bleibt normalerweise true, aber wenn ein Shutdown erfolgt,
    // waehrend ein Interface noch im MM haengt, wird das flag auf false gesetzt
    bool notShuttingDown = true;
    // erst die Events, da die nicht blockieren
    for (auto proxy=proxyList_.begin(); notShuttingDown && (proxy!=proxyList_.end()); ++proxy) {
        if (proxy->isEventProxy()) {
            //std::cout << "publishAllInterfaces<" << ModuleName[getMyAppId()]<< ">: publishing " << InterfaceName[interfaceId] << std::endl;
            const ProxyEntry &proxEntry(*proxy);
            notShuttingDown &= registrar().publish(getModuleHandle(), proxy->eventProxy().info_.interfaceId, proxEntry.numMessages(),
                                                                                        getMyAppId(), proxEntry.modId, proxEntry.path);
            //std::cout << "publishAllInterfaces: published " << InterfaceName[interfaceId] << " ('" << system::module::InterfaceNames()[interfaceId] << "')" << std::endl;
        }
    }
    // dann die Messages, da die nicht blockieren
    for (auto proxy=proxyList_.begin(); notShuttingDown && (proxy!=proxyList_.end()); ++proxy) {
        if (proxy->isMessageProxy()) {
            //std::cout << "publishAllInterfaces<" << ModuleName[getMyAppId()]<< ">: publishing " << InterfaceName[interfaceId] << std::endl;
            const ProxyEntry &proxEntry(*proxy);
            notShuttingDown &= registrar().publish(getModuleHandle(), proxy->msgProxy().info_.interfaceId, proxEntry.numMessages(),
                                                                                        getMyAppId(),  proxEntry.modId, proxEntry.path);
            //std::cout << "publishAllInterfaces: published " << InterfaceName[interfaceId] << " ('" << system::module::InterfaceNames()[interfaceId] << "')" << std::endl;
        }
    }
    //std::cout << "publishAllInterfaces: ok" << std::endl;
    return true; //notShuttingDown;
}

/**
 * After all interfaces have been 'collected' by the publish/subscribe/export/import-functions,
 * these Lists are now worked off and individually sent to the ModuleManager
 */
void ModuleManagerConnector::subscribeAllInterfaces()
{
    //std::cout << "subscribeAllInterfaces: messages" << std::endl;
    for (auto handler=mHandlerList_.begin(); handler!=mHandlerList_.end(); ++handler) {
        int interfaceId = handler->handler->info_.interfaceId;
        AnalyzerEntry &msgEntry(*handler);
        //std::cout << "subscribeAllInterfaces<" << ModuleName[getMyAppId()]<< ">: subscribing msg " << InterfaceName[interfaceId] << std::endl;
        registrar().subscribe(getModuleHandle(), interfaceId, msgEntry.handler->info_.numMessages, getMyAppId(), system::module::AnyModule, msgEntry.path);
        //std::cout << "subscribeAllInterfaces: subscribed msg " << InterfaceName[interfaceId] << std::endl;
    }
    //std::cout << "subscribeAllInterfaces: events" << std::endl;
    for (EventHandlerList::iterator handler=eHandlerList_.begin(); handler!=eHandlerList_.end(); ++handler) {
        int interfaceId = handler->first;
        EventServerEntry &evntEntry(handler->second);
        //std::cout << "subscribeAllInterfaces<" << ModuleName[getMyAppId()] << ">: subscribing event " << InterfaceName[interfaceId] << std::endl;
        registrar().autoSubscribe(getModuleHandle(), interfaceId, evntEntry.handler->info_.numMessages, getMyAppId());
        //std::cout << "subscribeAllInterfaces: subscribed event " << InterfaceName[interfaceId] << std::endl;
    }
    //std::cout << "subscribeAllInterfaces: ok" << std::endl;
}

/// muss vor startModule aufgerufen werden, macht ein bereitgestelltes Interface bekannt
void ModuleManagerConnector::registerSubscription(PMHandler handler, Modules appId, PvString const& path) {
    //std::cout << "registerSubscription " << ModuleName[appId] << " " << InterfaceName[handler->info_.interfaceId] << std::endl;
if (!isMessageInterface(Interfaces(handler->info_.interfaceId))) throw Poco::Exception("EventInterface subscribed as Message");
    mHandlerList_.push_back(AnalyzerEntry(handler, appId, path));
    //std::cout << "registerSubscription " << ModuleName[appId] << " " << InterfaceName[handler->info_.interfaceId] << "   ok" << std::endl;
}

/// muss vor startModule aufgerufen werden, macht ein bereitgestelltes Interface bekannt
void ModuleManagerConnector::registerSubscription(PEHandler handler, Modules appId, PvString const& path) {
    //std::cout << "registerSubscription " << ModuleName[appId] << " " << InterfaceName[handler->info_.interfaceId] << std::endl;
    if (!isEventInterface(Interfaces(handler->info_.interfaceId))) throw Poco::Exception("MessageInterface subscribed as Event");
    eHandlerList_[handler->info_.interfaceId] = EventServerEntry(handler, appId, path);
    //std::cout << "registerSubscription " << ModuleName[appId] << " " << InterfaceName[handler->info_.interfaceId] << "   ok" << std::endl;
}

/// muss vor startModule aufgerufen werden, macht ein verwendetes Interface bekannt
void ModuleManagerConnector::registerPublication(PMProxy proxy, Modules appId, PvString const& path) {
    //std::cout << "registerPublication " << ModuleName[appId] << " " << precitec::system::module::InterfaceName[proxy->info_.interfaceId] << std::endl;
    if (!isMessageInterface(Interfaces(proxy->info_.interfaceId))) throw Poco::Exception("MessageInterface subscribed as Message");
    proxyList_.push_back(ProxyEntry(proxy, appId, path));
    //std::cout << "registerPublication " << ModuleName[appId] << " " << precitec::system::module::InterfaceName[proxy->info_.interfaceId] << " ok "<< std::endl;
}

/// muss vor startModule aufgerufen werden, macht ein verwendetes Interface bekannt
void ModuleManagerConnector::registerPublication(PEProxy proxy, Modules appId, PvString const& path) {
    // std::cout << "registerPublication " << ModuleName[appId] << " " << InterfaceName[proxy->info_.interfaceId] << std::endl;
    if (!isEventInterface(Interfaces(proxy->info_.interfaceId))) throw Poco::Exception("MessageInterface subscribed as Event");
    proxyList_.push_back(ProxyEntry(proxy, appId, path));
    //listProxies(std::cout);
    //std::cout << "registerPublication " << ModuleName[appId] << " " << InterfaceName[proxy->info_.interfaceId] << " ok "<< std::endl;
}

/// alternatives Interface
/// muss vor startModule aufgerufen werden, macht ein bereitgestelltes Interface bekannt == registerSubscription
void ModuleManagerConnector::exportInterface(PMHandler handler, Modules appId, PvString const& path) {
    if (!isMessageInterface(Interfaces(handler->info_.interfaceId))) throw Poco::Exception("EventInterface subscribed as Message");
    mHandlerList_.push_back(AnalyzerEntry(handler, appId, path));
}

/// muss vor startModule aufgerufen werden, macht ein bereitgestelltes Interface bekannt == registerSubscription
void ModuleManagerConnector::exportInterface(PEHandler handler, Modules appId, PvString const& path) {
    if (!isEventInterface(Interfaces(handler->info_.interfaceId))) throw Poco::Exception("MessageInterface subscribed as Event");
    eHandlerList_[handler->info_.interfaceId] = EventServerEntry(handler, appId, path);
}

/// muss vor startModule aufgerufen werden, macht ein verwendetes Interface bekannt == registerPublication
void ModuleManagerConnector::importInterface(PMProxy proxy, Modules appId, PvString const& path) {
    if (!isMessageInterface(Interfaces(proxy->info_.interfaceId))) throw Poco::Exception("EventInterface subscribed as Message");
    proxyList_.push_back(ProxyEntry(proxy, appId, path));
}

/// muss vor startModule aufgerufen werden, macht ein verwendetes Interface bekannt == registerPublication
void ModuleManagerConnector::importInterface(PEProxy proxy, Modules appId, PvString const& path) {
    if (!isEventInterface(Interfaces(proxy->info_.interfaceId))) throw Poco::Exception("MessageInterface subscribed as Event");
    proxyList_.push_back(ProxyEntry(proxy, appId, path));
}

}
}
}
