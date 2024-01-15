#include <iostream>
#include <unistd.h>
#include "AppMain.h"
#include "event/inspection.proxy.h"
#include "message/module.h"
#include "message/device.h"
#include "module/moduleLogger.h"
#include "common/triggerContext.h"
#include "common/connectionConfiguration.h"
#include "common/systemConfiguration.h"
#include "system/realTimeSupport.h"


namespace precitec{
namespace grabber{

AppMain::AppMain() :
    BaseModule(system::module::GrabberModul)
    , m_triggerServer(m_sensorProxy)
    , m_triggerHandler(&m_triggerServer)
    , m_triggerCmdServer(m_triggerServer)
    , m_triggerCmdHandler(&m_triggerCmdServer)
    , m_deviceServer(m_triggerCmdServer) 
    , m_deviceHandler(&m_deviceServer)
    , m_grabberStatusServer(m_triggerServer)
    , m_grabberStatusHandler(&m_grabberStatusServer)
{
    char* oEnvStrg = getenv((char*)"WM_STATION_NAME");
    if (oEnvStrg != nullptr)
    {
        if (strcmp("WM-QNX-PC", oEnvStrg) == 0)
        {
            m_hasCamera = SystemConfiguration::instance().getBool("HasCamera", true);
        }
        else
        {
            m_hasCamera = false;
        }
    }
    else
    {
        m_hasCamera = false;
    }
}


AppMain::~AppMain() 
{
    uninitialize();
}


void AppMain::runClientCode()
{
    if (!m_hasCamera)
    {
        m_triggerServer.loadImages();
    }
    notifyStartupFinished();
}


int AppMain::init(int argc, char * argv[])
{
    processCommandLineArguments(argc, argv);

    if(!m_hasCamera)
    {
        ConnectionConfiguration::instance().setInt(pidKeys[GRABBER_KEY_INDEX], getpid());
        registerPublication(&m_sensorProxy);
        registerSubscription(&m_deviceHandler, system::module::GrabberModul);
        registerSubscription(&m_triggerHandler);
        registerSubscription(&m_triggerCmdHandler);
        registerSubscription(&m_grabberStatusHandler);
    }
    initialize(this);
    return 0;
}

void AppMain::uninitialize() 
{
    m_deviceServer.uninitialize();
    m_triggerServer.uninit();
}
}
}
