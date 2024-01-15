#ifndef CALIBRATION_COMMON_H
#define CALIBRATION_COMMON_H


#ifndef NDEBUG
//#define extradebugcalib
#endif

#include <string>

namespace precitec
{
namespace math
{


//see also SystemConfiguration Type_of_Sensor in camToCalibData
enum class SensorModel
{
    eUndefined =-1,
	eLinearMagnification=0, //uses beta0, betaZ (supports multiple triangulation angle)
    eCalibrationGridOnLaserPlane=1, //precomputed grid on the laser plane (only 1 laser line)
	eNumSensorModels = 2,
    eSensorModelMin=eUndefined,
    eSensorModelMax = eCalibrationGridOnLaserPlane
};

inline std::string SensorModelDescription(SensorModel s )
{
    switch (s)
    {
        case SensorModel::eUndefined:
            return "Undefined";
        case SensorModel::eLinearMagnification:
            return "Linear magnification (coax)";
        case SensorModel::eCalibrationGridOnLaserPlane:
            return "Calibration grid (Scheimpflug)";
        default:
            return "?(" + std::to_string(int(s)) + ")";
    }
}



enum SensorId{
	eSensorIdWildcard = -1,
	eSensorId0=0,
	eSensorId1,
	eSensorId2,
	eNumSupportedSensor, //see also  CalibData.MaxSensors in Precitec.Common/ServiceEntities/CalibData.cs
	eSensorIdDefault = eSensorId0, //used to convert eSensorIdWildcard
	eInvalidSensor = -100  //negative number as expected by validSensorID
};



}
}
#endif
