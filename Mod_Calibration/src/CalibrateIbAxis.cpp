#include "Poco/Thread.h" // for sleep
#include "math/2D/avgAndRegression.h"
#include "math/mathUtils.h"
#include "math/calibrationData.h"
#include "fliplib/BaseFilter.h"
#include <limits>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <memory>
#include <map>
#include <cmath>

#include "common/connectionConfiguration.h"
#include "image/image.h"
#include "common/bitmap.h"
#include "calibration/calibrate.h"
#include "common/systemConfiguration.h"
//#include "util/calibDataParameters.h"
#include "math/3D/projectiveMathStructures.h"
#include "math/mathUtils.h"
#include "math/mathCommon.h"
#include "math/calibrationStructures.h"
#include "calibration/CalibrateIbAxis.h"

namespace precitec {

using namespace interface;
using namespace precitec::math;

using filter::LaserLine;


/**
 * @brief In this namespace all the calibration related classes are organized - the CalibrationManager is the main class that controls and executes the calibration procedures.
 */
namespace calibration {

// =============================================================================
// ============================ axis specific stuff ============================
// =============================================================================


/**
 * This is NOT a calibration of the optical system with an axis but a calibration of the axis itself.
 * IOW if the axis is not moving absolutely straight, this calibration computes a factor that can
 * be applied either to measurements or to movements of the axis to compensate for inaccurate distances.
 */

CalibrateIbAxis::CalibrateIbAxis(CalibrationManager &p_rCalibMgr) : CalibrateIb(p_rCalibMgr), m_oGraph(p_rCalibMgr, "GraphCalibrationNoAxis.xml")
{
}

CalibrateIbAxis::~CalibrateIbAxis()
{
}

bool CalibrateIbAxis::calibrateAxis(double &p_rRatio, CalibrationData &p_rCalibData, CalibrationGraph &p_rGraph)
{
	// todo: rewrite
	p_rRatio = 0.0;
	const int oNumPos = 5; // MUST BE >= 3
	const int oRelPos[oNumPos]={0, 1, 1, -3, 2};  // relative positions in mm

	
	assert(&p_rCalibData ==  &(m_rCalibrationManager.getCalibrationData(0)) && "calibrateAxis, check origin of argument p_rCalibData");
	auto& rCalibrationParameters = p_rCalibData.getParameters();

	double oMmPerStep = rCalibrationParameters.getDouble("mmPerIncrement");

	UNUSED double oXPosLeft[oNumPos], oXPosRight[oNumPos]; // m_oXHigher at encoder positions
	UNUSED double oYPosLeft[oNumPos], oYPosRight[oNumPos]; // m_oYHigher at encoder positions
	tVecDouble oDist; oDist.resize(2, 0.0);
	unsigned int oBCnt=0; // we will compute one beta0 average afterwards

	// get current position and save value. we also need it to set new absolute positions
	int oOldPos = m_rCalibrationManager.getHeadPos();
	int oCurrPos = oOldPos;
//	m_rCalibrationManager.setHeadMode(eAxisY, Position, false);

	setFilterParametersLine(p_rGraph, LaserLine::FrontLaserLine, false, rCalibrationParameters);  // nimm einfach die erste Linie - es muss eine ausgewaehlt werden.

	double oSlope = 0.0; double oIntercept = 0.0;  // averaged slope and intercept over a
	double oTmpSlope = 0.0; double oTmpInt = 0.0;

	std::stringstream oTitle("");

	// for stability reasons and potential jitter of the axis we average over four images
	for (int m=0; m < oNumPos; ++m)
	{
		oCurrPos += static_cast<int>(oRelPos[m]/oMmPerStep);
		oTitle.str("");
		oTitle << "@AxisPos " << m+1 << "/" << oNumPos << ":";
		wmLogTr(eInfo, "QnxMsg.Calib.AxisCalibPos", "Please wait while collecting data for axis position %d of %d...\n", m+1, oNumPos);
		m_rCalibrationManager.setHeadPos( oCurrPos ); // set (new) head position given by oRelPos
		Poco::Thread::sleep(1200);
		if ( getAvgResultOfImages(m_oGraph, 4, 0, math::CameraRelatedParameters{}, oTitle.str()) ) // todo: sensorID
		{
			oXPosLeft[oBCnt] = m_oXHigher[0]; oXPosRight[oBCnt] = m_oXHigher[1];
			oYPosLeft[oBCnt] = m_oYHigher[0]; oYPosRight[oBCnt] = m_oYHigher[1];
			++oBCnt;
			computeLineRegression(oTmpSlope, oTmpInt, m_oXPlaneHigher, m_oYPlaneHigher); // get slope and intercept data for higher layer of calibration workpiece
			oSlope += oTmpSlope; oIntercept += oTmpInt;
		}
	}

	// at least all but one axis positions must give useful results and at least 3 positions need to have been computed successfully at all
	if ( (oBCnt < 3) || (oBCnt < (oNumPos - 1)) )
	{
		wmLogTr(eError, "QnxMsg.Calib.BadCalib",  "Calibration failed. Bad lie or illumination of calibration workpiece or improper parameters.\n");
		showLivePic("Axis calibration failed");
		return false;
	}

	oSlope /= oBCnt; oIntercept /= oBCnt; // get averaged regression data...
	double oAngle = std::atan(oSlope);    // ... to compute angle of calibration workpiece image in computer

	// compute axis ratioPerMm factor averaged over successful image evaluations from above
	double oDPixX = rCalibrationParameters.getDouble("DpixX");
	double oDPixY = rCalibrationParameters.getDouble("DpixY");
	double oFactor = 0.0;
	for (unsigned int i=1; i < oBCnt; ++i)
	{
		double oYDist = std::abs(oYPosLeft[i] - oYPosRight[i] - oYPosLeft[i-1] + oYPosRight[i-1]);
		if ( (oYDist*oDPixY) >  0.1)
		{
			// todo warning movement > 0.1mm
		}
		// unused -> double oYAvg = (int)( (oYPosLeft[i] + oYPosRight[i] + oYPosLeft[i-1] + oYPosRight[i-1]) * 0.25); // take y position average of two positions
		double oBeta0 = 0.5;// p_rCalibData.beta0(oYAvg);
		if (oBeta0 < math::eps)
		{
			wmLogTr(eError, "QnxMsg.Calib.InvalidBeta0", "Calibration failed, invalid magnification. Recalibration recommended!\n");
			showLivePic("Axis calibration failed");
			return false;
		}
		oDist[0] = 0.0;

		// we divide by cos(oAngle) to respect length differences originating from a potential tilt, which results in a down- or upslope of the layers.
		oDist[1] = (std::abs(oXPosLeft[i] - oXPosLeft[i-1] + std::abs(oXPosRight[i] - oXPosRight[i-1]))*0.5)*oDPixX/std::cos(oAngle) / oBeta0; // averaging over both ends for stability reasons
		math::lengthRatio(oFactor, 0, oRelPos[i], oRelPos[i], oDist); // compute pix to mm factor for axis
		p_rRatio += oFactor;
	}
	p_rRatio /= (oBCnt-1); // oSum is now the factor in mm distances of the axis need to be multiplied with to get the REAL distances!

	m_rCalibrationManager.setHeadPos(oOldPos);   // move axis back to former position

	BImage oOldImg = m_rCalibrationManager.getImage();
	showLivePic("Axis calibration OK");

	return true;
}

// main routine for axis calibration
bool CalibrateIbAxis::calibrate() // todo: sensorID
{
	CalibrateIb::calibrate();

	if (false) // todo:
	{
		wmLogTr(eWarning, "QnxMsg.Calib.NoBaseCalib", "Base calibration of the optical system has not yet been performed!\n");
		return false;
	}

	// reload data
	auto& rCalibrationData = m_rCalibrationManager.getCalibrationData(0);
	assert(rCalibrationData.isInitialized() && "CalibrateIbAxis::calibrate() sensor not initialized");

	// if the axis does not move into just one direction, we need to take this into account: how much does the axis really move into the x-direction?
	double oRatio = 1.0;
	if ( calibrateAxis(oRatio, rCalibrationData, m_oGraph) )
	{
		wmLogTr(eInfo, "QnxMsg.Calib.AxisFactorY", "Correction multiplier for y-direction of axis: %f\n", oRatio);
		rCalibrationData.setKeyValue("axisCorrectionFactorY", oRatio);
		//let calibrationmanager write the xml file and do the rest
        m_rCalibrationManager.sendCalibDataChangedSignal(rCalibrationData.getSensorId(), true ) ;
		wmLogTr(eInfo, "QnxMsg.Calib.CalibOK", "Calibration successfully completed !\n");
		return true;
	}

	wmLogTr(eWarning, "QnxMsg.Calib.BadAxisCalib", "Calibration of axis failed. Bad image or calibration parameters?\n");
	return true; //return false;
}

}
}

