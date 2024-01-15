#pragma once

#include "common/definesSmartMove.h"

/**
 * The global meta language contains data structures which are used to generate global commands for a specific scanner.
 * Global commands are global functions or settings of the scanner. Global settings cannot be changed within a contour but can be changed between different contours.
 * To explain what global functions or settings are some examples:
 * - Moving fast to a specific position while the laser is off is a global function --> Can be done before the contour to take a photo or make a height measurement
 * - Delays like laser on/off, mark, jump and polygon delay are global settings     --> Can only be changed between contours
 *
 * Setting global properties mostly is achieved by one data structure.
 * Achieving a specific global function can also require a set or series of data structures because a function needs more than one command. (SmartMove jump to position)
 *
 * Generator is the globalCommandGenerator class
 * Interpreter is the smartMoveInterpreter class
**/

using precitec::smartMove::SystemOperationModes;
using precitec::smartMove::InputModes;
using precitec::smartMove::JobRepeats;

namespace precitec
{
namespace hardware
{

struct GlobalCommand
{
    virtual ~GlobalCommand() = default;
    bool set = true;
};

struct ProcessJob : GlobalCommand
{
    int repeats = 1;
};

struct JobSelect : GlobalCommand
{
    int jobID = 1;
};

struct SystemOperationMode : GlobalCommand
{
    static constexpr char command[] = "SYS_OPMODE ";
    SystemOperationModes sysOpMode = SystemOperationModes::Service;

    int value() const
    {
        return static_cast<int>(sysOpMode);
    }
};

struct InputMode : GlobalCommand
{
    static constexpr char command[] = "INPUT_MODE ";
    InputModes inputMode = InputModes::Alignment;

    int value() const
    {
        return static_cast<int>(inputMode);
    }
};

struct Align : GlobalCommand
{
    int x = 0;                                              //[Bits]
    int y = 0;                                              //[Bits]
};

struct AlignX : GlobalCommand
{
    static constexpr char command[] = "ALIGN_X ";
    AlignX(const std::shared_ptr<Align>& align)
        : align(align)
    {
        set = align->set;
    }

    int value() const
    {
        return align->x;
    }

private:
    std::shared_ptr<Align> align;
};

struct AlignY : GlobalCommand
{
    static constexpr char command[] = "ALIGN_Y ";
    AlignY(const std::shared_ptr<Align>& align)
        : align(align)
    {
        set = align->set;
    }

    int value() const
    {
        return align->y;
    }

private:
    std::shared_ptr<Align> align;
};

struct LaserOnForAlignment : GlobalCommand
{
    int laserOn = 0;                                        //[Ticks] every system clock tick is decremented by one
    int laserOnMax = 0;                                     //[Ticks] every system clock tick is decremented by one
};

struct LaserOn : GlobalCommand
{
    static constexpr char command[] = "ALIGN_LASERON ";
    LaserOn(std::shared_ptr<LaserOnForAlignment> laserOn)
        : laserOn(laserOn)
    {
        set = laserOn->set;
    }

    int value() const
    {
        return laserOn->laserOn;
    }
private:
    std::shared_ptr<LaserOnForAlignment> laserOn;
};

struct LaserOnMax : GlobalCommand
{
    static constexpr char command[] = "ALIGN_LASERON_MAX ";
    LaserOnMax(std::shared_ptr<LaserOnForAlignment> laserOn)
        : laserOn(laserOn)
    {
        set = laserOn->set;
    }

    int value() const
    {
        return laserOn->laserOnMax;
    }
private:
    std::shared_ptr<LaserOnForAlignment> laserOn;
};

struct CalibrationFilename : GlobalCommand
{
};

struct FocalLength : GlobalCommand
{
    static constexpr char command[] = "SFC_FOCALLEN ";
    double focalLength = 1.0;                               //[m]

    double value() const
    {
        return focalLength;
    }
};

struct ScanfieldSize : GlobalCommand
{
    static constexpr char command[] = "FIELD_SIZE ";
    double scanfieldSize = 1.0;                             //[m]

    double value() const
    {
        return scanfieldSize;
    }
};

struct JumpSpeed : GlobalCommand
{
    static constexpr char command[] = "MV_JUMPSPEED ";
    double speed = 1.0;                                     //[m/s]

    double value() const
    {
        return speed;
    }
};

struct MarkSpeed : GlobalCommand
{
    static constexpr char command[] = "MV_MARKSPEED ";
    double speed = 1.0;                                     //[m/s]

    double value() const
    {
        return speed;
    }
};

struct PositionFeedbackBits : GlobalCommand
{
};

struct PositionCommandBits : GlobalCommand
{
};

struct ForceEnable : GlobalCommand
{
};

struct SysTs : GlobalCommand
{
};

struct LaserPower : GlobalCommand
{
    static constexpr char command[] = "MV_PENWIDTH ";
    double center{0.0};
    double ring{0.0};

    double value() const
    {
        return center;
    }
};

}
}
