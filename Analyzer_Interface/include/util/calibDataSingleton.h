/*
 * calibDataSingleton.h
 *
 *  Created on: May 22, 2014
 *      Author: abeschorner
 */

#ifndef CALIBDATASINGLETON_H_
#define CALIBDATASINGLETON_H_

#include <Analyzer_Interface.h>
#include <string>
#include "math/calibrationData.h"
namespace precitec {

namespace system {

class ANALYZER_INTERFACE_API CalibDataSingleton
{
public:
	
	const static math::CalibrationData & getCalibrationData(math::SensorId oId) 
	{
		return getCalibrationDataReference(oId); 
	}

    //needed by filtertest, InspectManager and SystemStatusServer.createCalibImage, CalibrationManager has its own instance
	static math::CalibrationData &getCalibrationDataReference(math::SensorId oId) 
	{
		//TODO: what to do in case of error?
		assert(math::CalibrationData::validSensorID(oId));
		//assert that the position in the array corresponds to the sensor number
		assert(int(m_oCalData[oId].getSensorId()) ==oId);
		return m_oCalData[oId]; 
	}

    static const math::Calibration3DCoords &getCalibrationCoords(math::SensorId  oId)
    {
        return getCalibrationData(oId).getCalibrationCoords();
    }

    static std::unique_ptr<math::ImageCoordsTo3DCoordsTransformer> getImageCoordsto3DCoordTransformer(math::SensorId  oId, const interface::ImageContext & rContext, filter::LaserLine p_oLaserLine)
    {
        return std::unique_ptr<math::ImageCoordsTo3DCoordsTransformer> (new math::ImageCoordsTo3DCoordsTransformer(getCalibrationCoords(oId), rContext, p_oLaserLine));
    }

    static std::unique_ptr<math::ImageCoordsTo3DCoordsTransformer> getImageCoordsto3DCoordTransformer(math::SensorId  oId, const interface::ImageContext & rContext, filter::MeasurementType p_oMeasurementType)
    {
        if (p_oMeasurementType == filter::MeasurementType::Image)
        {
            return std::unique_ptr<math::ImageCoordsTo3DCoordsTransformer> (new math::ImageCoordsTo3DCoordsTransformer(getCalibrationCoords(oId), rContext));
        }
        auto laserLine = static_cast<filter::LaserLine>(p_oMeasurementType);
        if (laserLine != static_cast<int>(p_oMeasurementType))
        {
            wmLog(eWarning, "Unknown measurementType %d, setting lineLaser1 \n", static_cast<int>(p_oMeasurementType));
            laserLine = filter::LaserLine::FrontLaserLine;
        }
        return getImageCoordsto3DCoordTransformer(oId, rContext, laserLine);
    }

private:
	CalibDataSingleton() {};
	static std::array<math::CalibrationData, math::eNumSupportedSensor> m_oCalData;
};

} // namespace calibration
} // namespace precitec


#endif /* CALIBDATASINGLETON_H_ */
