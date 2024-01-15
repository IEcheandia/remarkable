/*
 * calibDataSingleton.cpp
 *
 *  Created on: May 22, 2014
 *      Author: abeschorner
 */


#include "util/calibDataSingleton.h"

namespace precitec {

namespace system {

	std::array<math::CalibrationData, math::eNumSupportedSensor> CalibDataSingleton::m_oCalData {{ math::eSensorId0, math::eSensorId1, math::eSensorId2 }};

} // namespace calibration
} // namespace precitec
