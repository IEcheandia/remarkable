#ifndef CALIBRATIONCOORDINATESREQUEST_H_
#define CALIBRATIONCOORDINATESREQUEST_H_

#include "math/CalibrationParamMap.h"
#include "math/calibration3DCoords.h" 
#include "message/device.h"

namespace precitec {
namespace interface {

using math::Calibration3DCoords;
using math::CalibrationParamMap;
using math::Vec3D;
using math::SensorId;
using filter::LaserLine;
using interface::ScannerContextInfo;

typedef std::uint16_t  ScreenCooordinate_t;
enum OCT_Mode
{
    eScan1D, eScan2D
};

} // namespace interface
} // namespace precitec

#endif /* CALIBRATIONCOORDINATESREQUEST_H_ */
