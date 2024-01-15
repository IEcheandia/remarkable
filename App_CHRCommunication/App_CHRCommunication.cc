#include "AppMain.h"

using precitec::grabber::AppMain;

int main(int argc, char *argv[]) {

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

