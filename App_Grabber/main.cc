#include <cstdlib>
#include <iostream>


#ifdef UNITEST

	#include "UnitTest/UnitTest++.h"
	#include "UnitTest/TestReporterStdout.h"
	
	int main(int argc, char *argv[]) {
		return UnitTest::RunAllTests();
	}

#else
	#include "AppMain.h"
	/** 
	 * Hauptroutine fuer die Application
	 */
	POCO_APP_MAIN(precitec::grabber::AppMain)

#endif



