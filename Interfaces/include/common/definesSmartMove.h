#pragma once

namespace precitec
{
namespace smartMove
{
/**
 *  TCP commands can be send to set a value or to get the current value.
 **/
enum TCPCommand : bool
{
    Get = false,
    Set = true,
};

/**
 * Status of sending a TCP command.
 **/
enum class TCPSendStatus : bool
{
    Failed = false,
    Successful = true,
};

/**
 * After initializing the scanner a return value is sent.
 * -1 means initialization failed
 *  0 means initialization ok
 **/
enum class InitializationError : int
{
    Failure = -1,
    Success = 0,
};

/**
 * After initializing the scanner a return value is sent.
 * -1 means initialization failed
 *  0 means initialization ok
 **/
enum class FileDescriptorError : int
{
    Failure = -1,
    Success = 0,
};

/**
 * After uploading a HPGL file (figure for marking) a return value is sent.
 * -1 means upload cancelled
 *  0 means upload ok
 * >1 means that HPGL errors detected
 * It doesn't matter which protocol (XModem or BTX) is used the return value is the same.
 **/
enum class HPGLUploadError : int
{
    Failure = -1,
    Success = 0,
    HPGLError = 1,
};

/**
 * After uploading a "Pen Parameter" file or a "Calibration" file a return value is sent.
 * It doesn't matter which protocol (XModem or BTX) is used the return value is the same.
 **/
enum class FileUploadError : int
{
    Failure = -1,
    Success = 0,
};

/**
 * Defines the different types of repetition of a job.
 **/
enum class JobRepeats : int
{
    Infinite = -1,
    Stop = 0,
    SingleShot = 1,
};

/**
* Operation modes
* Marking: The default operation mode. In this mode uploaded and pre-processed marking data can be used to performa a marking.
* Service: In this mode service and alignment tasks can be used.
**/
enum class SystemOperationModes
{
    Service = 0,
    Marking,
};

/**
* Input modes
* WaveformGenerator:    Can be used for generate a waveform.    //TODO Ask what this mode does.
* Calibration:          Can be used for calibration.            //TODO Ask what this mode does.
* Alignment:            Can be used for moving to a position. The movement can be with laser on or off.
**/
enum class InputModes
{
    WaveformGenerator = 0,
    Calibration,
    Alignment = 10,
};

/**
 * Driving to a position is done with discretization.
 * There is a different resolution for positive and negative fields of the scanfield.
 * This is just a blank enum to avoid unnecessary casting in the formula.
 * 16 Bits
 **/
enum DriveToLimits
{
    NegativeScanfield = -32768,
    OriginScanfield = 0,
    PositiveScanfield = 32767,
};

/**
 * The resolution for positioning is 24 bits.
 * Min is the negative scanfield limit
 * Max is the positive scanfield limit
 **/
enum ScanfieldBits
{
    Min = -8388607,
    Origin = 0,
    Max = 8388607,
};

/**
 * The jump and mark speed resolution is 16 bits.
 **/
enum SpeedLimitsBits
{
    MinSpeedBits = 1,
    MaxSpeedBits = 65536,
};

}
}
