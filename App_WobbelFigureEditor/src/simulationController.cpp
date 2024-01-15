#include "simulationController.h"

#include <QVector2D>
#include <QVector3D>

#include "FileModel.h"
#include "laserPointController.h"

#include "figureEditorSettings.h"

#include "editorDataTypes.h"
#include "velocityLimits.h"

using precitec::scanmaster::components::wobbleFigureEditor::FigureEditorSettings;

using precitec::scanmaster::components::wobbleFigureEditor::velocityLimits::VelocityLimits;

namespace precitec
{
namespace scanmaster
{
namespace components
{
namespace wobbleFigureEditor
{

SimulationController::SimulationController(QObject* parent)
    : QObject(parent)
{
    connect(this, &SimulationController::simulationModeChanged, this, &SimulationController::clear);
    connect(this, &SimulationController::figureForSimulationChanged, this, &SimulationController::startSimulationFigure);
    connect(this, &SimulationController::tenMicroSecondsFactorChanged, this, &SimulationController::startSimulationFigure);
    connect(FigureEditorSettings::instance(), &FigureEditorSettings::scannerSpeedChanged, this, &SimulationController::startSimulationFigure);
    connect(this, &SimulationController::simulationFigureCalculated, this, &SimulationController::checkSimulatedFigureReady);
    connect(this, &SimulationController::loopCountChanged, this, &SimulationController::checkSimulatedFigureReady);
    connect(this, &SimulationController::simulationFigureCalculated, this, &SimulationController::pointCountSimulationFigureChanged);
    connect(this, &SimulationController::tenMicroSecondsFactorChanged, this, &SimulationController::pointCountSimulationFigureChanged);
    connect(this, &SimulationController::loopCountChanged, this, &SimulationController::pointCountSimulationFigureChanged);
}

SimulationController::~SimulationController() = default;

void SimulationController::setFileModel(precitec::scantracker::components::wobbleFigureEditor::FileModel* model)
{
    if (m_fileModel == model)
    {
        return;
    }

    disconnect(m_fileModelDestroyedConnection);
    m_fileModel = model;

    if (m_fileModel)
    {
        m_fileModelDestroyedConnection = connect(m_fileModel, &QObject::destroyed, this, std::bind(&SimulationController::setFileModel, this, nullptr));
    }
    else
    {
        m_fileModelDestroyedConnection = {};
    }
    emit fileModelChanged();
}

void SimulationController::setLaserPointController(precitec::scanmaster::components::wobbleFigureEditor::LaserPointController* newLaserPointController)
{
    if (m_laserPointController == newLaserPointController)
    {
        return;
    }

    if (m_laserPointController)
    {
        disconnect(this, &SimulationController::simulationModeChanged, m_laserPointController, &LaserPointController::clearFigure);
    }
    disconnect(m_laserPointControllerDestroyedConnection);
    m_laserPointController = newLaserPointController;

    if (m_laserPointController)
    {
        m_laserPointControllerDestroyedConnection = connect(m_laserPointController, &QObject::destroyed, this, std::bind(&SimulationController::setLaserPointController, this, nullptr));
        connect(this, &SimulationController::simulationModeChanged, m_laserPointController, &LaserPointController::clearFigure);
    }
    else
    {
        m_laserPointControllerDestroyedConnection = {};
    }
    emit laserPointControllerChanged();
}

void SimulationController::setSimulationMode(bool isSimulation)
{
    if (m_simulationMode == isSimulation)
    {
        return;
    }

    m_simulationMode = isSimulation;
    emit simulationModeChanged();
}

void SimulationController::setLoopCount(unsigned int newLoopCount)
{
    if (m_loopCount == newLoopCount)
    {
        return;
    }

    m_loopCount = newLoopCount;
    emit loopCountChanged();
}

void SimulationController::setTenMicroSecondsFactor(int newTenMicroSecondsFactor)
{
    if (m_tenMicroSecondsFactor == newTenMicroSecondsFactor)
    {
        return;
    }

    m_tenMicroSecondsFactor = newTenMicroSecondsFactor;
    emit tenMicroSecondsFactorChanged();
}

void SimulationController::setReady(bool isReady)
{
    if (m_ready == isReady)
    {
        return;
    }

    m_ready = isReady;
    emit readyChanged();
}

void SimulationController::getFigureForSimulation(const QString& filename, precitec::scantracker::components::wobbleFigureEditor::FileType type)
{
    if (!m_fileModel || filename.isEmpty())
    {
        return;
    }

    switch (type)
    {
    case FileType::Seam:
    {
        m_seamFigure = m_fileModel->loadSeamFigure(filename);
        emit figureForSimulationChanged();
        break;
    }
    case FileType::Wobble:
    {
        m_wobbleFigure = m_fileModel->loadWobbleFigure(filename);
        emit figureForSimulationChanged();
        break;
    }
    default:
        return;
    }
}

void SimulationController::showSimulatedFigure()
{
    if (!m_laserPointController || !m_ready)
    {
        return;
    }

    m_laserPointController->drawSimulatedFigure(m_simulatedSeamFigure, m_visualizationInformation);
    m_laserPointController->setPointsAreModifiable(false);
}

void SimulationController::clear()
{
    clearSimulationFigure();
    m_wobbleFigure = {};
    m_seamFigure = {};
    emit pointCountSimulationFigureChanged();
}

void SimulationController::startSimulationFigure()
{
    setReady(false);

    if (checkOneFigureMissing())
    {
        return;
    }

    calculateSimulationFigure();
}

bool SimulationController::checkOneFigureMissing()
{
    return m_seamFigure.figure.empty() || m_wobbleFigure.figure.empty();
}

void SimulationController::calculateSimulationFigure()
{
    clearSimulationFigure();
    m_focusSpeed.clear();

    if (m_seamFigure.figure.empty() || m_wobbleFigure.figure.empty())
    {
        return;
    }

    const auto& startPoint = m_seamFigure.figure.at(0).endPosition;
    const auto& seamVelocities = createSeamVelocities();
    const auto& wobbleVelocities = createWobbleVelocities();

    m_simulatedSeamFigure.figure.reserve(seamVelocities.size());
    RTC6::seamFigure::command::Order order;
    order.endPosition = std::make_pair(startPoint.first, startPoint.second);
    order.power = m_wobbleFigure.figure.front().power;
    m_simulatedSeamFigure.figure.push_back(order);

    for (std::size_t i = 0; i < seamVelocities.size(); i++)
    {
        auto seamPart = seamVelocities.at(i);
        auto wobblePart = wobbleVelocities.at(i % wobbleVelocities.size());

        auto seamAngle = angleRadFromXAxis(seamPart);
        auto wobblePartRotated = rotateVectorRad(wobblePart.toVector2D(), seamAngle);

        auto superImposedPart = seamPart + wobblePartRotated;

        order.endPosition = std::make_pair(
            m_simulatedSeamFigure.figure.back().endPosition.first + superImposedPart.x(),
            m_simulatedSeamFigure.figure.back().endPosition.second + superImposedPart.y());
        order.power += wobblePart.z();
        m_simulatedSeamFigure.figure.push_back(order);

        m_focusSpeed.push_back(calculateFocusSpeed(superImposedPart));
    }

    emit simulationFigureCalculated();
}

void SimulationController::clearSimulationFigure()
{
    m_simulatedSeamFigure = {};
    m_simulatedSeamFigure.name = std::string("Simulation");
    m_simulatedSeamFigure.ID = std::to_string(0);
    m_simulatedSeamFigure.description = std::string("Simulated seam superimposed with a wobble figure!");
}

double SimulationController::firstValidSpeed()
{
    return FigureEditorSettings::instance()->scannerSpeed();
}

std::vector<QVector2D> SimulationController::createSeamVelocities()
{
    std::vector<QVector2D> seamVelocities;
    seamVelocities.reserve(m_seamFigure.figure.size() * 500000);

    auto lastValidSpeed = 1.0;
    if (qFuzzyCompare(m_seamFigure.figure.at(0).velocity, static_cast<double>(VelocityLimits::Default)))
    {
        lastValidSpeed = firstValidSpeed();
    }

    for (std::size_t i = 1; i < m_seamFigure.figure.size(); i++)
    {
        const auto& lastPoint = m_seamFigure.figure.at(i - 1);
        const auto& currentPoint = m_seamFigure.figure.at(i);
        const auto& lastToCurrentPoint = QVector2D(currentPoint.endPosition.first - lastPoint.endPosition.first, currentPoint.endPosition.second - lastPoint.endPosition.second);
        const auto& lengthLastToCurrent = lastToCurrentPoint.length();
        auto partSpeed = 0.0;
        if (qFuzzyCompare(lastPoint.velocity, static_cast<double>(VelocityLimits::Default)))
        {
            partSpeed = lastValidSpeed;
        }
        else
        {
            partSpeed = lastPoint.velocity;
        }

        const auto& timeForSeamPart = lengthLastToCurrent / partSpeed;
        const auto& pointCount10us = qRound(timeForSeamPart / m_secondsIn10us);
        for (int i = 0; i < pointCount10us; i++)
        {
            seamVelocities.emplace_back(lastToCurrentPoint / pointCount10us);
        }
    }

    return seamVelocities;
}

std::vector<QVector3D> SimulationController::createWobbleVelocities()
{
    std::vector<QVector3D> wobbleVelocities;
    wobbleVelocities.reserve((m_wobbleFigure.figure.size() - 1) * m_wobbleFigure.microVectorFactor);

    for (std::size_t i = 1; i < m_wobbleFigure.figure.size(); i++)
    {
        const auto& lastPoint = m_wobbleFigure.figure.at(i - 1);
        const auto& currentPoint = m_wobbleFigure.figure.at(i);
        const auto& lastToCurrentPoint = QVector2D(currentPoint.endPosition.first - lastPoint.endPosition.first, currentPoint.endPosition.second - lastPoint.endPosition.second);
        const auto& microVectorFactor = static_cast<unsigned int>(m_wobbleFigure.microVectorFactor);
        auto relativePower = (currentPoint.power - lastPoint.power) / microVectorFactor;

        if (i == m_wobbleFigure.figure.size() - 1)
        {
            relativePower = 0.0;
        }

        for (std::size_t j = 0; j < microVectorFactor; j++)
        {
            wobbleVelocities.emplace_back(lastToCurrentPoint.x() / microVectorFactor, lastToCurrentPoint.y() / microVectorFactor, relativePower);
        }
    }

    return wobbleVelocities;
}

std::pair<unsigned int, int> SimulationController::calculateVisualizationInformation()
{
    if (m_loopCount != 0)
    {
        return std::make_pair((m_wobbleFigure.figure.size() * m_wobbleFigure.microVectorFactor * m_loopCount) - 1, m_tenMicroSecondsFactor);
    }
    return std::make_pair(0, m_tenMicroSecondsFactor);
}

bool SimulationController::checkTooManyPoints()
{
    return m_simulatedSeamFigure.figure.size() / m_tenMicroSecondsFactor >= m_pointCountVisualizationLimit;
}

void SimulationController::checkSimulatedFigureReady()
{
    m_visualizationInformation = calculateVisualizationInformation();

    if (m_simulatedSeamFigure.figure.empty() || m_visualizationInformation.first >= m_pointCountVisualizationLimit || (m_visualizationInformation.first == 0 && checkTooManyPoints()))
    {
        setReady(false);
        return;
    }

    setReady(true);
}

double SimulationController::calculateFocusSpeed(const QVector2D& velocity)
{
    return velocity.length() / m_secondsIn10us;
}

double SimulationController::angleRadFromXAxis(const QVector2D& vector)
{
    if (qFuzzyIsNull(vector.x())) //90°/270° --> vector.x() == 0
    {
        if (vector.y() > 0.0)
        {
            return M_PI * 0.5;
        }
        else if (vector.y() < 0.0)
        {
            return 3 * M_PI * 0.5;
        }
        else
        {
            return 0.0;
        }
    }

    auto angle = qAtan2(std::abs(vector.y()), std::abs(vector.x())); //Works for I quadrant of coordinate system (see complex numbers, especially angle of complex numbers)
    if (vector.x() < 0.0 && vector.y() > 0.0)                        //II quadrant
    {
        return M_PI - angle;
    }
    else if (vector.x() < 0.0 && vector.y() < 0.0) //III quadrant
    {
        return M_PI + angle;
    }
    else if (vector.x() > 0.0 && vector.y() < 0.0) //IV quadrant
    {
        return (2 * M_PI) - angle;
    }
    else if (qFuzzyIsNull(angle) && vector.x() < 0.0)
    {
        return M_PI;
    }
    return angle;
}

QVector2D SimulationController::rotateVectorRad(const QVector2D& vector, double angleRad)
{
    return QVector2D(qCos(angleRad) * vector.x() - qSin(angleRad) * vector.y(), qSin(angleRad) * vector.x() + qCos(angleRad) * vector.y());
}

}
}
}
}
