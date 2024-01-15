#pragma once

namespace precitec
{
static const int SEAMWELDING_RESULT_FIELDS_PER_POINT = 5; //x,y, power, ring power, velocity
static const int SCANMASTERWELDINGDATA_UNDEFINEDVALUE = -1;
static const int SCANMASTERWELDINGDATA_USESTATICVALUE = -2;

enum class WobbleMode {
    NoWobbling = -2,
    StandingEight = -1,
    Ellipse = 0,
    LyingEight = 1,
    Free = 2
};

enum class LoggerSignals {
    NoSignal = -1,
    LaserOn = 0,
    StatusAx = 1,               //A is first scanner
    StatusAy = 2,
    StatusBx = 4,               //B is second scanner
    StatusBy = 5,
    SampleX = 7,
    SampleY = 8,
    SampleZ = 9,
    SampleAx_Corr = 10,
    SampleAy_Corr = 11,
    SampleAz_Corr = 12,
    SampleBx_Corr = 13,
    SampleBy_Corr = 14,
    SampleBz_Corr = 15,
    StatusAx_LaserOn = 16,
    StatusAy_LaserOn = 17,
    StatusBx_LaserOn = 18,
    StatusBy_LaserOn = 19,
    SampleAx_Output = 20,        //Output means real output value for x-axis with offset and gain (and other stuff)
    SampleAy_Output = 21,
    SampleBx_Output = 22,
    SampleBy_Output = 23,
    LaserControl = 24,
    SampleAx_Transform = 25,
    SampleAy_Transform = 26,
    SampleAz_Transform = 27,
    SampleBx_Transform = 28,
    SampleBy_Transform = 29,
    SampleBz_Transform = 30,
    VectorControl = 31,
    FocusShift = 32,
    Analog_Out1 = 33,           //12 Bit
    Analog_Out2 = 34,
    Digital_Out16Bit = 35,
    Digital_Out8Bit = 36,
    PulseLenght = 37,
    HalfPeriod = 38,
    Free0 = 39,
    Free1 = 40,
    Free2 = 41,
    Free3 = 42,
    Encoder0 = 43,
    Encoder1 = 44,
    Mark_Speed = 45,
    DigitalIn_Extension1 = 46,
    Zoom = 47,
    Free4 = 48,
    Free5 = 49,
    Free6 = 50,
    Free7 = 51,
    TimeStamp = 52,
    WobbleAmplitude = 53,
    ReadAnalogIn = 54,
    ScaledEncoderX = 55,
    ScaledEncoderY = 56,
    ScaledEncoderZ = 57
};

enum class StatusSignals {
    NoSignal = -1,
    XY2_100_Statusword = 0,
    CurrentPosition = 1,
    TargetPosition = 2,
    PositionDifference = 3,
    CurrentCurrent = 4,
    RelativeGalvanometerscanner = 5,
    CurrentSpeed = 6,
    CurrentPositionZAxis = 7,
    TemperatureGalvanometerscanner = 8,
    TemperatureServocard = 9,
};

enum class WobbleControl {
    AnalogOut1Variation = 1,
    AnalogOut2Variation = 2,
    AnalogOut1And2Variation = 8
};

enum ScannerHead {
    ScannerHead1 = 1,
    ScannerHead2 = 2
};

enum Axis {
    Axis1 = 1,
    Axis2 = 2
};

enum class PositionDifferenceTolerance {
    MinTolerance = 1,
    DefaultTolerance = 183,
    MaxTolerance = 255,
    ToleranceCommand = 5376
};

enum class ScanlabOperationMode
{
    ScanlabWelding,
    ScanlabPreview,
};

enum class FigureFileType
{
    WobbleFigureType,
    WeldingFigureType,
};

enum class ScanlabCodes
{
    StatusWordCode = 0x0500,
    TemperatureCode = 0x0514,
    ServoCardTemperatureCode = 0x0515,
    StoppEventCode = 0x052A,
};

enum class CorrectionFileErrorCode
{
    Success = 0,
    FileCorrupt,
    Memory,
    FileNotFound,
    DSPMemory,
    PCIDownload,
    RTC6Driver,
    CorrectionFileNumber,
    Access,
    Option3DUnlocked,
    Busy,
    PCIUpload,
    Verify
};

enum ScanlabList
{
    ListOne = 1,
    ListTwo,
};

enum ScanlabOutputPorts
{
    AnalogOne = 1,
    AnalogTwo,
};

}
