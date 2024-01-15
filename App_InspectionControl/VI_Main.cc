#include "AppMain.h"

int main(int argc, char *argv[]) {

	Poco::AutoPtr<precitec::viInspectionControl::AppMain> pApp = new precitec::viInspectionControl::AppMain;

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
