#pragma once
namespace precitec::hardware
{

struct LaserDelays
{
    long int on;
    long int off;
};

struct AnalogOutput
{
    unsigned int value;
    unsigned int min;
    unsigned int max;
};

struct ScannerGeometricalTransformationData
{
    long int x;
    long int y;
    double angle;
};

class Scanner2DWeldingData
{
public:
    virtual ~Scanner2DWeldingData() = default;
    [[nodiscard]] virtual LaserDelays getLaserDelays() const = 0;
    [[nodiscard]] virtual AnalogOutput getAnalogOutput() const = 0;
    [[nodiscard]] virtual ScannerGeometricalTransformationData getScannerGeometricalTransformationData() const = 0;
};

}