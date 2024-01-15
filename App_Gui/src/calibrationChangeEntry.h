#pragma once

#include <precitec/change.h>
#include "permissions.h"

namespace precitec
{
namespace gui
{

enum class CalibrationType {
    Camera,
    LED,
    IDM_Line,
    IDM_TCP,
    IDM_DarkReference,
    ScanfieldTarget,
    ScanfieldImage,
    ScanfieldDepth,
    DepthImage,
    Chessboard,
    LaserAngle,
    ScannerWelding,
    ScannerCalibrationMeasure,
    CameraCalibration,
};

class CalibrationChangeEntry : public components::userLog::Change
{
    Q_OBJECT
public:
    CalibrationChangeEntry(CalibrationType type, QObject *parent = nullptr);
    ~CalibrationChangeEntry() override;

protected:
    QJsonObject data() const override;
    void initFromJson(const QJsonObject &data) override;
};

}
}
