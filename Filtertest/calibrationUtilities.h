#ifndef CALIBRATIONUTILITIES_H
#define CALIBRATIONUTILITIES_H

#include "util/calibDataSingleton.h"
#include "math/calibrationCommon.h"
#include "common/calibrationConfiguration.h"
#include "system/tools.h"
#include "util/camGridData.h"
#include "math/calibration3DCoords.h"
#include "common/bitmap.h"
#include <sstream>

namespace precitec
{
namespace calibration
{

    using precitec::system::CalibDataSingleton;
    using precitec::system::CamGridData;
    using precitec::math::SensorModel;
    using precitec::math::SensorId;
    using precitec::math::Calibration3DCoords;
    using precitec::system::wmBaseDir;
    using precitec::coordinates::CalibrationConfiguration;
    
    void initializeCalibData(precitec::math::SensorId oSensorID, std::string calibration_override_wm_dir, bool canWriteToDisk=true)
    {
        
        if (canWriteToDisk)
        {
            //make sure that there are configuration folders inth wm_folder
            for ( std::string folder : { "config", "calib"} )
            {
                Poco::File	oDestDir( calibration_override_wm_dir+"/"+folder );
                if ( oDestDir.exists() == false )
                {
                    oDestDir.createDirectories();
                    std::ostringstream oMsg;
                    oMsg << __FUNCTION__ << ": Directory created:" << "\n";
                    oMsg << "'" << oDestDir.path() << "'\n";
                    precitec::wmLog( precitec::eDebug, oMsg.str() );
                } // if
            }
        }
        
        std::cout << calibration_override_wm_dir << std::endl;

        auto & rCalibData = CalibDataSingleton::getCalibrationDataReference(oSensorID);
        rCalibData.resetConfig(canWriteToDisk, calibration_override_wm_dir);
        rCalibData.loadFromConfigFolder(/*recomputeFromCamGridData*/ true, /*useOnlyBinaryCache*/ false);
        
        assert(rCalibData.getSensorModel() != SensorModel::eUndefined);
        assert(rCalibData.hasData());
        
    }

    bool initializeFromGridData(precitec::math::SensorId oSensorID, const std::string & p_oCamGridImageFilename)
    {
        if ( p_oCamGridImageFilename == "" )
        {
            return false;
        }

        auto & rCalibData = CalibDataSingleton::getCalibrationDataReference(oSensorID);

        //rCalibData.setSensorModel(SensorModel::eCalibrationGrid);
        CamGridData oCamGridData;
        bool hasGridImage = true;

        std::string oMsgError = oCamGridData.loadFromCSV(p_oCamGridImageFilename);
        if ( !oMsgError.empty() )
        {
            hasGridImage = false;
        }
        
        if ( !hasGridImage )
        {
            return false;
        }

        std::ofstream info(CalibrationConfiguration::toWMCalibFile("info.txt", wmBaseDir()));
        oCamGridData.show(info);

        rCalibData.initConfig( SensorModel::eCalibrationGridOnLaserPlane, /*createDefault*/ true);
        bool oAns = rCalibData.load3DFieldFromCamGridData(oCamGridData);


        if ( oAns )
        {
            poco_assert_dbg(rCalibData.getSensorModel() == SensorModel::eCalibrationGridOnLaserPlane);
        }
        else
        {
            std::cout << "Calibration data file " << p_oCamGridImageFilename << " can't be loaded as a valuid calibration grid" << std::endl;
            return false;
        }
        return hasGridImage;
    }

    bool processCamGridData(const std::string & p_CamGridFilename, precitec::math::SensorId oSensorID, int argc, char * argv[])
    {
        std::ofstream info(CalibrationConfiguration::toWMCalibFile("info.txt", wmBaseDir()));
		{
			//debug output
			for ( int i = 0; i < argc; i++ )
			{
				info << argv[i] << " ";
			}
			info << "\n";
		}
		bool ok = precitec::calibration::initializeFromGridData(oSensorID, p_CamGridFilename);
        return ok;

    }
    
    void testEquivalentCheckerboard (float oCheckerboardSize, precitec::math::SensorId oSensorID)
    {
        std::cout << "testEquivalentCheckerboard" << std::endl;
        auto &rCalibData(CalibDataSingleton::getCalibrationData(oSensorID));

		using precitec::image::BImage;
		using precitec::geo2d::Point;
		using precitec::math::angleUnit;
		using precitec::filter::LaserLine;
		using checkPlane = Calibration3DCoords::CoordinatePlaneMode;

		std::ofstream info(CalibrationConfiguration::toWMCalibFile("info.txt", wmBaseDir()));

		const auto & oSensorSize = rCalibData.getSensorSize();
		info << "Corner Grid" << "\n";
		
		auto oDiscontinuityPoints = rCalibData.getCalibrationCoords().checkDiscontinuities();
		if ( oDiscontinuityPoints.size() > 0)
		{
			std::cout << "Discontinuities found " << std::endl;
		}
		BImage oCheckerboard;
		

        std::vector<checkPlane> checkPlanes = rCalibData.getSensorModel() == SensorModel::eLinearMagnification ? 
                std::vector<checkPlane> {checkPlane::LineLaser1Plane, checkPlane::LineLaser2Plane, checkPlane::LineLaser3Plane, checkPlane::XYPlane} : 
                std::vector<checkPlane> {checkPlane::LineLaser2Plane};
            
		for ( checkPlane oPlane : checkPlanes )
		{
			filter::LaserLine oLaserLine;
			std::string oDescription;
			float oAngleDeg(0);
			switch ( oPlane )
			{
				case checkPlane::LineLaser1Plane:
					oLaserLine = filter::LaserLine::FrontLaserLine;
					oDescription = laserLineName(oLaserLine);
					oAngleDeg = rCalibData.getCalibrationCoords().getTriangulationAngle(angleUnit::eDegrees, oLaserLine);
					break;
				case checkPlane::LineLaser2Plane:
					oLaserLine = filter::LaserLine::BehindLaserLine;
					oDescription = laserLineName(oLaserLine);
					oAngleDeg = rCalibData.getCalibrationCoords().getTriangulationAngle(angleUnit::eDegrees, oLaserLine);
					break;
				case checkPlane::LineLaser3Plane:
					oLaserLine = filter::LaserLine::CenterLaserLine;
					oDescription = laserLineName(oLaserLine);
					oAngleDeg = rCalibData.getCalibrationCoords().getTriangulationAngle(angleUnit::eDegrees, oLaserLine);
					break;
				case checkPlane::XYPlane:
					oDescription = "Horizontal";
					oAngleDeg = 0;
					break;
				default:
					break;
			}

			std::ostringstream oFilename;
			oFilename << CalibrationConfiguration::toWMCalibFile("", wmBaseDir()) << "CoordinateGrid" << oDescription << "_" << std::setprecision(2) << oCheckerboardSize << ".bmp";

			std::cout << "Exporting equivalent checkerboard at angle " << oAngleDeg << " with side " << oCheckerboardSize << " mm to " << oFilename.str() << std::endl;
				info << "Saved " << oFilename.str() << " angle=" << oAngleDeg << "\n";

			rCalibData.getCalibrationCoords().coordsToCheckerBoardImage(oCheckerboard,
				Point(0, 0), oSensorSize, //test the whole sensor size
				oCheckerboardSize, oPlane);

			fileio::Bitmap oBitmap(oFilename.str().c_str(), oCheckerboard.width(), oCheckerboard.height(), false); //default value of topdown gives mirrored images
			bool ok = oBitmap.isValid() && oBitmap.save(oCheckerboard.data());
			if ( !ok )
			{
				std::cout << "Error for " << oFilename.str() << std::endl;
			}	
		}
    }
    
     void testCoordinateField ( precitec::math::SensorId oSensorID)
    {
        auto &rCalibData(CalibDataSingleton::getCalibrationData(oSensorID));

		using precitec::image::BImage;
		using precitec::geo2d::Point;
		using precitec::math::angleUnit;
		using precitec::filter::LaserLine;
		

		std::ofstream info(CalibrationConfiguration::toWMCalibFile("info.txt", wmBaseDir()));

		const auto & oSensorSize = rCalibData.getSensorSize();
		info << "Corner Grid" << "\n";
		
		
		BImage oField;
		

		for ( int coordType : {0,1} )
		{
			std::string oDescription;
			
			switch ( coordType )
			{
				case 0:
					oDescription = "MagnificationX";
               		rCalibData.getCalibrationCoords().coordsToMagnificationX(oField,
                        Point(0, 0), oSensorSize, //test the whole sensor size
                        1);

					break;
				case 1:
					oDescription = "MagnificationY";
               		rCalibData.getCalibrationCoords().coordsToMagnificationY(oField,
                        Point(0, 0), oSensorSize, //test the whole sensor size
                        1);
					break;

				default:
                    oDescription = "Unsupported";
					break;
			}

			std::ostringstream oFilename;
			oFilename << CalibrationConfiguration::toWMCalibFile("", wmBaseDir()) << "CoordinateGrid" << oDescription  << ".bmp";

	
			fileio::Bitmap oBitmap(oFilename.str().c_str(), oField.width(), oField.height(), false); //default value of topdown gives mirrored images
			bool ok = oBitmap.isValid() && oBitmap.save(oField.data());
			if ( !ok )
			{
				std::cout << "Error for " << oFilename.str() << std::endl;
			}	
		}
    }
    
}
}


#endif
