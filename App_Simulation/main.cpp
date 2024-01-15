#include "AppMain.h"
#include <unistd.h>

// code taken from POCO_APP_MAIN with an adjusted nice
int main(int argc, char** argv)
{
    // lower priority than hardware Weldmaster processes
    nice(1);

    Poco::AutoPtr<precitec::Simulation::AppMain> pApp = new precitec::Simulation::AppMain;
    try
    {
        pApp->init(argc, argv);
    }
    catch (Poco::Exception& exc)
    {
        pApp->logger().log(exc);
        return Poco::Util::Application::EXIT_CONFIG;
    }
    return pApp->run();
}
