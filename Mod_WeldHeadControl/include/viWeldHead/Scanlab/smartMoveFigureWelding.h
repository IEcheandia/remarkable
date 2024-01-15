#pragma once
#include "viWeldHead/Scanlab/AbstractFigureWelding.h"
#include "common/definesWeldingFigure.h"

namespace precitec
{
namespace hardware
{
class SmartMoveControl;
namespace smartMove
{

class FigureWelding : public AbstractFigureWelding
{
public:
    explicit FigureWelding(Scanner2DWeldingData& control, welding::ZCompensation& zCompensation);
    ~FigureWelding();

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

    void setOnePortWobbleVector(std::size_t countWobbelVectors) override;
    void setTwoPortWobbleVector(std::size_t countWobbelVectors) override;

private:
    SmartMoveControl& m_control;

};

}
}
}
