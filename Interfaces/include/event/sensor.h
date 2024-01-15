/**
 * 	@file
 *  @copyright		Precitec Vision GmbH & Co. KG
 *  @author			kir, wor, js, al, hs, al
 *  @date			2009
 *  @brief			Sensor types
 */

#pragma once


#if ((_MANAGED == 1) || (_M_CEE == 1)) && PrecitecJunctionDll
	#pragma message( " Managed build " )
#else
	// #pragma message( " Native build " ) // Sieht aus wie warning, daher kommentiert.
#endif

#if ((_MANAGED == 1) || (_M_CEE == 1)) && PrecitecJunctionDll
namespace Precitec { namespace Junction {
#else

#include <vector>

#include "image/image.h"
#include "common/geoContext.h"

#include "common/triggerContext.h"

namespace precitec
{
namespace interface
{
#endif

	// Liste aller moeglicher Sensoren

#if ((_MANAGED == 1) || (_M_CEE == 1)) && PrecitecJunctionDll
	public enum class SensorType {
#else
	enum Sensor	{
#endif

		// all ids between 0 and 9999 denominate camera sensors

		eImageSensorDefault		= 0,
		eImageSensorMin			= 0,
		eImageSensorMax			= 9999,

		// all ids between 10000 and 19999 denominate analog sensors

		eExternSensorDefault		= 10000,
		eWeldHeadAxisXPos		= 10000,
		eWeldHeadAxisYPos,
		eWeldHeadAxisZPos,
		eGlasNotPresent,
		eGlasDirty,
		eTempGlasFail,
		eTempHeadFail,
		eLaserPowerSignal,
		eEncoderInput1,
		eEncoderInput2,
		eRobotTrackSpeed,         // 10010
		eOversamplingSignal1,
		eOversamplingSignal2,
		eOversamplingSignal3,
		eOversamplingSignal4,
		eOversamplingSignal5,
		eOversamplingSignal6,
		eOversamplingSignal7,
		eOversamplingSignal8,
		eGenPurposeDigIn1,
		eGenPurposeDigIn2,        // 10020
		eGenPurposeDigIn3,
		eGenPurposeDigIn4,
		eGenPurposeDigIn5,
		eGenPurposeDigIn6,
		eGenPurposeDigIn7,
		eGenPurposeDigIn8,
        eGenPurposeAnalogIn1,
        eIDMWeldingDepth,
        eIDMTrackingLine,
		eIDMOverlayImage,         // 10030
		eIDMSpectrumLine,
		eScannerXPosition,
		eScannerYPosition,
		eLWM40_1_Plasma,
		eLWM40_1_Temperature,
		eLWM40_1_BackReflection,
		eLWM40_1_AnalogInput,
        eGenPurposeAnalogIn2,
        eGenPurposeAnalogIn3,
        eGenPurposeAnalogIn4,     // 10040
        eGenPurposeAnalogIn5,
        eGenPurposeAnalogIn6,
        eGenPurposeAnalogIn7,
        eGenPurposeAnalogIn8,
        eS6K_Leading_Result1,
        eS6K_Leading_Result2,
        eS6K_Leading_Result3,
        eS6K_Leading_Result4,
        eZCPositionDigV1,
        eScannerWeldingFinished,  // 10050
        eFiberSwitchPosition,
        eProfileLines,
        eIDMQualityPeak1,
        eContourPrepared,
		eExternSensorMin		= 10000,
		eExternSensorMax		= 19999,

		// delimiters

		eSensorMin				= eImageSensorMin,
		eSensorMax				= eExternSensorMax
	};


} // namespace interface
} // namespace precitec


