#ifndef CALIBRATIONSTRUCTURES_H
#define CALIBRATIONSTRUCTURES_H

#include "calibrationCommon.h"
#include "math/CalibrationParamMap.h"

// stl includes
#include <string>
#include <vector>
#include <map>
#include <memory>

#include "math/mathCommon.h"
#include "filter/parameterEnums.h" //filter::LaserLine
#include "math/2D/LineEquation.h"

namespace precitec
{
namespace math
{


struct CoaxTriangulationAngle
{
public :
	double beta0;
	double betaz;
	bool highPlaneOnImageTop;
    LineEquation m_laserLineOnXYPlane = {0.0, 1.0, 0.0}; //horizontal by default
	//explain line direction conventions
	static bool getHighPlaneOnImageTopFromAngle(const double pAngle)
	{
		return tan(pAngle) > 0;
	}
	static double getAngleFromLineDirectionConvention(const double p_angle, const bool isHighPlaneOnTop)
	{
		return isHighPlaneOnTop ? std::abs(p_angle) : -std::abs(p_angle);
	}

	//reimplemented also in SecondScreenSettings.ComputeTriangulationAngleFromKeyVal (Services/SecondScreenSettings.cs)
	bool computeAngle(double & r_angle, angleUnit p_oUnit) const
	{
		if ( beta0 == 0 || betaz == 0 )
		{
			return false;
		}

		double angle_rad = getAngleFromLineDirectionConvention(atan2(betaz, beta0), highPlaneOnImageTop);
		r_angle = p_oUnit == angleUnit::eDegrees ? math::radiansToDegrees(angle_rad) : angle_rad;
		return true;
	}

	double getAngle(angleUnit p_oUnit = angleUnit::eDegrees) const
	{
        assert(beta0 != 0 && betaz != 0 && "getAngle used with invalid configuration");
		//initialize with 0 (value returned in case of invalid configuration)
        double result = 0.0;
		computeAngle(result, p_oUnit);
		return result;
	}
	
};


//camera parameters used in the calibration parameters computation
struct CameraRelatedParameters
{
	double m_oDpixX;
	double m_oDpixY;
	int m_oWidth;
	int m_oHeight;
};


/**
* @brief Tiny structure reflecting the Coax calibration KeyValues.
*/
struct  CoaxCalibrationData
{
public:
	// data
	double m_oBeta0;
	double m_oBetaZ;
	double m_oBetaZ2;
	double m_oBetaZTCP;
	double m_oDpixX;
	double m_oDpixY;
	unsigned int m_oWidth;
	unsigned int m_oHeight;
	unsigned int m_oOrigX;
	unsigned int m_oOrigY;
	double m_oAxisFactor;
	bool m_oHighPlaneOnImageTop;
	bool m_oHighPlaneOnImageTop_2;
	bool m_oHighPlaneOnImageTop_TCP;
    bool m_oInvertX;
    bool m_oInvertY;
    math::LineEquation m_oLineXY;
    math::LineEquation m_oLineXY_2;
    math::LineEquation m_oLineXY_TCP;

	// methods
	CoaxCalibrationData() :
		m_oBeta0(0.5),
		m_oBetaZ(0.5), m_oBetaZ2(0.5), m_oBetaZTCP(0.5),
		m_oDpixX(0.0106), m_oDpixY(0.0106),
		m_oWidth(1024), m_oHeight(1024),
		m_oOrigX(512), m_oOrigY(512),
		m_oAxisFactor(1.0),
		m_oHighPlaneOnImageTop(true), m_oHighPlaneOnImageTop_2(true), m_oHighPlaneOnImageTop_TCP(true),
		m_oInvertX(false),
		m_oInvertY(false),
		m_oLineXY(0.0,1.0,0.0), m_oLineXY_2(0.0, 1.0, 0.0), m_oLineXY_TCP(0.0, 1.0, 0.0)
	{};
	
	CoaxCalibrationData(const math::CalibrationParamMap& p_ParamMap):
		m_oBeta0 ( p_ParamMap.getDouble("beta0")),
		m_oBetaZ ( p_ParamMap.getDouble("betaZ")),
		m_oBetaZ2 ( p_ParamMap.getDouble("betaZ_2")),
		m_oBetaZTCP(p_ParamMap.getDouble("betaZ_TCP")),
		m_oDpixX ( p_ParamMap.getDouble("DpixX")),
		m_oDpixY ( p_ParamMap.getDouble("DpixY")),
		m_oWidth ( p_ParamMap.getInt("sensorWidth")),
		m_oHeight ( p_ParamMap.getInt("sensorHeight")),
		m_oAxisFactor ( p_ParamMap.getDouble("axisCorrectionFactorY")),
		m_oHighPlaneOnImageTop(p_ParamMap.getInt("HighPlaneOnImageTop") !=0),
		m_oHighPlaneOnImageTop_2(p_ParamMap.getInt("HighPlaneOnImageTop_2") != 0),
		m_oHighPlaneOnImageTop_TCP(p_ParamMap.getInt("HighPlaneOnImageTop_TCP") != 0),
		m_oInvertX(p_ParamMap.getBool("InvertX")),
		m_oInvertY(p_ParamMap.getBool("InvertY")),
		m_oLineXY ( p_ParamMap.getDouble("laserLine_a"),p_ParamMap.getDouble("laserLine_b") ,p_ParamMap.getDouble("laserLine_c")),
		m_oLineXY_2 ( p_ParamMap.getDouble("laserLine_a_2"),p_ParamMap.getDouble("laserLine_b_2") ,p_ParamMap.getDouble("laserLine_c_2")),
		m_oLineXY_TCP ( p_ParamMap.getDouble("laserLine_a_TCP"),p_ParamMap.getDouble("laserLine_b_TCP") ,p_ParamMap.getDouble("laserLine_c_TCP"))
	{
        //handle automatic initialization of xcc, ycc
        auto xcc = p_ParamMap.getDouble("xcc");
		m_oOrigX = xcc >= 0?  static_cast<unsigned int>(xcc) : m_oWidth/2;
        auto ycc = p_ParamMap.getDouble("ycc");
		m_oOrigY = ycc >= 0?  static_cast<unsigned int>(ycc) : m_oHeight/2;

	}

	void getLineDependentParameters(double & r_betaz, bool & r_isHighPlaneOnImageTop, math::LineEquation & r_lineXYEquation,
		const filter::LaserLine p_oLaserLine) const
	{
		switch ( p_oLaserLine )
				{
					case filter::LaserLine::FrontLaserLine:
					default:
						r_betaz = m_oBetaZ;
						r_isHighPlaneOnImageTop = m_oHighPlaneOnImageTop;
 						r_lineXYEquation = m_oLineXY;
						break;
					case filter::LaserLine::BehindLaserLine:
						r_betaz = m_oBetaZ2;
						r_isHighPlaneOnImageTop = m_oHighPlaneOnImageTop_2;
 						r_lineXYEquation = m_oLineXY_2;
						break;
					case filter::LaserLine::CenterLaserLine:
						r_betaz = m_oBetaZTCP;
						r_isHighPlaneOnImageTop = m_oHighPlaneOnImageTop_TCP;
 						r_lineXYEquation = m_oLineXY_TCP;
						break;
				}
	}

	static std::string ParameterKeySuffix(filter::LaserLine p_laserLine) //suffix for calibration key values
	{
        switch(p_laserLine)
        {
            case filter::LaserLine::FrontLaserLine: return ""; //linelaser1 in gui
            case filter::LaserLine::BehindLaserLine: return "_2"; //linelaser2 in gui
            case filter::LaserLine::CenterLaserLine: return "_TCP"; // linelaser3 in gui;
            case filter::LaserLine::NumberLaserLines: return "?";
        }
        assert(false && "case not handled in switch");
        return "?";
	}

	bool computeTriangulationAngle(double & r_angle, const filter::LaserLine p_oLaserLine) const
	{
		CoaxTriangulationAngle angleParameters;	
		angleParameters.beta0 = m_oBeta0;
		angleParameters.betaz = 0;
		angleParameters.highPlaneOnImageTop = true;
 		math::LineEquation lineXY;

		getLineDependentParameters(angleParameters.betaz, angleParameters.highPlaneOnImageTop, lineXY, p_oLaserLine);
		return angleParameters.computeAngle(r_angle, angleUnit::eDegrees);
	}


	double computeTriangulationAngle(const filter::LaserLine p_oLaserLine) const
	{
		double angle = 0;
		computeTriangulationAngle(angle, p_oLaserLine);
		return angle;
	}
};


inline bool operator==(const CoaxCalibrationData& lhs, const CoaxCalibrationData& rhs)
{
	double tol = 1e-10;
	if ( std::abs(lhs.m_oBeta0					-  rhs.m_oBeta0					 ) > tol )	{return false;};
	if ( std::abs(lhs.m_oBetaZ					-  rhs.m_oBetaZ					 ) > tol )	{return false;}; 
	if ( std::abs(lhs.m_oBetaZ2					-  rhs.m_oBetaZ2				 ) > tol )	{return false;};
	if ( std::abs(lhs.m_oBetaZTCP				-  rhs.m_oBetaZTCP				 ) > tol )	{return false;}; 
	if ( std::abs(lhs.m_oDpixX					-  rhs.m_oDpixX					 ) > tol )	{return false;}; 
	if ( std::abs(lhs.m_oDpixY					-  rhs.m_oDpixY					 ) > tol )	{return false;};
	if			( lhs.m_oWidth					!= rhs.m_oWidth					 )			{return false;};
	if			( lhs.m_oHeight					!= rhs.m_oHeight				 )			{return false;};
	if			( lhs.m_oOrigX					!= rhs.m_oOrigX					 )			{return false;};
	if			( lhs.m_oOrigY					!= rhs.m_oOrigY					 )			{return false;};
	if ( std::abs(lhs.m_oAxisFactor				-  rhs.m_oAxisFactor			 ) > tol )	{return false;};
	if			( lhs.m_oHighPlaneOnImageTop	!= rhs.m_oHighPlaneOnImageTop	 )			{return false;};
	if			( lhs.m_oHighPlaneOnImageTop_2	!= rhs.m_oHighPlaneOnImageTop_2	 )			{return false;};
	if			( lhs.m_oHighPlaneOnImageTop_TCP!= rhs.m_oHighPlaneOnImageTop_TCP)			{return false;};
    if			( lhs.m_oInvertX				!= rhs.m_oInvertX			 	)			{return false;};
    if			( lhs.m_oInvertY				!= rhs.m_oInvertY			 	)			{return false;};

	return true;
};



/**
* @brief Tiny structure reflecting the LED calibration KeyValues.
*/
struct LEDCalibrationData
{
public:
    int m_width;
    int m_height;

    int m_calibrationChannel;

    int m_referenceBrightnessChannel1;
    int m_referenceBrightnessChannel2;
    int m_referenceBrightnessChannel3;
    int m_referenceBrightnessChannel4;
    int m_referenceBrightnessChannel5;
    int m_referenceBrightnessChannel6;
    int m_referenceBrightnessChannel7;
    int m_referenceBrightnessChannel8;

    int m_measuredBrightnessChannel1;
    int m_measuredBrightnessChannel2;
    int m_measuredBrightnessChannel3;
    int m_measuredBrightnessChannel4;
    int m_measuredBrightnessChannel5;
    int m_measuredBrightnessChannel6;
    int m_measuredBrightnessChannel7;
    int m_measuredBrightnessChannel8;

    int m_blackLevelShift;

    LEDCalibrationData():
        m_width(1),
        m_height(1),
        m_calibrationChannel(0),
        m_referenceBrightnessChannel1(125),
        m_referenceBrightnessChannel2(125),
        m_referenceBrightnessChannel3(125),
        m_referenceBrightnessChannel4(125),
        m_referenceBrightnessChannel5(125),
        m_referenceBrightnessChannel6(125),
        m_referenceBrightnessChannel7(125),
        m_referenceBrightnessChannel8(125),
        m_measuredBrightnessChannel1(0),
        m_measuredBrightnessChannel2(0),
        m_measuredBrightnessChannel3(0),
        m_measuredBrightnessChannel4(0),
        m_measuredBrightnessChannel5(0),
        m_measuredBrightnessChannel6(0),
        m_measuredBrightnessChannel7(0),
        m_measuredBrightnessChannel8(0)
    {};

    LEDCalibrationData(const math::CalibrationParamMap& parameterMap):
        m_width(1),
        m_height(1),
        m_calibrationChannel(parameterMap.getInt("ledCalibrationChannel")),
        m_referenceBrightnessChannel1(parameterMap.getInt("ledChannel1ReferenceBrightness")),
        m_referenceBrightnessChannel2(parameterMap.getInt("ledChannel2ReferenceBrightness")),
        m_referenceBrightnessChannel3(parameterMap.getInt("ledChannel3ReferenceBrightness")),
        m_referenceBrightnessChannel4(parameterMap.getInt("ledChannel4ReferenceBrightness")),
        m_referenceBrightnessChannel5(parameterMap.getInt("ledChannel5ReferenceBrightness")),
        m_referenceBrightnessChannel6(parameterMap.getInt("ledChannel6ReferenceBrightness")),
        m_referenceBrightnessChannel7(parameterMap.getInt("ledChannel7ReferenceBrightness")),
        m_referenceBrightnessChannel8(parameterMap.getInt("ledChannel8ReferenceBrightness")),
        m_measuredBrightnessChannel1(parameterMap.getInt("ledChannel1MeasuredBrightness")),
        m_measuredBrightnessChannel2(parameterMap.getInt("ledChannel2MeasuredBrightness")),
        m_measuredBrightnessChannel3(parameterMap.getInt("ledChannel3MeasuredBrightness")),
        m_measuredBrightnessChannel4(parameterMap.getInt("ledChannel4MeasuredBrightness")),
        m_measuredBrightnessChannel5(parameterMap.getInt("ledChannel5MeasuredBrightness")),
        m_measuredBrightnessChannel6(parameterMap.getInt("ledChannel6MeasuredBrightness")),
        m_measuredBrightnessChannel7(parameterMap.getInt("ledChannel7MeasuredBrightness")),
        m_measuredBrightnessChannel8(parameterMap.getInt("ledChannel8MeasuredBrightness"))
    {};
};

inline bool operator==(const LEDCalibrationData& lhs, const LEDCalibrationData& rhs)
{
    if (lhs.m_width != rhs.m_width)
    {
        return false;
    }
    if (lhs.m_height != rhs.m_height)
    {
        return false;
    }
    if (lhs.m_calibrationChannel != rhs.m_calibrationChannel)
    {
        return false;
    }
    if (lhs.m_referenceBrightnessChannel1 != rhs.m_referenceBrightnessChannel1)
    {
        return false;
    }
    if (lhs.m_referenceBrightnessChannel2 != rhs.m_referenceBrightnessChannel2)
    {
        return false;
    }
    if (lhs.m_referenceBrightnessChannel3 != rhs.m_referenceBrightnessChannel3)
    {
        return false;
    }
    if (lhs.m_referenceBrightnessChannel4 != rhs.m_referenceBrightnessChannel4)
    {
        return false;
    }
    if (lhs.m_measuredBrightnessChannel1 != rhs.m_measuredBrightnessChannel1)
    {
        return false;
    }
    if (lhs.m_measuredBrightnessChannel2 != rhs.m_measuredBrightnessChannel2)
    {
        return false;
    }
    if (lhs.m_measuredBrightnessChannel3 != rhs.m_measuredBrightnessChannel3)
    {
        return false;
    }
    if (lhs.m_measuredBrightnessChannel4 != rhs.m_measuredBrightnessChannel4)
    {
        return false;
    }

    return true;
};

} //math
} //precitec
#endif
