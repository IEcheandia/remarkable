#pragma once

/**
 * In this file constants and enums are defined which are used by Scanlab and SmartMove external periphery.
 * The constants and enums address mostly global things like the fields of the result which is the same and so on.
 **/

namespace precitec
{
namespace weldingFigure
{

static const int SEAMWELDING_RESULT_FIELDS_PER_POINT = 5;           //X, Y, power, ring power, velocity
static const double JUMP_POWER_VALUE = 0.0;

enum class SeamWeldingResultFields
{
    X = 0,
    Y = 1,
    Power = 2,
    RingPower = 3,
    Velocity = 4,
};

enum class SeamWeldingResultSpecialCases
{
    TakeLastValid = -1,                                             //If first point has -1 then the power from the product is used
    TakeProduct = -2,                                          //Still not implemented!
};

struct Point
{
    double X = 0.0;
    double Y = 0.0;
    double laserPower = 0.0;
    double laserPowerRing = 0.0;
    double speed = 0.0;
};

struct SpecialValueInformation
{
    double currentPointValue = 0.0;
    double pointValueBefore = -1.0;
    double productValue = 0.0;
};

struct ProductValues
{
    double laserPower = 0.0;
    double laserPowerRing = 0.0;
    double velocity = 0.0;
};

}
}
