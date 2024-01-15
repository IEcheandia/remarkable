#include "calibrationChangeEntry.h"

namespace precitec
{
namespace gui
{

CalibrationChangeEntry::CalibrationChangeEntry(CalibrationType type, QObject *parent)
    : components::userLog::Change(parent)
{
    switch(type)
    {
        case CalibrationType::Camera:
            setMessage(tr("Laser Line Calibration"));
            break;
        case CalibrationType::LED:
            setMessage(tr("LED Calibration"));
            break;
        case CalibrationType::IDM_Line:
            setMessage(tr("IDM Line Calibration"));
            break;
        case CalibrationType::IDM_TCP:
            setMessage(tr("IDM TCP Calibration"));
            break;
        case CalibrationType::IDM_DarkReference:
            setMessage(tr("IDM Dark Reference"));
            break;
        case CalibrationType::ScanfieldTarget:
            setMessage(tr("Scan Field Image Calibration"));
            break;
        case CalibrationType::ScanfieldImage:
            setMessage(tr("Scan Field Image Acquisition"));
            break;
        case CalibrationType::ScanfieldDepth:
            setMessage(tr("Scan Field Depth Calibration"));
            break;
        case CalibrationType::DepthImage:
            setMessage(tr("Scan Field Depth Acquisition"));
            break;
        case CalibrationType::Chessboard:
            setMessage(tr("Chessboard Grid Calibration"));
            break;
        case CalibrationType::LaserAngle:
            setMessage(tr("Laser Angle Calibration"));
            break;
        case CalibrationType::ScannerWelding:
            setMessage(tr("Scanner Welding pattern"));
            break;
        default:
            break;
    }
}

CalibrationChangeEntry::~CalibrationChangeEntry() = default;

QJsonObject CalibrationChangeEntry::data() const
{
    return {};
}

void CalibrationChangeEntry::initFromJson(const QJsonObject &data)
{
    Q_UNUSED(data)
}

}
}


