/**
*	@file
*	@copyright	Precitec Vision GmbH & Co. KG
*	@author		Simon Hilsenbeck (HS)
*	@date		2011
*	@brief		enums that represent filter paramters
*/


#ifndef PARAMETERENUMS_20110921_H
#define PARAMETERENUMS_20110921_H

#include <string>
#include <cassert>

namespace precitec {
	namespace filter {


/**
*	@brief		rank definition for int rank, serves as reliability measure for measuring values
*	@details	ATTENTION: there is also a rank of type double used in TGeo. eRankMax must not be zero.
*/
enum ValueRankType {
	eRankMin  			= 0,			///< worst rank
	eRankMax			= 255,			///< best rank
	eValueRankTypeMin	= eRankMin,		///< delimiter
	eValueRankTypeMax	= eRankMax		///< delimiter
};



/**
*	@brief		Type of data in a TArray. May either be data or rank.
*	@details	Used for tuple access. Eg: std::tuple<int, int> first = myIntArray[0]; int oRank = std::get<eRank>(first);
*/
enum TArrayDataType {
	eData  				= 0,		///< data
	eRank				= 1,		///< rank
	eValue = 2,
	TArrayDataTypeMin	= eData,	///< delimiter
	TArrayDataTypeMax	= eRank		///< delimiter
};



/**
*	@brief		Extremum type for extremum search
*	@details	Used in filter 'LineExtremum' and 'SimpleTracking'
*/
enum ExtremumType {
	eMinimum  			= 0,				///< minimum
	eMaximum			= 1,				///< maximum
	eZeroCrossing       = 2,
	eExtremumTypeMin	= eMinimum,			///< delimiter
	eExtremumTypeMax    = eZeroCrossing			///< delimiter
}; // ExtremumType

/**
*	@brief		Search direction type for extremum search
*	@details	Used in filter 'LineExtremum'
*/
enum SearchDirType {
	eFromLeft  			= 0,
	eFromRight			= 1,
	eSearchDirTypeMin	= eFromLeft,		///< delimiter
	eSearchDirTypeMax	= eFromRight		///< delimiter
}; // SearchDirType

// Richtung der Nahtraupe (nach oben oder nach unten)
enum RunOrientation {
	eOrientationInvalid = 0, // Nahtueberhoehung. Buckel IM  BILD (!) nach oben gerichtet, d.h. starkes Minimum da coords umgekehrt!
	eOrientationConcave = 1, // Nahteinzug. dito, nach unten gerichtet
	eOrientationConvex = 2,
	eOrientationRunUpward = eOrientationConvex, // der Klarheit halber
	eOrientationRunDownward = eOrientationConcave //  der Klarheit halber
};
typedef RunOrientation TRunOrientation;

/**
*	@brief		Specifies which laser line is being used
*	@details	Used in all filter that need coordinates from the calibration
*/
enum LaserLine {
	FrontLaserLine = 0, //vorlaufende Linie (WM full), laserline1 (WM compact)
	BehindLaserLine, //nachlaufende Linie (WM full), laserline2 (WM compact)
	CenterLaserLine, //TCP Linie (wm full), laserline3 (WM compact)
	NumberLaserLines
};

inline LaserLine castToLaserLine(int tmpLaserLine)
{
	if ( tmpLaserLine >= 0 && tmpLaserLine < static_cast<int>(LaserLine::NumberLaserLines) )
	{
		return static_cast<LaserLine>(tmpLaserLine);
	}
	else
	{
		return LaserLine::FrontLaserLine;
	}
}
inline std::string laserLineName(LaserLine p_laserline) 
{
    switch(p_laserline)
    {
        case LaserLine::FrontLaserLine: return "LineLaser1";
        case LaserLine::BehindLaserLine: return "LineLaser2";
        case LaserLine::CenterLaserLine: return "LineLaser3";
        case LaserLine::NumberLaserLines: return "Number of laser lines";
    }
    assert(false && "case not handled in switch");
    return "";
}

enum class MeasurementType
{
    LineLaser1 = LaserLine::FrontLaserLine,
    LineLaser2 = LaserLine::BehindLaserLine,
    LineLaser3 = LaserLine::CenterLaserLine,
    Image //measurement on gray scale image
};
static_assert(static_cast<int>(MeasurementType::Image)>= LaserLine::NumberLaserLines, "MeasurementType in conflict with LaserLine enum");


/**
*	@brief		comparison type for functor definition
*	@details	Used in filter 'Binarize'
*/
enum ComparisonType {
	eLess				= 0,				///< smaller (op <)
	eGreaterEqual  		= 1,				///< greater equal (op >=)
	eComparisonTypeMin	= eLess,			///< delimiter
	eComparisonTypeMax	= eGreaterEqual		///< delimiter
}; // ComparisonType



/**
*	@brief		gradient type for assymetric gradient filter
*	@details	Used in filter 'Gradient'
*/
enum GradientType {
	eAbsolute  			= 0,				///< absolute gradient
	eDarkSeam			= 1,				///< dark seam (bright dark bright)
	eBrightSeam			= 2,				///< bright seam  (dark bright dark)
	eGradientTypeMin	= eAbsolute,		///< delimiter
	eGradientTypeMax	= eBrightSeam		///< delimiter
}; // GradientType



/**
*	@brief		Filter type
*	@details	Used in LineMovingAverage, TemporalLowPass
*/
enum class FilterAlgorithmType {
	eMean  			= 0,				///< filter algorithm mean
	eMedian			= 1,				///< filter algorithm median
	eMinLowPass = 2,					///< filter algorithm minimum
	eMaxLowPass = 3,					///< filter algorithm maximum
    eStdDeviation = 4,
	eTypeMin	= eMean,
	eTypeMax	= eStdDeviation
};



/**
*	@brief		Coordinate type 
*	@details	Used in array.h
*/
enum CoordinateType {
	eX  				= 0,			///< x coordinate
	eY,									///< y coordinate
	eZ,									///< z coordinate
	eCoordinateTypeMin	= eX,			///< delimiter
	eCoordinateTypeMax	= eZ			///< delimiter
}; // CoordinateType



/**
*	@brief		Texture analysis algorithm type 
*	@details	Used in surface.h, tileFeature.h
*/
enum class AlgoTextureType {
	eVariance  			= 0,				///< texture analysis algorithm calcVariance
	eMinMaxDistance,						///< texture analysis algorithm calcMinMaxDistance
	eGradientX,								///< texture analysis algorithm calcGradientX
	eGradientY,								///< texture analysis algorithm calcGradientY
	eMeanIntensity,							///< texture analysis algorithm calcMeanIntensity
    eMinIntensity,
    eMaxIntensity,
	eAlgoTextureTypeMin = eVariance,		///< delimiter
	eAlgoTextureTypeMax = eMaxIntensity	///< delimiter
}; // AlgoTextureType



/**
*	@brief		Morphological operation type 
*	@details	Used in morphology.h
*/
enum MorphOpType {
	eOpening  			= 0,				///< Opening
	eClosing,								///< Closing
	eOpeningClosing,						///< Opening and Closing
	eMorphOpTypeMin		= eOpening,			///< delimiter
	eMorphOpTypeMax		= eOpeningClosing	///< delimiter
}; // MorphOpType


/**
*	@brief		Edge Detection operation type
*	@details	Used in edgeDetection.h
*/
enum EdgeOpType {
	eRoberts =0,					///< roberts Op
	eSobel,						    ///< Sobel Op
	eKirsch,						///< Kirsch Op
	eCanny,                         ///<Canny Edge Op
}; // MorphOpType

/**
*	@brief		Calculation type for laser lines
*	@details	Used in cavvex.h
*/
enum CalcType {
	eNormal = 0,
	eInverted
};

/**
*	@brief		Calculation type for laser lines
*	@details	Used in cavvex.h
*/
enum HoughDirectionType {
	eLeft = 0,
	eRight
};

/**
*	@brief		Binarize type
*	@details	Used in binarize.h
*/
enum class BinarizeType {
	eGlobal				= 0,		///< binarizes globally (global dynamic threshold)
	eLocal,							///< binarizes locally (windowed dynamic threshold)
	eStatic,						///< binarizes globally (global static threshold)
	eBinarizeTypeMin = eGlobal,		///< delimiter
	eBinarizeTypeMax = eStatic		///< delimiter
}; // BinarizeType

/**
*	@brief		Unary arithmetic operation type
*/
enum UnaryArithmeticType {
	eExponentialFunction  			= 0,			///< filter algorithm exponential function
	eSquareRoot						= 1,			///< filter algorithm square root
	eAbsoluteValue 					= 2,			///< filter algorithm absolute value
	eLogicalNOT 					= 3,			///< filter algorithm logical not
	eIntToUInt16 					= 4,			///< filter algorithm signed to unsigned int16
	eCumulativeSum					= 5,
	eSin                            = 6,            ///< filter algorithm sinus funktion
	eCos                            = 7,            ///< filter algorithm cosinus funktion
	eTan                            = 8,            ///< filter algorithm tan funktion
	eArcSin                         = 9,            ///< filter algorithm Arcsinus funktion
	eArcCos                         = 10,           ///< filter algorithm Arccosinus funktion
	eArcTan                         = 11,           ///< filter algorithm Arctan funktion
    eUnaryArithmeticTypeMin	= eExponentialFunction,	///< filter algorithm minimum
	eUnaryArithmeticTypeMax = eArcTan			///< filter algorithm maximum
}; // UnaryArithmeticType


enum BlobAdapterType
{
	eMassCenterX = 0, // Blob Structure
	eMassCenterY, // Blob Structure
	eMassCenterNormalizingFactor, // Blob Structure
	eMassCenterXNormalized,
	eMassCenterYNormalized,
	eArea, // Blob Structure
	eXMin, // Blob Structure
	eXMax, // Blob Structure
	eYMin, // Blob Structure
	eYMax, // Blob Structure
	eWidth,
	eHeight,
	eStartX, // Blob Structure
	eStartY // Blob Structure
};

enum SeamFindingAdapterType
{
	eSeamLeft = 0, 
	eSeamRight, 
	eSeamPosition, 
	eSeamWidth,
	eAlgoType,
	eQuality,
};

	} // namespace filter
} // namespace precitec


#endif // PARAMETERENUMS_20110921_H
