#pragma once

#include "viWeldHead/Scanlab/AbstractFigureWelding.h"
#include "common/definesScanlab.h"

#include <vector>

class ScanlabLoggerTest;
class LoadWobbleFigureTest;
class PrePositionTest;

namespace RTC6
{
class FigureWelding : public AbstractFigureWelding
{
public:
    explicit FigureWelding(const precitec::hardware::Scanner2DWeldingData& control,
                               precitec::hardware::welding::ZCompensation& zCompensation);
    ~FigureWelding();

    struct lastPointProperties
    {
        bool firstTime;
        unsigned int power;
        unsigned int ringPower;
        double velocity;
    };

    enum class WobbleReturn
    {
        NoWobbling = 0,
        BasicWobbling,
        InvalidWobbleFigure,
        OnePortWobbling,
        TooManySegments,
        InvalidMicroVectorFactor,
        TwoPortWobbling
    };

    void buildPreviewList(double newScannerXPosition,
                          double newScannerYPosition,
                          bool oFigureWobbleFileIsReady,
                          double oJumpSpeedInBitsPerMs) override;

    void prepareWeldingList(const precitec::interface::ResultDoubleArray& m_oWeldingData,
                            std::size_t velocitiesSize,
                            int m_oLaserPowerStatic,
                            int m_oLaserPowerStaticRing,
                            double m_oADCValue) override;

    void loadWobbleFigureFile(const std::string& figureFile, int m_laserPowerDelayCompensation, double m_oADCValue) override;

    void buildWeldingList(precitec::hardware::welding::Calculator& calculator,
                          const precitec::interface::ResultDoubleArray& m_oWeldingData,
                          const std::vector<double>& m_velocities) override;

    void start_Mark(precitec::ScanlabOperationMode p_oOperationMode) override;

    void saveLoggedScannerData() override;

private:
    void define_MiniSeamInit(precitec::ScanlabOperationMode p_oOperationMode);
    void define_MiniSeamInitDigital(long unsigned int program);
    FigureWelding::WobbleReturn define_WobbleFigure(precitec::WobbleMode mode, precitec::ScanlabOperationMode operationMode = precitec::ScanlabOperationMode::ScanlabWelding);
    void define_MiniSeamStart(long p_oXStart, long p_oYStart);
    void define_MiniSeamJump(long p_oXJump, long p_oYJump);
    void define_MiniSeamLineDigital(long p_oXEnd, long p_oYEnd, double velocity, bool firstTime = false);
    void define_MiniSeamLineDual(long p_oXEnd, long p_oYEnd, unsigned long laserPowerCenter, double velocity, bool wobbleMode, precitec::ScanlabOperationMode p_oOperationMode);
    void define_MiniSeamLine(long p_oXEnd, long p_oYEnd, unsigned long laserPowerCenter, unsigned long laserPowerRing, double velocity, precitec::ScanlabOperationMode p_oOperationMode);
    void define_MiniSeamEnd(precitec::ScanlabOperationMode p_oOperationMode);
    void define_MiniSeamEndDigital();

    void setOnePortWobbleVector(std::size_t countWobbelVectors) override;
    void setTwoPortWobbleVector(std::size_t countWobbelVectors) override;

    bool saveDataToFile(const std::string& dir);
    void deactivateWobbleMode();
    void setPowerModulationSettings(precitec::WobbleControl wobbleControl);
    void resetLastPointProperties();

    unsigned long int convertPowerToBits(double power);

    void buildAnalogList(precitec::hardware::welding::Calculator& calculator,
                         const precitec::interface::ResultDoubleArray& m_oWeldingData,
                         const std::vector<double>& m_velocities);

    void buildDigitalList(precitec::hardware::welding::Calculator& calculator,
                          const precitec::interface::ResultDoubleArray& m_oWeldingData,
                          const std::vector<double>& m_velocities);

    std::array<std::vector<int>, 4> m_measurementSignals;

    lastPointProperties m_lastProperties;

    std::vector<long unsigned int> laserPowerValues;
    std::vector<long unsigned int> laserPowerRingValues;

    friend ScanlabLoggerTest;
    friend LoadWobbleFigureTest;
    friend PrePositionTest;
};
}