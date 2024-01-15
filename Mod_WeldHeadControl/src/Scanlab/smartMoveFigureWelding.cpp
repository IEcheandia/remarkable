#include "viWeldHead/Scanlab/smartMoveFigureWelding.h"
#include "viWeldHead/Scanlab/smartMoveWeldingStrategy.h"
#include "viWeldHead/Scanlab/smartMoveControl.h"

using precitec::weldingFigure::Point;
using precitec::weldingFigure::SpecialValueInformation;
using precitec::weldingFigure::SeamWeldingResultFields;
using precitec::weldingFigure::ProductValues;

namespace precitec
{
namespace hardware
{
namespace smartMove
{

FigureWelding::FigureWelding(Scanner2DWeldingData& control, welding::ZCompensation& zCompensation)
    : AbstractFigureWelding(control, zCompensation, std::make_unique<WeldingStrategy>(dynamic_cast<SmartMoveControl&>(control).networkInterface()))
    , m_control(dynamic_cast<SmartMoveControl&>(control))
{
}

FigureWelding::~FigureWelding() = default;

void FigureWelding::buildPreviewList(double newScannerXPosition,
                        double newScannerYPosition,
                        bool oFigureWobbleFileIsReady,
                        double oJumpSpeedInBitsPerMs)
{
    // TODO: what about the start position
    // TODO: what about wobble
    RTC6::Figure* weldingSeam = getFigure(FigureFileType::WeldingFigureType);

    std::vector<double> weldingData;
    weldingData.reserve(weldingSeam->figure.size() * SEAMWELDING_RESULT_FIELDS_PER_POINT);
    for (const auto& order: weldingSeam->figure)
    {
        weldingData.push_back(order.endPosition.first);
        weldingData.push_back(order.endPosition.second);
        weldingData.push_back(order.relativePower);
        weldingData.push_back(order.relativeRingPower);
        weldingData.push_back(order.velocity);
    }
    ProductValues defaultValuesProduct;
    defaultValuesProduct.laserPower = 0;
    defaultValuesProduct.laserPowerRing = 0;
    defaultValuesProduct.velocity = oJumpSpeedInBitsPerMs;

    m_control.prepareWeldingList(weldingData, defaultValuesProduct);
    m_control.buildPreviewList();
}

void FigureWelding::prepareWeldingList(const precitec::interface::ResultDoubleArray& m_oWeldingData,
                        std::size_t velocitiesSize,
                        int m_oLaserPowerStatic,
                        int m_oLaserPowerStaticRing,
                        double m_oADCValue)
{
    ProductValues defaultValuesProduct;
    defaultValuesProduct.laserPower = m_oLaserPowerStatic;
    defaultValuesProduct.laserPowerRing = m_oLaserPowerStaticRing;
    defaultValuesProduct.velocity = m_oMarkSpeed;
    m_control.prepareWeldingList(m_oWeldingData.value(), defaultValuesProduct);
}

void FigureWelding::loadWobbleFigureFile(const std::string& figureFile, int m_laserPowerDelayCompensation, double m_oADCValue)
{
    // TODO: implement
}

void FigureWelding::buildWeldingList(precitec::hardware::welding::Calculator& calculator,
                        const precitec::interface::ResultDoubleArray& m_oWeldingData,
                        const std::vector<double>& m_velocities)
{
    m_control.buildWeldingList();
}

void FigureWelding::start_Mark(precitec::ScanlabOperationMode p_oOperationMode)
{
    switch(p_oOperationMode)
    {
    case ScanlabOperationMode::ScanlabPreview:
        m_control.startJob(JobRepeats::Infinite);
        break;
    case ScanlabOperationMode::ScanlabWelding:
        m_control.startJob(JobRepeats::SingleShot);
        break;
    default:
        __builtin_unreachable();
        break;
    }
}

void FigureWelding::saveLoggedScannerData()
{
    // TODO: implement
}

void FigureWelding::setOnePortWobbleVector(std::size_t countWobbelVectors)
{
    // TODO: implement
}

void FigureWelding::setTwoPortWobbleVector(std::size_t countWobbelVectors)
{
    // TODO: implement
}

}
}
}
