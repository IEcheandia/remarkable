#pragma once
#include "viWeldHead/Scanlab/AbstractScanner2DWelding.h"

#include <optional>

namespace precitec::hardware::welding
{
class Calculator;
class ZCompensation;
}

namespace precitec::interface
{
template<typename T>
class TResultArgs;
using ResultDoubleArray = TResultArgs<double>;
}

class AbstractFigureWelding : public AbstractScanner2DWelding
{
public:
    explicit AbstractFigureWelding(const precitec::hardware::Scanner2DWeldingData& control,
                                   precitec::hardware::welding::ZCompensation& zCompensation,
                                   std::unique_ptr<precitec::hardware::AbstractWeldingStrategy> weldingStrategy);

    virtual ~AbstractFigureWelding();

    int laserPowerDelayCompensation() const
    {
        return m_laserPowerDelayCompensation;
    }
    void setLaserPowerDelayCompensation(int powerDelayCompensation);
    int getADCValue()
    {
        return m_ADCValue;
    }
    void setADCValue(int ADCValue);
    bool setNominalPowers();
    double nominalPower() const;
    void setNominalPower(double newNominalPower);
    double nominalRingPower() const;
    void setNominalRingPower(double newNominalRingPower);

    void SetLaserDelay(double oValue) { m_oLaserDelay = oValue; };
    double GetLaserDelay(void) { return m_oLaserDelay; };
    void setHasDigitalLaserPower(bool value) { m_hasDigitalLaserPower = value; };
    double hasDigitalLaserPower() { return m_hasDigitalLaserPower; };

    void SetScannerWobbleXSize(double oValue) { m_oWobbelXSize = oValue; };
    double GetScannerWobbleXSize(void) { return m_oWobbelXSize; };
    void SetScannerWobbleYSize(double oValue) { m_oWobbelYSize = oValue; };
    double GetScannerWobbleYSize(void) { return m_oWobbelYSize; };
    void SetScannerWobbleRadius(double oValue) { m_oWobbelRadius = oValue; };
    double GetScannerWobbleRadius(void) { return m_oWobbelRadius; };
    void SetWobbleMode(precitec::WobbleMode oValue) { m_oWobbleMode = oValue; };
    precitec::WobbleMode GetWobbleMode(void) { return m_oWobbleMode; };

    std::optional<double> getActualXPosition() { return m_actualX; };
    std::optional<double> getActualYPosition() { return m_actualY; };

    virtual void setOnePortWobbleVector(std::size_t countWobbelVectors) = 0;
    virtual void setTwoPortWobbleVector(std::size_t countWobbelVectors) = 0;

    virtual void buildPreviewList(double newScannerXPosition,
                                  double newScannerYPosition,
                                  bool oFigureWobbleFileIsReady,
                                  double oJumpSpeedInBitsPerMs) = 0;

    virtual void prepareWeldingList(const precitec::interface::ResultDoubleArray& m_oWeldingData,
                                    std::size_t velocitiesSize,
                                    int m_oLaserPowerStatic,
                                    int m_oLaserPowerStaticRing,
                                    double m_oADCValue) = 0;
    virtual void loadWobbleFigureFile(const std::string& figureFile, int m_laserPowerDelayCompensation, double m_oADCValue) = 0;
    virtual void buildWeldingList(precitec::hardware::welding::Calculator& calculator,
                                  const precitec::interface::ResultDoubleArray& m_oWeldingData,
                                  const std::vector<double>& m_velocities) = 0;

    virtual void start_Mark(precitec::ScanlabOperationMode p_oOperationMode) = 0;
    virtual void saveLoggedScannerData() = 0;

    /**
     *  Pre position        [mm, mm]
     *      Pre position is the return value of the function.
     *  Laser delay         [ms]
     *  First point  (x,y)  [mm, mm]
     *  Second point (x,y)  [mm, mm]
     *  Mark speed          [Bits / ms]
     *      To get speed with unit [mm / ms] the mark speed is divided by calibration factor [Bits / ms].
     *  Intended length     [mm]
     *      Distance defined by laser deceleration that the scanner uses to accelerate to the selected welding speed.
     *      Acceleration prevents burin-in at the beginning of the figure.
     *      Orientation of the acceleration segment is aligned based on the first segment of the weld figure.
     *      The segment is defined by first and second point.
     **/
    std::pair<double, double> definePrePosition(double laserDelay, std::pair<double, double> firstPoint, std::pair<double, double> secondPoint);

protected:
    void defineOnePortWobbleFigure(precitec::WobbleControl wobbleControl);
    void defineTwoPortWobbleFigure(std::size_t countWobbelVectors);
    void calculateShiftAndShiftedNominalPower(precitec::WobbleControl wobbleControl);

    int m_ADCValue = 4095;

    std::vector<double> m_wobbelTransVector; //Right and left from the seam --> x
    std::vector<double> m_wobbelLongVector;  //With and against the seam --> y
    std::vector<double> m_deltaPower;
    std::vector<double> m_deltaRingPower;

    double m_nominalPower = 0.0;
    double m_nominalRingPower = 0.0;

    double m_oLaserDelay = 0.0;

    double m_oWobbelXSize;  // mm
    double m_oWobbelYSize;  // mm
    double m_oWobbelRadius; // mm
    precitec::WobbleMode m_oWobbleMode = precitec::WobbleMode::NoWobbling;

    std::reference_wrapper<precitec::hardware::welding::ZCompensation> m_zCompensation;

    bool m_hasDigitalLaserPower = false;

    std::optional<double> m_actualX;
    std::optional<double> m_actualY;

private:
    int m_shift = 0;
    int m_laserPowerDelayCompensation = 0;
};