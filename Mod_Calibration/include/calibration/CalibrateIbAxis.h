#ifndef CALIBRATEIBAXIS_H
#define CALIBRATEIBAXIS_H

#include <vector>
#include <array>
#include <map>
#include <string>

#include "calibrate.h"

using precitec::math::angleUnit;
using precitec::filter::LaserLine;

namespace precitec {

using namespace interface;

/**
 * @brief In this namespace all the calibration related classes are organized - the CalibrationManager is the main class that controls and executes the calibration procedures.
 */

namespace calibration {

/*
 * @brief Calibrates axis.
 *
 * Calibrate axis after having performed base (and facultatively grid) computations. The axis will move a predefined distance several times and
 * a factor is computed that can be used to get the real number of mm the axis moved. If grid calibration has been performed before, the magnification beta0
 * used for the axis calibration is dependent on the y position. Otherwise, base beta0 will be used for computations.
 */
class CalibrateIbAxis : public CalibrateIb
{
public:
	explicit CalibrateIbAxis(CalibrationManager &p_rCalibMgr); ///< Explicit constructor.
	virtual ~CalibrateIbAxis(); ///< Destructor.

	virtual bool calibrate(); ///< Main routine for graph based axis calibration.
	bool calibrateAxis(double &p_rRatio, math::CalibrationData &p_rCalibData, CalibrationGraph &p_rGraph); ///< Subroutine called by calibrate() to perform axis calibration.

protected:
	CalibrationGraph m_oGraph;    ///< The graph to be executed.

	void drawInfo(const double, const double) {};  ///< No painting yet.
};

} // namespaces
}

#endif /* CALIBRATEIBAXIS_H */
