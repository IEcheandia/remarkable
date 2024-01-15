#pragma once
/**
 * The contour meta language contains data structures which are used to generate a list of commands for welding a contour under specific conditions.
 * A contour contains the position of a point and associated information like power and speed. The power and speed is set to the value when the scanner reaches the position.
 * A array with the meta language is transfered to an interpreter which generates the scanner specific commands depending on the selected scanner.
 * The five adjustable properties are:
 * Mark (Move with laser on from the current position to the configured one)
 * Jump (Move with laser off from the current position to the configured one)
 * LaserPower (Control the power of the laser)
 * MarkSpeed (Control the speed while moving with laser on)
 * JumpSpeed (Control the speed while moving with laser off)
 *
 * Generator is the contourGenerator class
 * Interpreter is the smartMoveInterpreter class
 **/
namespace precitec
{
namespace hardware
{
namespace contour
{

struct Command
{
    virtual ~Command() = default;
};

struct Initialize : Command
{
};

struct Position : Command
{
    virtual ~Position() = default;
    double x = 0.0;
    double y = 0.0;
};

struct Mark : Position
{
};

struct Jump : Position
{
};

struct LaserPower : Command
{
    double power = 0.0;
};

struct MarkSpeed : Command
{
    double speed = 1.0;
};

//TODO Ask SmartMove to implement jump speed as contour command too?
/*struct JumpSpeed : Command
{
    double speed = 1.0;
};*/

}
}
}
