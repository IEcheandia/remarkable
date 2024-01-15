/**
 * 	@file
 * 	@copyright	Precitec Vision GmbH & Co. KG
 * 	@author		Fabian Agrawal (AL), Stefan Birmanns (SB)
 * 	@date		2011
 * 	@brief		Homing of the weld-head axis.
 */

#include <calibration/calibrationManager.h>
#include <calibration/homing.h>

#include "common/systemConfiguration.h"

namespace precitec {
	using namespace interface;
namespace calibration {


bool Homing::calibrate(interface::CalibType p_oCalibType)
{
	try
	{
		if (p_oCalibType == eHomingY)
		{
			// Homing of Y-Axis
			bool oAxisYEnable = SystemConfiguration::instance().getBool("AxisYEnable", false);
			if (oAxisYEnable)
			{
				wmLog( eDebug, "Homing y axis ...\n" );
				m_rCalibrationManager.setHeadMode( eAxisY, Position, true );
			} else {
				wmLog( eDebug, "Homing: No y axis configured in SystemConfig.xml.\n" );
			}
		}
		if (p_oCalibType == eHomingZ)
		{
			// Homing of Z-Axis
			bool oAxisZEnable = SystemConfiguration::instance().getBool("AxisZEnable", false);
			if (oAxisZEnable)
			{
				wmLog( eDebug, "Homing z axis ...\n" );
				m_rCalibrationManager.setHeadMode( eAxisZ, Position, true );
			} else {
				wmLog( eDebug, "Homing: No z axis configured in SystemConfig.xml.\n" );
			}
		}
		if (p_oCalibType == eHomingZColl)
		{
			// Homing of Z-Collimator
			bool oZCollimatorEnable = SystemConfiguration::instance().getBool("ZCollimatorEnable", false);
			if (oZCollimatorEnable)
			{
				wmLog( eDebug, "Homing z collimator ...\n" );
				m_rCalibrationManager.doZCollHoming();
			} else {
				wmLog( eDebug, "Homing: No z collimator configured in SystemConfig.xml.\n" );
			}
		}
	}
	catch(...)
	{
		//Error while homing (-> Homing not finished in time)
		std::cout << "Error while homing (-> Maybe homing not finished in time)." << std::endl;
		return false;
	}

#if !defined(NDEBUG)
	std::cout << "Homing::calibrate() - SetHeadMode successful." << std::endl;
#endif

	return true;

} // calibrate


} // namespace calibration
} // namespace precitec
