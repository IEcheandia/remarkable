#include <cstdlib>
#include <iostream>

#include "Poco/DOM/AutoPtr.h"
#include "Poco/Exception.h"
#include "AppMain.h"

#include "system/realTimeSupport.h"

using Poco::XML::AutoPtr;
using Poco::Exception;
using namespace precitec::interface;

#define DEBUG_

using precitec::ethercat::AppMain;

int main(int argc, char *argv[]) {
    precitec::system::raiseRtPrioLimit();
	Poco::AutoPtr<AppMain> pApp = new AppMain;

	try
	{
		pApp->init(argc, argv);
	}
	catch (Poco::Exception& exc)
	{
		pApp->logger().log(exc);
		return Poco::Util::Application::EXIT_CONFIG;
	}

	pApp->run();

	return EXIT_SUCCESS;
}

