#include "hardwareParameters.h"
#include "common/systemConfiguration.h"

namespace precitec
{

using interface::SystemConfiguration;

namespace gui
{

HardwareParameters::HardwareParameters()
    : QObject()
    , m_keys {
        {Key::YAxisAbsolutePosition, {QT_TRANSLATE_NOOP("", "Start position"), {QByteArrayLiteral("FC69766B-E8C0-42CE-BC95-471EADBC66DB")}, UnitConversion::MilliFromMicro, Device::AxisY}},
        {Key::YAxisSoftwareLimits, {QT_TRANSLATE_NOOP("", "Software limits"), {QByteArrayLiteral("66B08DBB-5A0E-4EB5-9A6C-66FB80A3175E")}, UnitConversion::None, Device::AxisY}},
        {Key::YAxisUpperLimit, {QT_TRANSLATE_NOOP("", "Upper software limit"), {QByteArrayLiteral("0C56B220-868C-4323-B1F9-33D85DF3F1F4")}, UnitConversion::MilliFromMicro, Device::AxisY}},
        {Key::YAxisLowerLimit, {QT_TRANSLATE_NOOP("", "Lower software limit"), {QByteArrayLiteral("EE613A26-012F-4EEA-A908-C9B07352C960")}, UnitConversion::MilliFromMicro, Device::AxisY}},
        {Key::YAxisVelocity, {QT_TRANSLATE_NOOP("", "Velocity"), {QByteArrayLiteral("E7DBEE0B-EBBE-4973-B553-C14F7F89D5BC")}, UnitConversion::None, Device::AxisY}},
        {Key::YAxisAcceleration, {QT_TRANSLATE_NOOP("", "Acceleration"), {QByteArrayLiteral("B47C8607-F8A2-4E40-A456-A0ABAAE75813")}, UnitConversion::None, Device::AxisY}},
        {Key::LineLaser1OnOff, {QT_TRANSLATE_NOOP("", "Line Laser 1 enabled"), {QByteArrayLiteral("3C5B7C40-A328-45F1-A107-4DD038A4FFEB")}, UnitConversion::None, Device::LaserLine1}},
        {Key::LineLaser1Intensity, {QT_TRANSLATE_NOOP("", "Line Laser 1 intensity"), {QByteArrayLiteral("13D8D645-BC7B-410A-B511-344E4D3216D7")}, UnitConversion::None, Device::LaserLine1}},
        {Key::LineLaser2OnOff, {QT_TRANSLATE_NOOP("", "Line Laser 2 enabled"), {QByteArrayLiteral("2668D3F1-8BAF-4B6C-8889-1B93D8533E0E")}, UnitConversion::None, Device::LaserLine2}},
        {Key::LineLaser2Intensity, {QT_TRANSLATE_NOOP("", "Line Laser 2 intensity"), {QByteArrayLiteral("23AE1EF0-CA79-4A85-9101-3D3D8BAAD8F5")}, UnitConversion::None, Device::LaserLine2}},
        {Key::FieldLight1OnOff, {QT_TRANSLATE_NOOP("", "Line Laser 3 enabled"), {QByteArrayLiteral("D3ED251D-D175-4A74-A9E9-09633FC4052E")}, UnitConversion::None, Device::LaserLine3}},
        {Key::FieldLight1Intensity, {QT_TRANSLATE_NOOP("", "Line Laser 3 intensity"), {QByteArrayLiteral("C3444855-F910-4593-881E-61BB50BD41EF")}, UnitConversion::None, Device::LaserLine3}},
        {Key::ExposureTime, {QT_TRANSLATE_NOOP("", "Exposure time"), {QByteArrayLiteral("DB178308-E2FD-4FA7-B60C-42CDDADA0B85")}, UnitConversion::None, Device::Camera}},
        {Key::CameraRoiX, {QT_TRANSLATE_NOOP("", "Camera X"), {QByteArrayLiteral("FF97662F-6DB7-4B77-9140-7E1267F902F0")}, UnitConversion::None, Device::Camera}},
        {Key::CameraRoiY, {QT_TRANSLATE_NOOP("", "Camera Y"), {QByteArrayLiteral("DB4EAF88-9A9F-43AE-A6D8-BE2C865336FC")}, UnitConversion::None, Device::Camera}},
        {Key::CameraRoiWidth, {QT_TRANSLATE_NOOP("", "Camera width"), {QByteArrayLiteral("30ACD6F4-41AE-4900-8F69-4DD700609BD2")}, UnitConversion::None, Device::Camera}},
        {Key::CameraRoiHeight, {QT_TRANSLATE_NOOP("", "Camera height"), {QByteArrayLiteral("EEE02A8F-298A-4C27-B6EE-2982CCAAD1A3")}, UnitConversion::None, Device::Camera}},
        {Key::LEDFlashDelay, {QT_TRANSLATE_NOOP("", "LED Flash Delay"), {QByteArrayLiteral("9824AAC2-0849-4C90-89B8-43E558E782CC")}, UnitConversion::None, Device::LedIllumination}},
        {Key::LEDPanel1OnOff, {QT_TRANSLATE_NOOP("", "LED panel 1 enabled"), {QByteArrayLiteral("D988FF12-9E3C-4B80-BF81-82C6C7AE76BB")}, UnitConversion::None, Device::LedIllumination}},
        {Key::LEDPanel1Intensity, {QT_TRANSLATE_NOOP("", "LED panel 1 intensity"), {QByteArrayLiteral("F4FE85AC-94C6-4EFC-A780-6FCB7B7413CF")}, UnitConversion::None, Device::LedIllumination}},
        {Key::LEDPanel1PulseWidth, {QT_TRANSLATE_NOOP("", "LED panel 1 pulse width"), {QByteArrayLiteral("575DF2BA-C950-46A6-A4A9-04D3E943D679")}, UnitConversion::None, Device::LedIllumination}},
        {Key::LEDPanel2OnOff, {QT_TRANSLATE_NOOP("", "LED panel 2 enabled"), {QByteArrayLiteral("08AD00E5-B86A-439B-A257-7C37E99CCE0D")}, UnitConversion::None, Device::LedIllumination}},
        {Key::LEDPanel2Intensity, {QT_TRANSLATE_NOOP("", "LED panel 2 intensity"), {QByteArrayLiteral("CFC70EB5-31E8-4585-B307-5A3E92632240")}, UnitConversion::None, Device::LedIllumination}},
        {Key::LEDPanel2PulseWidth, {QT_TRANSLATE_NOOP("", "LED panel 2 pulse width"), {QByteArrayLiteral("01655048-FDAC-43DD-BBF5-36D65AF99F52")}, UnitConversion::None, Device::LedIllumination}},
        {Key::LEDPanel3OnOff, {QT_TRANSLATE_NOOP("", "LED panel 3 enabled"), {QByteArrayLiteral("5A2223A0-8EB7-4A02-A224-405771DC7B6E")}, UnitConversion::None, Device::LedIllumination}},
        {Key::LEDPanel3Intensity, {QT_TRANSLATE_NOOP("", "LED panel 3 intensity"), {QByteArrayLiteral("28E8F256-EAD5-472C-A0CB-00BE4558C3A1")}, UnitConversion::None, Device::LedIllumination}},
        {Key::LEDPanel3PulseWidth, {QT_TRANSLATE_NOOP("", "LED panel 3 pulse width"), {QByteArrayLiteral("75F74A37-AE25-4FE6-B117-9C2DAA3ADED3")}, UnitConversion::None, Device::LedIllumination}},
        {Key::LEDPanel4OnOff, {QT_TRANSLATE_NOOP("", "LED panel 4 enabled"), {QByteArrayLiteral("82CE05F5-EA23-48E0-8BA8-7FC23F1B0592")}, UnitConversion::None, Device::LedIllumination}},
        {Key::LEDPanel4Intensity, {QT_TRANSLATE_NOOP("", "LED panel 4 intensity"), {QByteArrayLiteral("6A7DCE0C-5C74-44FB-B7AC-395D975A138C")}, UnitConversion::None, Device::LedIllumination}},
        {Key::LEDPanel4PulseWidth, {QT_TRANSLATE_NOOP("", "LED panel 4 pulse width"), {QByteArrayLiteral("8241AFD5-C10E-4A30-BD92-AB0FA420832B")}, UnitConversion::None, Device::LedIllumination}},
        {Key::LEDPanel5OnOff, {QT_TRANSLATE_NOOP("", "LED panel 5 enabled"), {QByteArrayLiteral("2af13ae3-afe8-40c5-a965-17fcc16688a4")}, UnitConversion::None, Device::LedIllumination}},
        {Key::LEDPanel5Intensity, {QT_TRANSLATE_NOOP("", "LED panel 5 intensity"), {QByteArrayLiteral("9999a6e7-3cc8-400e-8fd9-1ac6e46f4dfa")}, UnitConversion::None, Device::LedIllumination}},
        {Key::LEDPanel5PulseWidth, {QT_TRANSLATE_NOOP("", "LED panel 5 pulse width"), {QByteArrayLiteral("220fb733-9806-4786-841d-40b87ae86906")}, UnitConversion::None, Device::LedIllumination}},
        {Key::LEDPanel6OnOff, {QT_TRANSLATE_NOOP("", "LED panel 6 enabled"), {QByteArrayLiteral("c0346188-f822-4464-9e48-ee6f431f7296")}, UnitConversion::None, Device::LedIllumination}},
        {Key::LEDPanel6Intensity, {QT_TRANSLATE_NOOP("", "LED panel 6 intensity"), {QByteArrayLiteral("ef5e0749-1b6b-4720-a093-93170e8ba56c")}, UnitConversion::None, Device::LedIllumination}},
        {Key::LEDPanel6PulseWidth, {QT_TRANSLATE_NOOP("", "LED panel 6 pulse width"), {QByteArrayLiteral("407aab66-e111-4a7b-92b7-5b3c7b092363")}, UnitConversion::None, Device::LedIllumination}},
        {Key::LEDPanel7OnOff, {QT_TRANSLATE_NOOP("", "LED panel 7 enabled"), {QByteArrayLiteral("fc3e4e89-893b-4ba1-90c7-9f7a8bf9acfc")}, UnitConversion::None, Device::LedIllumination}},
        {Key::LEDPanel7Intensity, {QT_TRANSLATE_NOOP("", "LED panel 7 intensity"), {QByteArrayLiteral("b11f4f1e-7e3f-4e6d-b58f-061dd27b4ff2")}, UnitConversion::None, Device::LedIllumination}},
        {Key::LEDPanel7PulseWidth, {QT_TRANSLATE_NOOP("", "LED panel 7 pulse width"), {QByteArrayLiteral("24c6ba71-687f-4c48-8f7f-b5348ae96ecd")}, UnitConversion::None, Device::LedIllumination}},
        {Key::LEDPanel8OnOff, {QT_TRANSLATE_NOOP("", "LED panel 8 enabled"), {QByteArrayLiteral("42072523-e15c-4158-aaf4-5acb53f3e822")}, UnitConversion::None, Device::LedIllumination}},
        {Key::LEDPanel8Intensity, {QT_TRANSLATE_NOOP("", "LED panel 8 intensity"), {QByteArrayLiteral("74a95dfe-5e76-4849-aed9-1a4e2e8012b3")}, UnitConversion::None, Device::LedIllumination}},
        {Key::LEDPanel8PulseWidth, {QT_TRANSLATE_NOOP("", "LED panel 8 pulse width"), {QByteArrayLiteral("52495c6b-8d64-4dd4-9ef4-0af2e06437fb")}, UnitConversion::None, Device::LedIllumination}},
        {Key::TrackerDriverOnOff, {QT_TRANSLATE_NOOP("", "ScanTracker: Switch on/off scan function"), {QByteArrayLiteral("70e7ef10-5462-47c2-979b-8fc706147349")}, UnitConversion::None, Device::ScanTracker}},
        {Key::ScanWidthOutOfGapWidth, {QT_TRANSLATE_NOOP("", "ScanTracker: Dynamic scan-width determined by graph"), {QByteArrayLiteral("9ad3c9b6-4bfa-42b9-bdee-408729418526")}, UnitConversion::None, Device::ScanTracker}},
        {Key::ScanPosOutOfGapPos, {QT_TRANSLATE_NOOP("", "ScanTracker: Dynamic scan-position determined by graph"), {QByteArrayLiteral("44dfce0b-ff2a-4b89-928b-a36c3735366e")}, UnitConversion::None, Device::ScanTracker}},
        {Key::ScanTrackerFrequencyContinuously, {QT_TRANSLATE_NOOP("", "ScanTracker: Frequency for scan function"), {QByteArrayLiteral("8b9129cf-3b59-4176-b252-5242e38eada0")}, UnitConversion::None, Device::ScanTracker}},
        {Key::ScanTrackerScanPosFixed, {QT_TRANSLATE_NOOP("", "ScanTracker: Fixed scan-position"), {QByteArrayLiteral("e38f7102-07ce-4915-ac2e-8bba878eda11")}, UnitConversion::None, Device::ScanTracker}},
        {Key::ScanTrackerScanWidthFixed, {QT_TRANSLATE_NOOP("", "ScanTracker: Fixed scan-width"), {QByteArrayLiteral("bca98bbf-404e-4260-97d3-97f1b4697d96")}, UnitConversion::None, Device::ScanTracker}},
        {Key::ClearEncoderCounter1, {QT_TRANSLATE_NOOP("", "Reset Encoder counter 1"), {QByteArrayLiteral("C2CE8555-E11C-4B83-A7FE-550920ACFFD4")}, UnitConversion::None, Device::Encoder1}},
        {Key::ClearEncoderCounter2, {QT_TRANSLATE_NOOP("", "Reset Encoder counter 2"), {QByteArrayLiteral("A534EB19-6E04-40C2-A299-880B44115378")}, UnitConversion::None, Device::Encoder2}},
        {Key::LWM40No1AmpPlasma, {QT_TRANSLATE_NOOP("", "LWM Plasma Amplification"), {QByteArrayLiteral("234FF086-4E5C-460E-BAC6-A3C9D532C07C")}, UnitConversion::None, Device::LWM}},
        {Key::LWM40No1AmpTemperature, {QT_TRANSLATE_NOOP("", "LWM Temperature Amplification"), {QByteArrayLiteral("D6D79422-E5DF-4454-9733-883A74F219B1")}, UnitConversion::None, Device::LWM}},
        {Key::LWM40No1AmpBackReflection, {QT_TRANSLATE_NOOP("", "LWM Back Reflection Amplification"), {QByteArrayLiteral("9CF69A54-65F1-40D4-B402-CF5A1824F6D7")}, UnitConversion::None, Device::LWM}},
        {Key::LWM40No1AmpAnalogInput, {QT_TRANSLATE_NOOP("", "LWM External Digital Input Amplification"), {QByteArrayLiteral("68FB5A6B-4FF9-4DCB-8246-92EE7004EAE5")}, UnitConversion::None, Device::LWM}},
        {Key::ScannerLaserPowerStatic, {QT_TRANSLATE_NOOP("", "Laser Power Static Center"), {QByteArrayLiteral("2862509f-6b2b-4827-831c-e1149fb52e86")}, UnitConversion::None, Device::ScanLabScanner}},
        {Key::SLDDimmerOnOff, {QT_TRANSLATE_NOOP("", "SLD Dimmer On/Off"), {QByteArrayLiteral("13B32EC3-ED25-4043-8C36-837AC04827EB")}, UnitConversion::None, Device::IDM}},
        {Key::IDMLampIntensity, {QT_TRANSLATE_NOOP("", "Lamp Intensity"), {QByteArrayLiteral("8E0993C9-5AEB-47E3-9E66-B6A73FF02743")}, UnitConversion::None, Device::IDM}},
        {Key::ScannerFileNumber, {QT_TRANSLATE_NOOP("", "Wobbel Figure File Number"), {QByteArrayLiteral("5A000DA3-C2D3-4F69-AAE9-C1921FDFD444")}, UnitConversion::None, Device::ScanLabScanner}},
        {Key::IsLaserPowerDigital, {QT_TRANSLATE_NOOP("", "Laser Power Digital"), {QByteArrayLiteral("E515AF6B-6BC6-4494-B8C2-9A2ABE00BFCD")}, UnitConversion::None, Device::ScanLabScanner}},
        {Key::LaserDelay, {QT_TRANSLATE_NOOP("", "LaserDelay"), {QByteArrayLiteral("458E73FF-AE6E-49B6-BA3F-9E8877145C67")}, UnitConversion::None, Device::ScanLabScanner}},
        {Key::ZCollimatorPositionAbsolute, {QT_TRANSLATE_NOOP("", "Z Collimator Absolute Position"), {QByteArrayLiteral("4B075FAE-3402-4C71-B245-8C085EECBDC0")}, UnitConversion::MilliFromMicro, Device::ZCollimator}},
        {Key::ScannerDriveToZero, {QT_TRANSLATE_NOOP("", "Scanner Drive To Zero"), {QByteArrayLiteral("DBCA2D41-68CE-4849-9C53-FA70489FD5F4")}, UnitConversion::None, Device::ScanLabScanner}},
        {Key::ScannerJumpSpeed, {QT_TRANSLATE_NOOP("", "Scanner Jump Speed"), {QByteArrayLiteral("DED8CFEC-7E89-4C0A-AE3D-A03F70C9E28A")}, UnitConversion::None, Device::ScanLabScanner}},
        {Key::LaserOnDelay, {QT_TRANSLATE_NOOP("", "Laser on delay"), {QByteArrayLiteral("5F479CF6-777F-4CEC-83C1-486B1A9B2407")}, UnitConversion::None, Device::ScanLabScanner}},
        {Key::LaserOffDelay, {QT_TRANSLATE_NOOP("", "Laser off delay"), {QByteArrayLiteral("E13815FB-DD58-46A8-8000-F7D3D693A539")}, UnitConversion::None, Device::ScanLabScanner}},
        {Key::IDMAdaptiveExposureBasicValue, {QT_TRANSLATE_NOOP("", "Adaptive Exposure Basic Value"), {QByteArrayLiteral("207FE332-3C5A-44A6-B1BF-11C2F2E76EFE")}, UnitConversion::None, Device::IDM}},
        {Key::IDMAdaptiveExposureModeOnOff, {QT_TRANSLATE_NOOP("", "Adaptive Exposure Mode On/Off"), {QByteArrayLiteral("A4FE4CFE-9093-4359-BA94-8E571A3CBE89")}, UnitConversion::None, Device::IDM}},
        {Key::ScannerNewXPosition, {QT_TRANSLATE_NOOP("", "Scanner X Position"), {QByteArrayLiteral("553C51F1-F66D-4B36-92B4-6E84D464F833")}, UnitConversion::None, Device::ScanLabScanner}},
        {Key::ScannerNewYPosition, {QT_TRANSLATE_NOOP("", "Scanner Y Position"), {QByteArrayLiteral("F8DAF95D-B164-4686-8BAB-02FA473FCA94")}, UnitConversion::None, Device::ScanLabScanner}},
        {Key::ScannerDriveToPosition, {QT_TRANSLATE_NOOP("", "Move Scanner to Position"), {QByteArrayLiteral("5C6D790E-3956-452F-825A-5D3EF1419E79")}, UnitConversion::None, Device::ScanLabScanner}},
        {Key::ScannerDriveWithOCTReference, {QT_TRANSLATE_NOOP("", "Move Scanner to Position With OCT Reference"), {QByteArrayLiteral("837fdee9-86d5-41d2-827f-1a6b0f5ece86")}, UnitConversion::None, Device::ScanLabScanner}},
        {Key::LinLogMode, {QT_TRANSLATE_NOOP("", "Lin Log: Mode"), {QByteArrayLiteral("11E8064E-CCAA-49DA-814E-6DD15B89BE21")}, UnitConversion::None, Device::Camera}},
        {Key::LinLogValue1, {QT_TRANSLATE_NOOP("", "Lin Log: Value 1"), {QByteArrayLiteral("E43DECF3-EB17-42AF-91ED-35354424CC9F")}, UnitConversion::None, Device::Camera}},
        {Key::LinLogValue2, {QT_TRANSLATE_NOOP("", "Lin Log: Value 2"), {QByteArrayLiteral("46C9FC2A-5FF6-411F-8510-16D7DD21CCB7")}, UnitConversion::None, Device::Camera}},
        {Key::LinLogTime1, {QT_TRANSLATE_NOOP("", "Lin Log: Time 1"), {QByteArrayLiteral("EC8E6ECD-B7BC-4E34-BF24-0BA4532650D5")}, UnitConversion::None, Device::Camera}},
        {Key::LinLogTime2, {QT_TRANSLATE_NOOP("", "Lin Log: Time 2"), {QByteArrayLiteral("91C51FD6-3C7F-49B5-943A-71FE504A0D8F")}, UnitConversion::None, Device::Camera}},
        {Key::LaserIsDualChannel, {QT_TRANSLATE_NOOP("", "Laser Dual Channel"), {QByteArrayLiteral("822D6803-9CD3-4F2C-943A-D40391844EBF")}, UnitConversion::None, Device::ScanLabScanner}},
        {Key::ScannerLaserPowerStaticRing, {QT_TRANSLATE_NOOP("", "Laser Power Static Ring"), {QByteArrayLiteral("87D6C0F4-1F1A-49FB-88BF-110EFDB43D2F")}, UnitConversion::None, Device::ScanLabScanner}},
        {Key::WobbleXSize, {QT_TRANSLATE_NOOP("", "Basic wobble figure horizontal size"), {QByteArrayLiteral("85F6BA10-9DC5-4B7F-A5BD-DC3FCFD1A07F")}, UnitConversion::None, Device::ScanLabScanner}},
        {Key::WobbleYSize, {QT_TRANSLATE_NOOP("", "Basic wobble figure vertical size"), {QByteArrayLiteral("0C1C82A0-5591-41CE-9529-710E3D6139CD")}, UnitConversion::None, Device::ScanLabScanner}},
        {Key::WobbleFrequency, {QT_TRANSLATE_NOOP("", "Basic wobble figure frequency"), {QByteArrayLiteral("0A97DA05-D9C6-482E-A8F6-022FB3B30A71")}, UnitConversion::None, Device::ScanLabScanner}},
        {Key::WobbleMode, {QT_TRANSLATE_NOOP("", "Wobble mode"), {QByteArrayLiteral("943B70CE-D43D-4275-B6B8-EF38164B5DAB")}, UnitConversion::None, Device::ScanLabScanner}},
        {Key::CameraAcquisitionMode, {QT_TRANSLATE_NOOP("", "Acquisition Mode (Continuous or SingleFrame)"), {QByteArrayLiteral("D07C54CB-F40B-4315-9867-D1FD8215669E")}, UnitConversion::None, Device::Camera}},
        {Key::CameraReuseLastImage, {QT_TRANSLATE_NOOP("", "Reuse last captured image (implies Acquisition Mode SingleFrame)"), {QByteArrayLiteral("78205AC3-6EEC-4262-A9CC-560A9DB2C4F6")}, UnitConversion::None, Device::Camera}},
        {Key::ScanTracker2DAngle, {QT_TRANSLATE_NOOP("", "Rotate figure"), {QByteArrayLiteral("ca12aeaa-fe98-43c9-81ec-4c3291aaf6c6")}, UnitConversion::None, Device::ScanLabScanner}},
        {Key::Scantracker2DLaserDelay, {QT_TRANSLATE_NOOP("", "Laser delay"), {QByteArrayLiteral("6c42bf43-e7d7-4065-9d9b-f16036909c7b")}, UnitConversion::None, Device::ScanLabScanner}},
        {Key::ScanTracker2DCustomFigure, {QT_TRANSLATE_NOOP("", "ScanTracker2D Custom Figure"), {QByteArrayLiteral("49cf4844-c664-4145-8a72-a2950277c1fd")}, UnitConversion::None, Device::ScanLabScanner}},
        {Key::ScanTracker2DScanWidthFixedX, {QT_TRANSLATE_NOOP("", "Scale in welding direction"), {QByteArrayLiteral("74cc5fca-7f73-4538-927b-ada54093db66")}, UnitConversion::None, Device::ScanLabScanner}},
        {Key::ScanTracker2DScanWidthFixedY, {QT_TRANSLATE_NOOP("", "Scale vertical to welding direction"), {QByteArrayLiteral("fb056e33-d080-4251-b17a-9a59fef5b0fd")}, UnitConversion::None, Device::ScanLabScanner}},
        {Key::ScanTracker2DScanPosFixedX, {QT_TRANSLATE_NOOP("", "ScanTracker2D: Fixed scan-position (X)"), {QByteArrayLiteral("020f8b29-e0c3-4922-b02f-438389619d51")}, UnitConversion::None, Device::ScanLabScanner}},
        {Key::ScanTracker2DScanPosFixedY, {QT_TRANSLATE_NOOP("", "ScanTracker2D: Fixed scan-position (Y)"), {QByteArrayLiteral("7144e9f8-166a-47bd-a40a-72c874878cbc")}, UnitConversion::None, Device::ScanLabScanner}},
        {Key::LiquidLensPosition, {QT_TRANSLATE_NOOP("", "Liquid Lens Position"), {QByteArrayLiteral("cb18e1f0-f32f-4ee6-8c70-ccba1e6c54da")}, UnitConversion::None, Device::ScanLabScanner}},
        {Key::ScannerCompensateHeight, {QT_TRANSLATE_NOOP("", "Scanner compensate height"), {QByteArrayLiteral("dcde505f-9a3b-4801-98e5-1472be10ef91")}, UnitConversion::None, Device::ScanLabScanner}},
        {Key::CompensationHeight, {QT_TRANSLATE_NOOP("", "Height for compensation"), {QByteArrayLiteral("17d03401-bf5a-4433-b8b4-7f056519f8b8")}, UnitConversion::None, Device::ScanLabScanner}},
        {Key::IsCompensationHeightFixed, {QT_TRANSLATE_NOOP("", "Use seam compensation height"), {QByteArrayLiteral("70b60163-061c-48c0-91c4-f40201fa3af6")}, UnitConversion::None, Device::ScanLabScanner}},
        {Key::CorrectionFileMode, {QT_TRANSLATE_NOOP("", "Set correction file"), {QByteArrayLiteral("7ced70f8-9882-4ec5-89f3-98d97a1f2d04")}, UnitConversion::None, Device::ScanLabScanner}},
        {Key::LWMProgramNumber, {QT_TRANSLATE_NOOP("", "LWM Program Number"), {QByteArrayLiteral("9c22b9f5-5d0f-401d-8cc6-7444ba7e8c38")}, UnitConversion::None, Device::ExternalLWM}},
        {Key::LWMInspectionActive, {QT_TRANSLATE_NOOP("", "LWM Inspection Active"), {QByteArrayLiteral("7cb8baa9-9f0a-470d-be91-e4d7a4d64576")}, UnitConversion::None, Device::ExternalLWM}}
    }
    , m_mutuallyExclusiveKeys {
        {Key::IDMAdaptiveExposureModeOnOff, Key::IDMLampIntensity}
    }
    , m_scanTracker2DMode{interface::SystemConfiguration::instance().get(interface::SystemConfiguration::IntKey::ScannerGeneralMode) == int(interface::ScannerGeneralMode::eScantracker2DMode)}
{
}

HardwareParameters::~HardwareParameters() = default;

HardwareParameters* HardwareParameters::instance()
{
    static HardwareParameters s_instance;
    return &s_instance;
}

HardwareParameters::Properties HardwareParameters::properties(Key key) const
{
    auto it = m_keys.find(key);
    if (it == m_keys.end())
    {
        return {std::string{""}, {}, UnitConversion::None, Device::InvalidDevice};
    }
    return it->second;
}

bool HardwareParameters::isLedKey(Key key) const
{
    if (key == Key::InvalidKey)
    {
        return false;
    }
    return m_keys.at(key).device == Device::LedIllumination;
}

bool HardwareParameters::isLedKey(const QUuid& uuid) const
{
    return isLedKey(key(uuid));
}

bool HardwareParameters::isScanTracker2DKey(Key key) const
{
    if (!m_scanTracker2DMode)
    {
        return false;
    }
    if (key == Key::InvalidKey)
    {
        return false;
    }
    static const std::vector<Key> s_keys{
        Key::ScannerFileNumber,
        Key::WobbleFrequency,
        Key::ScannerLaserPowerStatic,
        Key::ScanTracker2DAngle,
        Key::Scantracker2DLaserDelay,
        Key::ScanTracker2DCustomFigure,
        Key::ScanWidthOutOfGapWidth,
        Key::ScanPosOutOfGapPos,
        Key::ScanTracker2DScanWidthFixedX,
        Key::ScanTracker2DScanWidthFixedY,
        Key::ScanTracker2DScanPosFixedX,
        Key::ScanTracker2DScanPosFixedY,
    };

    return std::find(s_keys.begin(), s_keys.end(), key) != s_keys.end();
}

bool HardwareParameters::isScanTracker2DKey(const QUuid& uuid) const
{
    // shortcut to not search the keys
    if (!m_scanTracker2DMode)
    {
        return false;
    }
    return isScanTracker2DKey(key(uuid));
}

HardwareParameters::Key HardwareParameters::key(const QUuid& uuid) const
{
    const auto it = std::find_if(m_keys.begin(), m_keys.end(), [&uuid] (const auto& pair) { return pair.second.uuid == uuid; });
    if (it == m_keys.end())
    {
        return Key::InvalidKey;
    }

    return it->first;
}

std::string HardwareParameters::deviceKey(Device device) const
{
    switch(device)
    {
        case Device::AxisY:
            return std::string{"AxisYEnable"};
        case Device::LaserLine1:
            return std::string{"LineLaser1Enable"};
        case Device::LaserLine2:
            return std::string{"LineLaser2Enable"};
        case Device::LaserLine3:
            return std::string{"FieldLight1Enable"};
        case Device::Camera:
            return std::string{"HardwareCameraEnabled"};
        case Device::LedIllumination:
            return std::string{"LED_IlluminationEnable"};
        case Device::Encoder1:
            return std::string{"EncoderInput1Enable"};
        case Device::Encoder2:
            return std::string{"EncoderInput2Enable"};
        case Device::ScanTracker:
            return m_scanTracker2DMode ? SystemConfiguration::keyToString(SystemConfiguration::BooleanKey::Scanner2DEnable) : std::string{"ScanTrackerEnable"};
        case Device::LWM:
            return std::string{"LWM40_No1_Enable"};
        case Device::IDM:
            return std::string{"IDM_Device1Enable"};
        case Device::ScanLabScanner:
            return SystemConfiguration::keyToString(SystemConfiguration::BooleanKey::Scanner2DEnable);
        case Device::ZCollimator:
            return std::string{"ZCollimatorEnable"};
        case Device::LiquidLens:
            return std::string{"LiquidLensControllerEnable"};
        case Device::ExternalLWM:
            return std::string{"Communication_To_LWM_Device_Enable"};
        default:
            return std::string{""};
    }
}

}
}

