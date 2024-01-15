#include <cstdlib>
#include <iostream>
#include "AppMain.h"


using precitec::ethercat::AppMain;


int main(int argc, char *argv[]) {

	Poco::AutoPtr<AppMain> pApp = new AppMain;
	pApp->processCommandLineArguments(argc, argv);
	pApp->run();





	return EXIT_SUCCESS;
}
