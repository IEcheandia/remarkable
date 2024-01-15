/**
 *	@file
 *  @copyright	Precitec Vision GmbH & Co. KG
 *  @author		Simon Hilsenbeck (HS)
 *  @date		2011
 * 	@brief 		VideoRecorder application - controls the application process.
 */
#include "AppMain.h"

#include "Poco/Util/Application.h"
/**
* Hauptroutine fuer die Application
*/
int main(int argc, char** argv) {
	try {
		Poco::AutoPtr<precitec::vdr::AppMain> pApp = new precitec::vdr::AppMain;
		pApp->init(argc, argv);
		return pApp->run();
	}
	catch(const Poco::Exception &p_rException) {
		std::cerr  << __FUNCTION__ << "poco::exception: " << p_rException.what() << "\n\t" << p_rException.message() << std::endl;
		const auto* pNested = p_rException.nested();
		while (pNested != nullptr) {
			std::cerr << "\tNested exception: " << "\n\t\t" << pNested->displayText() << std::endl;
			pNested = pNested->nested();
		}
		std::cerr  << "\nreturning in some seconds...\n"; sleep(10); // TODO LOGGER
		return Poco::Util::Application::EXIT_CONFIG;
	}
	catch(const std::exception &p_rException) {
		std::cerr << __FUNCTION__ << "\t: std::exception: " << p_rException.what() << std::endl;
		std::cerr  << "\nreturning in some seconds...\n"; sleep(10); // TODO LOGGER
		return Poco::Util::Application::EXIT_CONFIG;
	}
	catch (...) {
		std::cout << __FUNCTION__ << "\t: Unhandled exception." << std::endl; // TODO LOGGER
		std::cerr  << "\nreturning in some seconds...\n"; sleep(10);
		return Poco::Util::Application::EXIT_CONFIG;
	} // catch

} // main
