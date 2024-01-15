#include "resultHelper.h"
#include "event/results.h"
#include "event/sensor.h"

#include <QCoreApplication>
#include <QColor>
#include <QString>

namespace precitec
{

QString nameForResult(const precitec::interface::ResultType &type)
{
    static const std::map<precitec::interface::ResultType, std::pair<const char*, const char *>> s_resultTypeNames = {
        {precitec::interface::GapWidth, QT_TRANSLATE_NOOP3("", "Gap width", "Precitec.Results.GapWidth")},
        {precitec::interface::GapPosition, QT_TRANSLATE_NOOP3("", "Gap position", "Precitec.Results.GapPosition")},
        {precitec::interface::Missmatch, QT_TRANSLATE_NOOP3("", "Mismatch", "Precitec.Results.Missmatch")},
        {precitec::interface::AxisPositionAbsolute, QT_TRANSLATE_NOOP3("", "Axis position absolute", "Precitec.Results.AxisPositionAbsolute")},
        {precitec::interface::AxisPositionRelative, QT_TRANSLATE_NOOP3("", "Axis position relative", "Precitec.Results.AxisPositionRelative")},
        {precitec::interface::SeamWidth, QT_TRANSLATE_NOOP3("", "Seam width", "Precitec.Results.SeamWidth")},
        {precitec::interface::KeyholePosition, QT_TRANSLATE_NOOP3("", "Keyhole position", "Precitec.Results.KeyholePosition")},
        {precitec::interface::Convexity, QT_TRANSLATE_NOOP3("", "Convexity", "Precitec.Results.Convexity")},
        {precitec::interface::Roundness, QT_TRANSLATE_NOOP3("", "Roundness", "Precitec.Results.Roundness")},
        {precitec::interface::NoSeam, QT_TRANSLATE_NOOP3("", "No seam", "Precitec.Results.NoSeam")},
        {precitec::interface::NoStructure, QT_TRANSLATE_NOOP3("", "No structure", "Precitec.Results.NoStructure")},
        {precitec::interface::SideBurn, QT_TRANSLATE_NOOP3("", "Sideburn", "Precitec.Results.SideBurn")},
        {precitec::interface::Pores, QT_TRANSLATE_NOOP3("", "Pores", "Precitec.Results.Pores")},
        {precitec::interface::Holes, QT_TRANSLATE_NOOP3("", "Holes", "Precitec.Results.Holes")},
        {precitec::interface::Spurt, QT_TRANSLATE_NOOP3("", "Spurt", "Precitec.Results.Spurt")},
        {precitec::interface::Notch, QT_TRANSLATE_NOOP3("", "Notch", "Precitec.Results.Notch")},
        {precitec::interface::EdgeSkew, QT_TRANSLATE_NOOP3("", "EdgeSkew", "Precitec.Results.EdgeSkew")},
        {precitec::interface::SeamThickness, QT_TRANSLATE_NOOP3("", "Seam thickness", "Precitec.Results.SeamThickness")},
        {precitec::interface::UnEqualLegs, QT_TRANSLATE_NOOP3("", "UnEquelLegs", "Precitec.Results.UnEqualLegs")},
        {precitec::interface::SeamLength, QT_TRANSLATE_NOOP3("", "Seam length", "Precitec.Results.SeamLength")},
        {precitec::interface::ReangeLength, QT_TRANSLATE_NOOP3("", "Range length", "Precitec.Results.ReangeLength")},
        {precitec::interface::Weldbed, QT_TRANSLATE_NOOP3("", "Welding pool", "Precitec.Results.Weldbed")},
        {precitec::interface::ControlOut, QT_TRANSLATE_NOOP3("", "Control out", "Precitec.Results.ControlOut")},
        {precitec::interface::NoLaser, QT_TRANSLATE_NOOP3("", "No laser", "Precitec.Results.NoLaser")},
        {precitec::interface::NoLight, QT_TRANSLATE_NOOP3("", "No light", "Precitec.Results.NoLight")},
        {precitec::interface::LaserPower, QT_TRANSLATE_NOOP3("", "Laser power", "Precitec.Results.LaserPower")},
        {precitec::interface::Value, QT_TRANSLATE_NOOP3("", "Value", "Precitec.Results.Value")},
        {precitec::interface::CoordPosition, QT_TRANSLATE_NOOP3("", "CoordPosition", "Precitec.Results.CoordPosition")},
        {precitec::interface::CoordPositionX, QT_TRANSLATE_NOOP3("", "CoordPositionX", "Precitec.Results.CoordPositionX")},
        {precitec::interface::CoordPositionY, QT_TRANSLATE_NOOP3("", "CoordPositionY", "Precitec.Results.CoordPositionY")},
        {precitec::interface::CoordPositionZ, QT_TRANSLATE_NOOP3("", "CoordPositionZ", "Precitec.Results.CoordPositionZ")},
        {precitec::interface::EFAINFeatures, QT_TRANSLATE_NOOP3("", "EFAINFeatures", "Precitec.Results.EFAINFeatures")},
        {precitec::interface::XCoordOutOfLimits, QT_TRANSLATE_NOOP3("", "X position out of limits", "Precitec.Results.XCoordOutOfLimits")},
        {precitec::interface::YCoordOutOfLimits, QT_TRANSLATE_NOOP3("", "Y position out of limits", "Precitec.Results.YCoordOutOfLimits")},
        {precitec::interface::ZCoordOutOfLimits, QT_TRANSLATE_NOOP3("", "Z position out of limits", "Precitec.Results.ZCoordOutOfLimits")},
        {precitec::interface::ValueOutOfLimits, QT_TRANSLATE_NOOP3("", "Value out of limits", "Precitec.Results.ValueOutOfLimits")},
        {precitec::interface::RankViolation, QT_TRANSLATE_NOOP3("", "RankViolation", "Precitec.Results.RankViolation")},
        {precitec::interface::GapPositionError, QT_TRANSLATE_NOOP3("", "GapPositionError", "Precitec.Results.GapPositionError")},
        {precitec::interface::LaserPowerOutOfLimits, QT_TRANSLATE_NOOP3("", "LaserPowerOutOfLimits", "Precitec.Results.LaserPowerOutOfLimits")},
        {precitec::interface::SensorOutOfLimits, QT_TRANSLATE_NOOP3("", "SensorOutOfLimit", "Precitec.Results.SensorOutOfLimits")},
        {precitec::interface::Surveillance01, QT_TRANSLATE_NOOP3("", "Surveillance 1", "Precitec.Results.Surveillance01")},
        {precitec::interface::Surveillance02, QT_TRANSLATE_NOOP3("", "Surveillance 2", "Precitec.Results.Surveillance02")},
        {precitec::interface::Surveillance03, QT_TRANSLATE_NOOP3("", "Surveillance 3", "Precitec.Results.Surveillance03")},
        {precitec::interface::Surveillance04, QT_TRANSLATE_NOOP3("", "Surveillance 4", "Precitec.Results.Surveillance04")},
        {precitec::interface::NoResultsError, QT_TRANSLATE_NOOP3("", "No Results", "Precitec.Results.NoResultsError")},
        {precitec::interface::BPTinySeam, QT_TRANSLATE_NOOP3("", "BPtinyseam", "Precitec.Results.BPTinySeam")},
        {precitec::interface::AnalysisOK, QT_TRANSLATE_NOOP3("", "Analysis OK", "Precitec.Results.AnalysisOK")},
        {precitec::interface::AnalysisErrBadLaserline, QT_TRANSLATE_NOOP3("", "Laser line insufficient", "Precitec.Results.AnalysisErrBadLaserline")},
        {precitec::interface::AnalysisErrNoBeadOrGap, QT_TRANSLATE_NOOP3("", "No weld bead or gap detected", "Precitec.Results.AnalysisErrNoBeadOrGap")},
        {precitec::interface::AnalysisErrBlackImage, QT_TRANSLATE_NOOP3("", "Black image", "Precitec.Results.AnalysisErrBlackImage")},
        {precitec::interface::AnalysisErrBadImage, QT_TRANSLATE_NOOP3("", "Image overtriggered", "Precitec.Results.AnalysisErrBadImage")},
        {precitec::interface::AnalysisErrBadCalibration, QT_TRANSLATE_NOOP3("", "Calibration errror", "Precitec.Results.AnalysisErrBadCalibration")},
        {precitec::interface::AnalysisErrBadROI, QT_TRANSLATE_NOOP3("", "Invalid ROI", "Precitec.Results.AnalysisErrBadROI")},
        {precitec::interface::AnalysisErrDefectiveSeam, QT_TRANSLATE_NOOP3("", "Defective Seam", "Precitec.Results.AnalysisErrDefectiveSeam")},
        {precitec::interface::AnalysisErrNotchDefect, QT_TRANSLATE_NOOP3("", "Defective notch", "Precitec.Results.AnalysisErrNotchDefect")},
        {precitec::interface::AnalysisErrNoStep, QT_TRANSLATE_NOOP3("", "No step", "Precitec.Results.AnalysisErrNoStep")},
        {precitec::interface::LD50_Y_AbsolutePosition, QT_TRANSLATE_NOOP3("", "LD50 Y absolute position", "Precitec.Results.LD50_Y_AbsolutePosition")},
        {precitec::interface::LD50_Y_RelativePosition, QT_TRANSLATE_NOOP3("", "LD50 Y relative position", "Precitec.Results.LD50_Y_RelativePosition")},
        {precitec::interface::LD50_Z_AbsolutePosition, QT_TRANSLATE_NOOP3("", "LD50 Z absolute position", "Precitec.Results.LD50_Z_AbsolutePosition")},
        {precitec::interface::LD50_Z_RelativePosition, QT_TRANSLATE_NOOP3("", "LD50 Z relative position", "Precitec.Results.LD50_Z_RelativePosition")},
        {precitec::interface::ScanTracker_Amplitude, QT_TRANSLATE_NOOP3("", "ScanTracker amplitude", "Precitec.Results.ScanTracker_Amplitude")},
        {precitec::interface::ScanTracker_Offset, QT_TRANSLATE_NOOP3("", "ScanTracker offset", "Precitec.Results.ScanTracker_Offset")},
        {precitec::interface::LaserControl_PowerOffset, QT_TRANSLATE_NOOP3("", "LaserControl power offset", "Precitec.Results.LaserControl_PowerOffset")},
        {precitec::interface::Z_Collimator_Position, QT_TRANSLATE_NOOP3("", "Z collimator position", "Precitec.Results.Z_Collimator_Position")},
        {precitec::interface::Fieldbus_PositionResult, QT_TRANSLATE_NOOP3("", "Fieldbus position result", "Precitec.Results.Fieldbus_PositionResult")},
        {precitec::interface::GeneralPurpose_AnalogOut1, QT_TRANSLATE_NOOP3("", "Analogout 1", "Precitec.Results.GeneralPurpose_AnalogOut1")},
        {precitec::interface::QualityFaultTypeA, QT_TRANSLATE_NOOP3("", "Quality Fault Type A", "Precitec.Results.QualityFaultTypeA")},
        {precitec::interface::QualityFaultTypeB, QT_TRANSLATE_NOOP3("", "Quality Fault Type B", "Precitec.Results.QualityFaultTypeB")},
        {precitec::interface::QualityFaultTypeC, QT_TRANSLATE_NOOP3("", "Quality Fault Type C", "Precitec.Results.QualityFaultTypeC")},
        {precitec::interface::QualityFaultTypeD, QT_TRANSLATE_NOOP3("", "Quality Fault Type D", "Precitec.Results.QualityFaultTypeD")},
        {precitec::interface::QualityFaultTypeE, QT_TRANSLATE_NOOP3("", "Quality Fault Type E", "Precitec.Results.QualityFaultTypeE")},
        {precitec::interface::QualityFaultTypeF, QT_TRANSLATE_NOOP3("", "Quality Fault Type F", "Precitec.Results.QualityFaultTypeF")},
        {precitec::interface::QualityFaultTypeG, QT_TRANSLATE_NOOP3("", "Quality Fault Type G", "Precitec.Results.QualityFaultTypeG")},
        {precitec::interface::QualityFaultTypeH, QT_TRANSLATE_NOOP3("", "Quality Fault Type H", "Precitec.Results.QualityFaultTypeH")},
        {precitec::interface::QualityFaultTypeI, QT_TRANSLATE_NOOP3("", "Quality Fault Type I", "Precitec.Results.QualityFaultTypeI")},
        {precitec::interface::QualityFaultTypeJ, QT_TRANSLATE_NOOP3("", "Quality Fault Type J", "Precitec.Results.QualityFaultTypeJ")},
        {precitec::interface::QualityFaultTypeK, QT_TRANSLATE_NOOP3("", "Quality Fault Type K", "Precitec.Results.QualityFaultTypeK")},
        {precitec::interface::QualityFaultTypeL, QT_TRANSLATE_NOOP3("", "Quality Fault Type L", "Precitec.Results.QualityFaultTypeL")},
        {precitec::interface::QualityFaultTypeM, QT_TRANSLATE_NOOP3("", "Quality Fault Type M", "Precitec.Results.QualityFaultTypeM")},
        {precitec::interface::QualityFaultTypeN, QT_TRANSLATE_NOOP3("", "Quality Fault Type N", "Precitec.Results.QualityFaultTypeN")},
        {precitec::interface::QualityFaultTypeO, QT_TRANSLATE_NOOP3("", "Quality Fault Type O", "Precitec.Results.QualityFaultTypeO")},
        {precitec::interface::QualityFaultTypeP, QT_TRANSLATE_NOOP3("", "Quality Fault Type P", "Precitec.Results.QualityFaultTypeP")},
        {precitec::interface::GeneralPurpose_DigitalOut1, QT_TRANSLATE_NOOP3("", "Digital Output 1", "Precitec.Results.GeneralPurpose_DigitalOut1")},
        {precitec::interface::FirstLineProfile, QT_TRANSLATE_NOOP3("", "Line profile 1", "Precitec.Results.FirstLineProfile")},
        {precitec::interface::LineProfile2, QT_TRANSLATE_NOOP3("", "Line profile 2", "Precitec.Results.LineProfile2")},
        {precitec::interface::LineProfile3, QT_TRANSLATE_NOOP3("", "Line profile 3", "Precitec.Results.LineProfile3")},
        {precitec::interface::GeneralPurpose_AnalogOut2, QT_TRANSLATE_NOOP3("", "Analogout 2", "Precitec.Results.GeneralPurpose_AnalogOut2")},
        {precitec::interface::GeneralPurpose_AnalogOut3, QT_TRANSLATE_NOOP3("", "Analogout 3", "Precitec.Results.GeneralPurpose_AnalogOut3")},
        {precitec::interface::GeneralPurpose_AnalogOut4, QT_TRANSLATE_NOOP3("", "Analogout 4", "Precitec.Results.GeneralPurpose_AnalogOut4")},
        {precitec::interface::GeneralPurpose_AnalogOut5, QT_TRANSLATE_NOOP3("", "Analogout 5", "Precitec.Results.GeneralPurpose_AnalogOut5")},
        {precitec::interface::GeneralPurpose_AnalogOut6, QT_TRANSLATE_NOOP3("", "Analogout 6", "Precitec.Results.GeneralPurpose_AnalogOut6")},
        {precitec::interface::GeneralPurpose_AnalogOut7, QT_TRANSLATE_NOOP3("", "Analogout 7", "Precitec.Results.GeneralPurpose_AnalogOut7")},
        {precitec::interface::GeneralPurpose_AnalogOut8, QT_TRANSLATE_NOOP3("", "Analogout 8", "Precitec.Results.GeneralPurpose_AnalogOut8")},
        {precitec::interface::ScanmasterScannerMoving, QT_TRANSLATE_NOOP3("", "Scanner Moving", "Precitec.Results.ScanmasterScannerMoving")},
        {precitec::interface::ScanmasterSeamWelding, QT_TRANSLATE_NOOP3("", "Scanner SeamWelding", "Precitec.Results.ScanmasterSeamWelding")},
        {precitec::interface::GeneralPurpose_DigitalOut2, QT_TRANSLATE_NOOP3("", "Digital Output 2", "Precitec.Results.GeneralPurpose_DigitalOut2")},
        {precitec::interface::GeneralPurpose_DigitalOut3, QT_TRANSLATE_NOOP3("", "Digital Output 3", "Precitec.Results.GeneralPurpose_DigitalOut3")},
        {precitec::interface::GeneralPurpose_DigitalOut4, QT_TRANSLATE_NOOP3("", "Digital Output 4", "Precitec.Results.GeneralPurpose_DigitalOut4")},
        {precitec::interface::GeneralPurpose_DigitalOut5, QT_TRANSLATE_NOOP3("", "Digital Output 5", "Precitec.Results.GeneralPurpose_DigitalOut5")},
        {precitec::interface::GeneralPurpose_DigitalOut6, QT_TRANSLATE_NOOP3("", "Digital Output 6", "Precitec.Results.GeneralPurpose_DigitalOut6")},
        {precitec::interface::GeneralPurpose_DigitalOut7, QT_TRANSLATE_NOOP3("", "Digital Output 7", "Precitec.Results.GeneralPurpose_DigitalOut7")},
        {precitec::interface::GeneralPurpose_DigitalOut8, QT_TRANSLATE_NOOP3("", "Digital Output 8", "Precitec.Results.GeneralPurpose_DigitalOut8")},
        {precitec::interface::EndOfSeamMarker, QT_TRANSLATE_NOOP3("", "End of seam marker", "Precitec.Results.EndOfSeamMarker")},
        {precitec::interface::ScanmasterSpotWelding, QT_TRANSLATE_NOOP3("", "Scanner SpotWelding", "Precitec.Results.ScanmasterSpotWelding")},
        {precitec::interface::PrepareContour, QT_TRANSLATE_NOOP3("", "Prepare contour", "Precitec.Results.PrepareContour")},
        {precitec::interface::WeldPreparedContour, QT_TRANSLATE_NOOP3("", "Weld prepared contour", "Precitec.Results.WeldPreparedContour")},
        {precitec::interface::ScanmasterHeight, QT_TRANSLATE_NOOP3("", "Height for compensation", "Precitec.Results.ScanmasterHeight")},
        {precitec::interface::LWMStandardResult, QT_TRANSLATE_NOOP3("", "LWM Standard Result", "Precitec.Results.LWMStandardResult")}
    };
    auto it = s_resultTypeNames.find(type);
    if (it == s_resultTypeNames.end())
    {
        return QString::number(type);
    } else
    {
        return QCoreApplication::instance()->translate("", it->second.first, it->second.second);
    }
}

QString nameForResult(const precitec::interface::ResultArgs &result)
{
    return nameForResult(result.resultType());
}

QString nameForResult(int type)
{
    return nameForResult(precitec::interface::ResultType(type));
}

QColor colorForResult(const precitec::interface::ResultType &type)
{
    static const std::map<precitec::interface::ResultType, QColor> s_defaultColors = {
        { precitec::interface::GapWidth, Qt::blue },
        { precitec::interface::GapPosition, Qt::cyan },
        { precitec::interface::Missmatch, {165, 42, 42} /* brown*/ },
        { precitec::interface::AxisPositionAbsolute, {95, 158, 160} /* CadetBlue*/ },
        { precitec::interface::AxisPositionRelative, {138, 43, 226} /*BlueViolet*/ },
        { precitec::interface::SeamWidth, Qt::darkCyan },
        { precitec::interface::KeyholePosition, {184, 134, 11} /*DarkGoldenrod*/ },
        { precitec::interface::Convexity, Qt::darkGreen },
        { precitec::interface::Roundness, Qt::darkMagenta },
        { precitec::interface::NoSeam, {85, 107, 47} /*DarkOliveGreen*/ },
        { precitec::interface::NoStructure, {255, 140, 0} /*DarkOrange*/ },
        { precitec::interface::SideBurn, {153, 50, 204} /*DarkOrchid*/ },
        { precitec::interface::Pores, Qt::darkRed },
        { precitec::interface::Holes, {233, 150, 122} /*DarkSalmon*/ },
        { precitec::interface::Spurt, {143, 188, 139} /*DarkSeaGreen*/ },
        { precitec::interface::Notch, {72, 61, 139} /*DarkSlateBlue*/ },
        { precitec::interface::EdgeSkew, {47, 79, 79} /*DarkSlateGray*/ },
        { precitec::interface::SeamThickness, {0, 206, 209} /*DarkTurquoise*/ },
        { precitec::interface::UnEqualLegs, {148, 0, 211} /*DarkViolet*/ },
        { precitec::interface::SeamLength, {255, 20, 147} /*DeepPink*/ },
        { precitec::interface::ReangeLength, {0, 191, 255} /*DeepSkyBlue*/ },
        { precitec::interface::Weldbed, {105, 105, 105} /*DimGray*/ },
        { precitec::interface::ControlOut, {30, 144, 255} /*DodgerBlue*/ },
        { precitec::interface::NoLaser, {178, 34, 34} /*Firebrick*/ },
        { precitec::interface::NoLight, {34, 139, 34} /*ForestGreen*/ },
        { precitec::interface::LaserPower, {128, 0, 0} /*Maroon*/ },
        { precitec::interface::Value, {102, 205, 170} /*MediumAquamarine*/ },
        { precitec::interface::CoordPosition, {0, 0, 205} /*MediumBlue*/ },
        { precitec::interface::CoordPositionX, {186, 85, 211} /*MediumOrchid*/ },
        { precitec::interface::CoordPositionY, {147, 112, 219} /*MediumPurple*/ },
        { precitec::interface::CoordPositionZ, {60, 179, 113} /*MediumSeaGreen*/ },
        { precitec::interface::LD50_Y_AbsolutePosition, {140, 255, 14} },
        { precitec::interface::LD50_Y_RelativePosition, {131, 246, 13} },
        { precitec::interface::LD50_Z_AbsolutePosition, {122, 229, 12} },
        { precitec::interface::LD50_Z_RelativePosition, {113, 212, 11} },
        { precitec::interface::ScanTracker_Amplitude, {104, 195, 10} },
        { precitec::interface::ScanTracker_Offset, {95, 178, 9} },
        { precitec::interface::LaserControl_PowerOffset, {86, 161, 8} },
        { precitec::interface::Z_Collimator_Position, {77, 144, 7} },
        { precitec::interface::Fieldbus_PositionResult, {68, 127, 6} },
        { precitec::interface::GeneralPurpose_AnalogOut1, {59, 110, 5} },
        { precitec::interface::GeneralPurpose_DigitalOut1, {50, 93, 4} },
        { precitec::interface::GeneralPurpose_AnalogOut2, {59, 110, 5} },
        { precitec::interface::GeneralPurpose_AnalogOut3, {59, 110, 5} },
        { precitec::interface::GeneralPurpose_AnalogOut4, {59, 110, 5} },
        { precitec::interface::GeneralPurpose_AnalogOut5, {59, 110, 5} },
        { precitec::interface::GeneralPurpose_AnalogOut6, {59, 110, 5} },
        { precitec::interface::GeneralPurpose_AnalogOut7, {59, 110, 5} },
        { precitec::interface::GeneralPurpose_AnalogOut8, {59, 110, 5} },
        { precitec::interface::ScanmasterSeamWelding, {159, 10, 5} },
        { precitec::interface::ScanmasterScannerMoving, {59, 10, 105} },
        { precitec::interface::ScanmasterSpotWelding, {59, 10, 150} },
        { precitec::interface::GeneralPurpose_DigitalOut2, {50, 93, 4} },
        { precitec::interface::GeneralPurpose_DigitalOut3, {50, 93, 4} },
        { precitec::interface::GeneralPurpose_DigitalOut4, {50, 93, 4} },
        { precitec::interface::GeneralPurpose_DigitalOut5, {50, 93, 4} },
        { precitec::interface::GeneralPurpose_DigitalOut6, {50, 93, 4} },
        { precitec::interface::GeneralPurpose_DigitalOut7, {50, 93, 4} },
        { precitec::interface::GeneralPurpose_DigitalOut8, {50, 93, 4} },
        { precitec::interface::EndOfSeamMarker, Qt::black }
    };
    auto colorIt = s_defaultColors.find(type);
    if (colorIt != s_defaultColors.end())
    {
        return colorIt->second;
    }
    return Qt::red;
}

QColor colorForResult(const precitec::interface::ResultArgs &result)
{
    return colorForResult(result.resultType());
}

QColor colorForResult(int type)
{
    return colorForResult(precitec::interface::ResultType(type));
}

QString nameForQualityError(int enumType)
{
    char cArray[24] = {'A','B','C','D','E','F','G','H','I','J','K','L','M','N','O','P','Q','R','S','T','U','V','W','X'};
    auto newString = QStringLiteral("Quality Fault Type ");
    if ((enumType >= precitec::interface::ResultType::QualityFaultTypeA) && (enumType <= precitec::interface::ResultType::QualityFaultTypeX))
    {
        return newString.append(QChar::fromLatin1(cArray[enumType - precitec::interface::ResultType::QualityFaultTypeA]));
    }
    if ((enumType >= precitec::interface::ResultType::QualityFaultTypeA_Cat2) && (enumType <= precitec::interface::ResultType::QualityFaultTypeX_Cat2))
    {
        return newString.append(QChar::fromLatin1(cArray[enumType - precitec::interface::ResultType::QualityFaultTypeA_Cat2])).append(QStringLiteral(" Category 2"));
    }
    if (enumType == precitec::interface::ResultType::FastStop_DoubleBlank)
    {
        return QStringLiteral("Fast Stop: Double Blank");
    }
    return QStringLiteral("");
}

}
