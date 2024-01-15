#include "FigureAnalyzer.h"
#include "WobbleFigureEditor.h"
#include "simulationController.h"
#include "figureEditorSettings.h"

using precitec::scanmaster::components::wobbleFigureEditor::FigureEditorSettings;
using precitec::scanmaster::components::wobbleFigureEditor::SimulationController;

namespace precitec
{
namespace scantracker
{
namespace components
{
namespace wobbleFigureEditor
{

FigureAnalyzer::FigureAnalyzer(QObject* parent) : QObject(parent)
{
    connect(this, &FigureAnalyzer::reset, this, &FigureAnalyzer::resetInformation);
    connect(this, &FigureAnalyzer::fileChanged, this, &FigureAnalyzer::updateProperties);
    connect(FigureEditorSettings::instance(), &FigureEditorSettings::scannerSpeedChanged, this, &FigureAnalyzer::updateProperties);
}

FigureAnalyzer::~FigureAnalyzer() = default;

void FigureAnalyzer::setFigureEditor(WobbleFigureEditor* newFigureEditor)
{
    if (m_figureEditor == newFigureEditor)
    {
        return;
    }

    if (m_figureEditor)
    {
        disconnect(m_figureEditor, &WobbleFigureEditor::fileChanged, this, &FigureAnalyzer::fileChanged);
        disconnect(m_figureEditor, &WobbleFigureEditor::figureCleared, this, &FigureAnalyzer::reset);
        disconnect(m_figureEditor, &WobbleFigureEditor::microVectorFactorChanged, this, &FigureAnalyzer::fileChanged);
        disconnect(m_figureEditor, &WobbleFigureEditor::nodePositionUpdated, this, &FigureAnalyzer::updateProperties);
    }

    disconnect(m_figureEditorDestroyedConnection);
    m_figureEditor = newFigureEditor;

    if (m_figureEditor)
    {
        m_figureEditorDestroyedConnection = connect(m_figureEditor, &QObject::destroyed, this, std::bind(&FigureAnalyzer::setFigureEditor, this, nullptr));
        connect(m_figureEditor, &WobbleFigureEditor::fileChanged, this, &FigureAnalyzer::fileChanged);
        connect(m_figureEditor, &WobbleFigureEditor::figureCleared, this, &FigureAnalyzer::reset);
        connect(m_figureEditor, &WobbleFigureEditor::microVectorFactorChanged, this, &FigureAnalyzer::fileChanged);
        connect(m_figureEditor, &WobbleFigureEditor::nodePositionUpdated, this, &FigureAnalyzer::updateProperties);
    }
    else
    {
        m_figureEditorDestroyedConnection = {};
    }

    emit figureEditorChanged();
}

void FigureAnalyzer::setSimulationController(SimulationController* newSimulationController)
{
    if (m_simulationController == newSimulationController)
    {
        return;
    }

    if (m_simulationController)
    {
        disconnect(m_simulationController, &SimulationController::readyChanged, this, &FigureAnalyzer::updateSimulationProperties);
        disconnect(m_simulationController, &SimulationController::simulationModeChanged, this, &FigureAnalyzer::resetInformation);
        disconnect(m_simulationController, &SimulationController::loopCountChanged, this, &FigureAnalyzer::updateSimulationProperties);
    }

    disconnect(m_simulationControllerDestroyedConnection);
    m_simulationController = newSimulationController;

    if (m_simulationController)
    {
        m_simulationControllerDestroyedConnection = connect(m_simulationController, &QObject::destroyed, this, std::bind(&FigureAnalyzer::setSimulationController, this, nullptr));
        connect(m_simulationController, &SimulationController::readyChanged, this, &FigureAnalyzer::updateSimulationProperties);
        connect(m_simulationController, &SimulationController::simulationModeChanged, this, &FigureAnalyzer::resetInformation);
        connect(m_simulationController, &SimulationController::loopCountChanged, this, &FigureAnalyzer::updateSimulationProperties);
    }
    else
    {
        m_simulationControllerDestroyedConnection = {};
    }

    emit simulationControllerChanged();
}

void FigureAnalyzer::updateProperties()
{
    emit reset();

    if (!m_figureEditor)
    {
        return;
    }
    auto worked = getInformationFromFigureEditor();
    if (!worked)
    {
        return;
    }

    updateStartPoint();
    updateEndPoint();
    updatePointCount();
    updateRouteProperties();
    updateFigureDimensions();
}

void FigureAnalyzer::resetInformation()
{
    m_pointCount = 0;
    emit pointCountChanged();
    m_routeLength = 0.0;
    emit routeLengthChanged();
    m_startPoint = {0.0, 0.0};
    emit startPointChanged();
    m_endPoint = {0.0, 0.0};
    emit endPointChanged();
    m_figureWidth = 0.0;
    emit figureWidthChanged();
    m_figureHeight = 0.0;
    emit figureHeightChanged();
    m_figureTime = 0.0;
    emit figureTimeChanged();
    m_wobblePointDistance = 0.0;
    emit wobblePointDistanceChanged();
    m_isWobbleFigure = false;
    m_wobbleFigurePointSize = 0;
    m_microVectorFactor = 0;
}

void FigureAnalyzer::updateSimulationProperties()
{
    emit reset();

    if (!m_simulationController || !m_simulationController->ready() || !getInformationFromSimulationController())
    {
        return;
    }

    updateStartPoint();
    updateEndPoint();
    updatePointCount();
    updateRouteProperties();
    updateFigureDimensions();
    updateWobblePointDistance();
    updateFocusSpeed();
}

bool FigureAnalyzer::getInformationFromFigureEditor()
{
    if (!m_figureEditor)
    {
        return false;
    }

    switch (m_figureEditor->fileType())
    {
        case FileType::Seam:
        {
            const auto &seamFigure = m_figureEditor->getSeamFigure()->figure;
            if (seamFigure.empty())
            {
                return false;
            }
            castSeamToRouteInformation(seamFigure);
            break;
        }
        case FileType::Wobble:
        {
            const auto &figure = m_figureEditor->wobbleFigure();
            const auto &wobbleFigure = figure.figure;
            m_microVectorFactor = figure.microVectorFactor;
            if (wobbleFigure.empty())
            {
                return false;
            }
            castWobbleToRouteInformation(wobbleFigure);
            m_isWobbleFigure = true;
            break;
        }
        case FileType::Overlay:
        {
            const auto &overlayFigure = m_figureEditor->overlay().functionValues;
            if (overlayFigure.empty())
            {
                return false;
            }
            castOverlayToRouteInformation(overlayFigure);
            break;
        }
        default:
            return false;
    }

    return true;
}

void FigureAnalyzer::castSeamToRouteInformation(const std::vector<RTC6::seamFigure::command::Order>& figure)
{
    m_routeInformation.clear();
    m_routeInformation.reserve(figure.size());
    for (const auto &order : figure)
    {
        m_routeInformation.emplace_back(FigureInformation{order.endPosition, order.velocity});
    }
    updateRouteProperties();
    updatePointCount();
}

void FigureAnalyzer::castWobbleToRouteInformation(const std::vector<RTC6::wobbleFigure::command::Order>& figure)
{
    m_routeInformation.clear();
    m_routeInformation.reserve(figure.size());
    for (const auto &order : figure)
    {
        m_routeInformation.emplace_back(FigureInformation{order.endPosition, 0.0});
    }
}

void FigureAnalyzer::castOverlayToRouteInformation(const std::vector<std::pair<double, double>>& figure)
{
    m_routeInformation.clear();
    m_routeInformation.reserve(figure.size());
    for (const auto &order : figure)
    {
        m_routeInformation.emplace_back(FigureInformation{order, 0.0});
    }
}

bool FigureAnalyzer::getInformationFromSimulationController()
{
    if (!m_simulationController)
    {
        return false;
    }

    auto simulatedFigure = m_simulationController->simulatedSeamFigure();
    m_wobbleFigurePointSize = m_simulationController->wobbleFigurePointSize();
    m_microVectorFactor = m_simulationController->microVectorFactor();
    m_wobbleFigurePointCount = m_simulationController->wobbleFigurePointCount();
    if (m_simulationController->loopCount() != 0)
    {
        std::vector<RTC6::seamFigure::command::Order> copySimulatedFigure = simulatedFigure;
        simulatedFigure.clear();
        for (int i = 0; i < static_cast<int>(m_simulationController->pointCountSimulationFigure() * m_simulationController->tenMicroSecondsFactor()); i++)
        {
            if (i % m_simulationController->tenMicroSecondsFactor() == 0)
            {
                simulatedFigure.push_back(copySimulatedFigure.at(i));
            }
        }
    }

    if (simulatedFigure.empty() || m_wobbleFigurePointSize == 0)
    {
        return false;
    }

    castSeamToRouteInformation(simulatedFigure);

    return !m_routeInformation.empty();
}

void FigureAnalyzer::updatePointCount()
{
    if (m_pointCount == m_routeInformation.size())
    {
        return;
    }

    m_pointCount = m_routeInformation.size();
    emit pointCountChanged();
}

void FigureAnalyzer::updateStartPoint()
{
    if (m_routeInformation.empty())
    {
        return;
    }

    m_startPoint = QPointF{m_routeInformation.front().coordinates.first, m_routeInformation.front().coordinates.second};
    emit startPointChanged();
}

void FigureAnalyzer::updateEndPoint()
{
    if (m_routeInformation.empty())
    {
        return;
    }

    m_endPoint = QPointF{m_routeInformation.back().coordinates.first, m_routeInformation.back().coordinates.second};
    emit endPointChanged();
}

void FigureAnalyzer::updateFigureDimensions()
{
    if (m_routeInformation.empty())
    {
        return;
    }

    auto minX = m_routeInformation.front().coordinates.first;
    auto maxX = m_routeInformation.front().coordinates.first;
    auto minY = m_routeInformation.front().coordinates.second;
    auto maxY = m_routeInformation.front().coordinates.second;

    for (auto const &point : m_routeInformation)
    {
        //Width (X)
        if (point.coordinates.first <= minX)
        {
            minX = point.coordinates.first;
        }
        else if (point.coordinates.first > maxX)
        {
            maxX = point.coordinates.first;
        }
        //Height (Y)
        if (point.coordinates.second <= minY)
        {
            minY = point.coordinates.second;
        }
        else if (point.coordinates.second > maxY)
        {
            maxY = point.coordinates.second;
        }
    }

    m_figureWidth = maxX - minX;
    emit figureWidthChanged();

    m_figureHeight = maxY - minY;
    emit figureHeightChanged();
}

void FigureAnalyzer::updateRouteProperties()
{
    if (m_routeInformation.empty())
    {
        return;
    }

    m_routeParts.clear();
    m_routeParts.reserve(m_routeInformation.size() - 1);
    auto length = 0.0;
    auto time = 0.0;
    auto useGlobalSpeed = std::none_of(m_routeInformation.begin(), m_routeInformation.end(), [] (const auto& information)
    {
        return information.speed > 0.0;
    });

    for (unsigned int i = 1; i < m_routeInformation.size(); i++)
    {
        const auto& previousPoint = m_routeInformation.at(i - 1);
        const auto& currentPoint = m_routeInformation.at(i);
        auto partLength = calculateEuclideanDistance({previousPoint.coordinates.first, previousPoint.coordinates.second}, {currentPoint.coordinates.first, currentPoint.coordinates.second});
        auto partTime = 0.0;
        auto partSpeed = 0.0;

        if (m_isWobbleFigure)
        {
            partLength /= m_microVectorFactor;      //mm
            partTime = 0.00001;                     //RTC6 card wobbles with a clock of 10us, thus the analyzation of the wobble figure needs the part time of 10us to be correct.
            partSpeed = partLength / partTime;      //mm/s
        }
        else if (!m_isWobbleFigure && useGlobalSpeed)
        {
            partSpeed = FigureEditorSettings::instance()->scannerSpeed();
            partTime = partLength / partSpeed;
        }
        else
        {
            partSpeed = previousPoint.speed;
            partTime = partLength / partSpeed;
        }

        length += partLength;
        time += partTime;
        m_routeParts.push_back(FigurePartInformation{partLength, partSpeed, partTime});
    }

    m_routeLength = length;
    emit routeLengthChanged();
    m_figureTime = time;
    emit figureTimeChanged();
}

void FigureAnalyzer::updateWobblePointDistance()
{
    if (m_microVectorFactor <= 0 || m_wobbleFigurePointCount < 1)
    {
        return;
    }

    auto frequency = qRound(1.0 / ((m_wobbleFigurePointCount - 1) * m_microVectorFactor * 0.00001));
    auto theoreticalWobblePointDistance = threeDigitLimitation(FigureEditorSettings::instance()->scannerSpeed() / frequency);

    m_wobblePointDistance = theoreticalWobblePointDistance;

    emit wobblePointDistanceChanged();
}

void FigureAnalyzer::updateFocusSpeed()
{
    if (!m_simulationController)
    {
        return;
    }

    const auto &focusSpeed = m_simulationController->simulationFocusSpeed();
    if (focusSpeed.empty())
    {
        return;
    }

    auto minMax = std::minmax_element(focusSpeed.begin(), focusSpeed.end());
    auto average = std::accumulate(focusSpeed.begin(), focusSpeed.end(), 0.0) / focusSpeed.size();

    m_minFocusSpeed = *minMax.first;
    m_maxFocusSpeed = *minMax.second;
    m_averageFocusSpeed = average;

    emit focusSpeedChanged();
}

double FigureAnalyzer::calculateEuclideanDistance(const QPointF &firstPoint, const QPointF &secondPoint)
{
    QVector2D distance{firstPoint - secondPoint};
    return distance.length();
}

double FigureAnalyzer::threeDigitLimitation(double value)
{
    return static_cast<double> (qRound(value * 1000)) / 1000.0;
}

}
}
}
}
