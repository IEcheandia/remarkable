#pragma once

namespace precitec
{
namespace hardware
{

class SmartMoveCalculator
{
public:
    explicit SmartMoveCalculator();
    ~SmartMoveCalculator();

    int scanfieldSize() const
    {
        return m_scanfieldSize;
    }
    void setScanfieldSize(int quadraticScanfieldSizeMillimeter);

    int calculateDriveToBits(double millimeter);
    int calculateDriveToMillimeter(int bits);

    int calculateMillimeterToBits(double millimeter);                                       //[counts]
    double calculateBitsToMillimeter(int bits);                                             //[mm]

    double calculateBitsPerMillisecondsFromMeterPerSecond(double speedInMeterPerSecond) const;       //[counts/ms]
    double calculateMeterPerSecondFromBitsPerMilliseconds(int speedInBitsPerMilliseconds);  //[m/s]

    double calculateMillimeterFromMeter(double meter) const;                                //[mm]
    double calculateMeterFromMillimeter(double millimeter) const;                           //[m]
    double calculateMicrometerFromMillimeter(double millimeter) const;                      //[µm]
    double calculateMillimeterFromMicrometer(double micrometer) const;                      //[mm]
    double calculateNanometerFromMicrometer(double micrometer) const;                       //[nm]
    double calculateMicrometerFromNanometer(double nanometer) const;                        //[µm]

    int calculateResolutionInNanometer() const;                                             //[nm]

    double conversionFactor() const;                                                        //[counts/mm]

private:
    int m_scanfieldSize{50};                                                                //[mm]
};

}
}
