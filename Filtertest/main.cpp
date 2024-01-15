/*!
 *  \n Copyright:	Precitec Vision GmbH & Co. KG
 *  \n Project:		WM Filtertest
 *  \author			Simon Hilsenbeck (HS), Stefan Birmanns (SB)
 *  \date			2010-2011
 *  \file			main.cpp
 */


// stl includes
#include <string>
#include <iostream>
#include <fstream>
#include <stdexcept>

// Poco includes
#include <Poco/Path.h>
#include <Poco/File.h>
#include <Poco/Exception.h>
#include <Poco/Environment.h>
// fliplib includes
#include <fliplib/Exception.h>
#include <fliplib/FilterLibrary.h>
// local includes
#include "graphManager.h"

#include "filtertestParameters.h"
#include "calibrationUtilities.h"

#ifdef CALLGRIND_PROFILE
#include <valgrind/callgrind.h>
#endif

struct ScopedLogger {};

#ifdef HAVE_QT
#include <QGuiApplication>
#endif

#ifdef HAS_GPERFTOOLS
#include "gperftools/profiler.h"
#endif

using precitec::filter::GraphManager;
using precitec::system::logExcpetion;


int main(int argc, char * argv[])
{
    UNUSED ScopedLogger oWinLogger;

	// default parameters
    FilterTestParameters  parameters;
    bool validParameters = parameters.parse(argc, argv);
	if (!validParameters)
	{
		std::cout << "invalid arguments" << std::endl;
		parameters.printUsage();
		return EXIT_FAILURE;
	} 

	
#ifdef HAVE_QT
	QGuiApplication app(argc, argv);
#endif


	const precitec::math::SensorId oSensorID(precitec::math::SensorId::eSensorId0);


	if (parameters.mCamGridImageFilename.empty())
    {
        //in wm this is done by inspectmanager
        //compare InspectManager::loadCalibDataAfterSignal
        //TODO: handle eScheimpflug

        std::cout << "initializeCalibData" << std::endl;
        precitec::calibration::initializeCalibData(oSensorID, parameters.mCalibrationOverrideWM_BASE_DIR );

        
        std::cout << "Loaded Calibration " << std::endl;
        
        auto &rCalibData( precitec::CalibDataSingleton::getCalibrationData(oSensorID));
        auto oCoaxCalib = rCalibData.getCoaxCalibrationData();
        auto oTCP = rCalibData.getTCPCoordinate(precitec::interface::ScannerContextInfo{}, precitec::filter::LaserLine::FrontLaserLine);
        
        std::cout << "Beta " << oCoaxCalib.m_oBeta0 << " " << oCoaxCalib.m_oBetaZ << " TCP " << oTCP.x << " " << oTCP.y << std::endl;
        if (oCoaxCalib.m_oBeta0== 0.5 && oCoaxCalib.m_oBetaZ == 0.5)
        {
            std::cout << "WARNING: using default calibration values \n" << std::endl;
        };
    }
    else
    {
        // explicitly provided a scheimpflug calibration file
        //compute internal coordinates and export debug information
        //camgriddata if the "scheimpflug fallback file"
        if ( !parameters.mCamGridImageFilename.empty() )
        {
            if (! precitec::calibration::processCamGridData(parameters.mCamGridImageFilename, oSensorID, argc, argv))
            {
                std::cout << "Calibration data file " << parameters.mCamGridImageFilename << " can't be loaded as a valid calibration grid" << std::endl;
                return EXIT_FAILURE;
            }	
        } //end oCamGridDataFilename

    }

    
	if (parameters.mCheckerboardSize > 0 )
	{
        precitec::calibration::testEquivalentCheckerboard (parameters.mCheckerboardSize, oSensorID);
        precitec::calibration::testCoordinateField(oSensorID);
        
        if (parameters.mXML_Filename.empty() || parameters.mBmpPath.empty())
        {
            return EXIT_SUCCESS;
        }
	}//end test checkerboard

    if (parameters.mXML_Filename.empty())
    {
        return EXIT_FAILURE;
    }
	
	try
	{
		GraphManager TestGraphManager( parameters.mXML_Filename, parameters.mBmpPath,67000,335, 
                                       { {precitec::interface::Sensor::eScannerXPosition, 1000},
                                        {precitec::interface::Sensor::eScannerYPosition, 2000}},  
                                 parameters.mHasCanvas, parameters.mResultFolder );
        TestGraphManager.redirectLogMessages(parameters.mRedirectLogMessages);
        TestGraphManager.printAllTimings(parameters.mPrintAllTimings);
        TestGraphManager.setPauseAfterEachImage(parameters.m_pauseAfterEachImage);

        if (parameters.mNumberOfImages > 0)
        {
            TestGraphManager.setNumImages(parameters.mNumberOfImages);
        }
        TestGraphManager.setArmAtSequenceRepetition(parameters.mArmAtSequenceRepetition);
		TestGraphManager.setParameter();


#ifdef CALLGRIND_PROFILE
		CALLGRIND_START_INSTRUMENTATION;
#endif

        
#ifdef HAS_GPERFTOOLS        
        int profilerStarted = 0;
        if (!parameters.mOutputGprof.empty()) 
        {
            profilerStarted = ProfilerStart(parameters.mOutputGprof.c_str());
        }
#endif


    

		TestGraphManager.fire( );
        
#ifdef HAS_GPERFTOOLS        
        if (profilerStarted != 0)
        {
            ProfilerStop();
        }
#endif

#ifdef CALLGRIND_PROFILE
	CALLGRIND_STOP_INSTRUMENTATION;
	CALLGRIND_DUMP_STATS;
#endif

	}
	catch (...) {
		logExcpetion(__FUNCTION__, std::current_exception());
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;

}
 
