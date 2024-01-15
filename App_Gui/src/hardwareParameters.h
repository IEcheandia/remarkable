#pragma once

#include <QObject>
#include <QUuid>

namespace precitec
{
namespace gui
{

/**
 * Singleton class providing access to all known hardware keys and their names and uuids
 **/

class HardwareParameters : public QObject
{
    Q_OBJECT

public:
    ~HardwareParameters() override;

    static HardwareParameters* instance();

    /**
     * The hardware parameters supported
     **/
    enum class Key {
        YAxisAbsolutePosition = 0,
        YAxisSoftwareLimits,
        YAxisLowerLimit,
        YAxisUpperLimit,
        YAxisVelocity,
        YAxisAcceleration,
        LineLaser1OnOff,
        LineLaser1Intensity,
        LineLaser2OnOff,
        LineLaser2Intensity,
        FieldLight1OnOff,
        FieldLight1Intensity,
        ExposureTime,
        CameraRoiX,
        CameraRoiY,
        CameraRoiWidth,
        CameraRoiHeight,
        LEDFlashDelay,
        LEDPanel1OnOff,
        LEDPanel1Intensity,
        LEDPanel1PulseWidth,
        LEDPanel2OnOff,
        LEDPanel2Intensity,
        LEDPanel2PulseWidth,
        LEDPanel3OnOff,
        LEDPanel3Intensity,
        LEDPanel3PulseWidth,
        LEDPanel4OnOff,
        LEDPanel4Intensity,
        LEDPanel4PulseWidth,
        LEDPanel5OnOff,
        LEDPanel5Intensity,
        LEDPanel5PulseWidth,
        LEDPanel6OnOff,
        LEDPanel6Intensity,
        LEDPanel6PulseWidth,
        LEDPanel7OnOff,
        LEDPanel7Intensity,
        LEDPanel7PulseWidth,
        LEDPanel8OnOff,
        LEDPanel8Intensity,
        LEDPanel8PulseWidth,
        TrackerDriverOnOff,
        ScanWidthOutOfGapWidth,
        ScanPosOutOfGapPos,
        ScanTrackerFrequencyContinuously,
        ScanTrackerScanPosFixed,
        ScanTrackerScanWidthFixed,
        ClearEncoderCounter1,
        ClearEncoderCounter2,
        LWM40No1AmpPlasma,
        LWM40No1AmpTemperature,
        LWM40No1AmpBackReflection,
        LWM40No1AmpAnalogInput,
        ScannerLaserPowerStatic,
        SLDDimmerOnOff,
        IDMLampIntensity,
        ScannerFileNumber,
        IsLaserPowerDigital,
        LaserDelay,
        ZCollimatorPositionAbsolute,
        ScannerDriveToZero,
        ScannerJumpSpeed,
        LaserOnDelay,
        LaserOffDelay,
        IDMAdaptiveExposureBasicValue,
        IDMAdaptiveExposureModeOnOff,
        ScannerNewXPosition,
        ScannerNewYPosition,
        ScannerDriveToPosition,
        LinLogMode,
        LinLogValue1,
        LinLogValue2,
        LinLogTime1,
        LinLogTime2,
        LaserIsDualChannel,
        ScannerLaserPowerStaticRing,
        WobbleXSize,
        WobbleYSize,
        WobbleFrequency,
        WobbleMode,
        CameraAcquisitionMode,
        CameraReuseLastImage,
        ScannerDriveWithOCTReference,
        ScanTracker2DAngle,
        Scantracker2DLaserDelay,
        ScanTracker2DCustomFigure,
        ScanTracker2DScanWidthFixedX,
        ScanTracker2DScanWidthFixedY,
        ScanTracker2DScanPosFixedX,
        ScanTracker2DScanPosFixedY,
        LiquidLensPosition,
        ScannerCompensateHeight,
        CompensationHeight,
        IsCompensationHeightFixed,
        CorrectionFileMode,
        LWMInspectionActive,
        LWMProgramNumber,
        InvalidKey // keep at end of enum
    };
    Q_ENUM(Key)

    /**
     * Conversion to be applied by the user interface to provide a better suited ui
     **/
    enum class UnitConversion {
        None,
        MilliFromMicro
    };

    enum class Device {
        AxisY,
        LaserLine1,
        LaserLine2,
        LaserLine3,
        Camera,
        LedIllumination,
        Encoder1,
        Encoder2,
        ScanTracker,
        LWM,
        IDM,
        ScanLabScanner,
        ZCollimator,
        LiquidLens,
        ExternalLWM,
        InvalidDevice
    };

    struct Properties {
        std::string name;
        QUuid uuid;
        HardwareParameters::UnitConversion conversion;
        Device device;
    };

    const std::map<Key, Properties>& keys() const
    {
        return m_keys;
    }

    Properties properties(Key key) const;

    bool isLedKey(Key key) const;
    bool isLedKey(const QUuid& uuid) const;

    bool isScanTracker2DKey(Key key) const;
    bool isScanTracker2DKey(const QUuid& uuid) const;

    int keyCount() const
    {
        return int(Key::InvalidKey);
    }

    Key key(const QUuid& uuid) const;

    const std::vector<std::vector<Key>>& mutuallyExclusiveKeys() const
    {
        return m_mutuallyExclusiveKeys;
    }

    std::string deviceKey(Device device) const;

private:
    explicit HardwareParameters();

    std::map<Key, Properties> m_keys;
    std::vector<Key> m_ledKyes;
    std::vector<std::vector<Key>> m_mutuallyExclusiveKeys;
    bool m_scanTracker2DMode;
};

}
}
