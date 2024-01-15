#include "trajectoryColorsValidator.h"
#include "LaserPoint.h"
#include "LaserTrajectory.h"
#include "figureEditorSettings.h"

using precitec::scanmaster::components::wobbleFigureEditor::FigureEditorSettings;

namespace precitec
{

namespace scantracker
{

namespace components
{

namespace wobbleFigureEditor
{

TrajectoryColorsValidator::TrajectoryColorsValidator(QObject* parent)
    : QObject(parent)
{
}

TrajectoryColorsValidator::~TrajectoryColorsValidator() = default;

WobbleFigure* TrajectoryColorsValidator::figure() const
{
    return m_figure;
}

void TrajectoryColorsValidator::setFigure(WobbleFigure* newFigure)
{
    if (m_figure == newFigure)
        return;
    m_figure = newFigure;
    emit figureChanged();
}

RTC6::seamFigure::SeamFigure* TrajectoryColorsValidator::seamfigure() const
{
    return m_seamfigure;
}

void TrajectoryColorsValidator::setSeamFigure(RTC6::seamFigure::SeamFigure* newSeamFigure)
{
    if (m_seamfigure == newSeamFigure)
        return;
    m_seamfigure = newSeamFigure;
    emit seamfigureChanged();
}

void TrajectoryColorsValidator::calculateColors()
{
    if (nullptr == m_figure || nullptr == m_seamfigure)
        return;
    auto const& ramps = m_seamfigure->ramps;
    if (ramps.size() > 0)
    {
        resetEdgeHasGradient();
    }
    for (auto const& ramp : ramps)
    {
        auto startPoint = m_figure->searchLaserPoint(ramp.startPointID);
        int startPointId = startPoint->ID();
        bool rampOut = {m_seamfigure->figure.size() == ramp.startPointID + 1};
        QMap<int, double> idListWithDistance;
        // Startpoint
        if (nullptr != startPoint)
        {
            idListWithDistance = calculateDistance(startPointId, ramp.length, rampOut);
            if (rampOut)
            {
                startPointId = idListWithDistance.firstKey();
            }
        }
        auto fes = FigureEditorSettings::instance();
        double startPower = ramp.startPower;
        double endPower = ramp.endPower;
        bool powerDown = startPower > endPower;
        double powerDiff = std::abs(endPower - startPower);
        double distance = 0;
        double lengthCounter = 0;
        bool newStartPosition{false};
        bool newEndPosition{false};
        double endPosition{1.0};
        double startPosition{0.0};
        QColor startColor = "black";
        QColor endColor = "black";

        if (rampOut)
        {
            double wholeDistance{0};
            for (auto const& value : idListWithDistance.values())
            {
                wholeDistance += value;
            }

            // Berechne die distance vom ersten zum zweiten Punkt
            double segmentLength = idListWithDistance.value(idListWithDistance.firstKey()) - (wholeDistance - ramp.length);
            distance = segmentLength / ramp.length;
            startPosition = 1.0 - (segmentLength / idListWithDistance.value(idListWithDistance.firstKey()));
            startColor = fes->colorFromValue(startPower * 100);
            if (powerDown)
                endPower = startPower - (distance * powerDiff);
            else
                endPower = (distance * powerDiff) + startPower;
            // merke dir die erste zurückgelegte Länge
            lengthCounter = distance;
            QColor endColor = fes->colorFromValue(endPower * 100);
            startPower = endPower;
            auto point = m_figure->searchLaserPoint(idListWithDistance.firstKey());
            auto connection = point->get_out_edges().at(0);
            auto edge = qobject_cast<LaserTrajectory*>(connection);
            edge->setStartColor(startColor);
            edge->setEndColor(endColor);
            edge->setRecStrength(m_recStrenght);
            edge->setStartPowerPosition(startPosition);
            edge->setHasGradient(true);
            auto style = edge->getItem()->getStyle();
            style->setLineColor(endColor);

            startPointId++;
        }

        for (auto const& key : idListWithDistance.keys())
        {
            if (rampOut)
            {
                if (key == idListWithDistance.firstKey())
                    continue;
                startPointId = key;
            }
            // Holen des Start-Farbwerts
            if (!newStartPosition)
                startColor = fes->colorFromValue(startPower * 100);
            // Kalkuliere die kommende Distance zum nächsten Punkt in Prozent
            distance = idListWithDistance.value(key) / ramp.length;
            // Merke dir die bisher zurückgelegt Strecke
            lengthCounter += distance;
            if (lengthCounter >= 1.0)
            {
                double oldDistance = distance;
                distance -= (lengthCounter - 1.0);
                endPosition = distance / oldDistance;
                newEndPosition = true;
            }
            if (powerDown)
                endPower = startPower - (distance * powerDiff);
            else
                endPower = (distance * powerDiff) + startPower;
            endColor = fes->colorFromValue(endPower * 100);
            startPower = endPower;
            auto point = m_figure->searchLaserPoint(startPointId);
            auto connection = point->get_out_edges().at(0);
            auto edge = qobject_cast<LaserTrajectory*>(connection);
            edge->setStartColor(startColor);
            edge->setEndColor(endColor);
            point->setLineStyle(false, endColor);
            edge->setRecStrength(m_recStrenght);
            if (newStartPosition)
            {
                edge->setStartPowerPosition(startPosition);
                newStartPosition = false;
            }
            if (newEndPosition)
            {
                edge->setEndPowerPosition(endPosition);
                newEndPosition = false;
            }
            edge->setHasGradient(true);
            auto style = edge->getItem()->getStyle();
            style->setLineColor(endColor);

            if (!rampOut)
                startPointId = key;
        }
        auto const& point = m_figure->searchLaserPoint(startPointId);
        double nextPower = point->laserPower();
        if (qFuzzyCompare(nextPower, -1) || rampOut)
        {
            point->setLineStyle(true, endColor);
        }
        else
        {
            point->setLineStyle(false, fes->colorFromValue(nextPower));
        }
        point->modifyAttachedEdge(true, true);
    }
}

QMap<int, double> TrajectoryColorsValidator::calculateDistance(int startPointID, double rampLength, bool rampOut)
{
    std::vector<QVector2D> seamPoints = castOrderPositionToVector(m_seamfigure->figure);
    auto lastPoint = seamPoints.at(startPointID);
    double length = rampLength;
    int stepDirection = rampOut ? -1 : 1;
    std::size_t currentPoint = startPointID + stepDirection;

    QMap<int, double> pointIDs;

    while (currentPoint < seamPoints.size())
    {
        auto targetPoint = seamPoints.at(currentPoint);
        auto vector = targetPoint - lastPoint;
        const auto distance = vector.length();

        if (distance < length)
        {
            pointIDs.insert(currentPoint, distance);
            length -= distance;
            lastPoint = targetPoint;
            currentPoint += stepDirection;
        }
        else
        {
            pointIDs.insert(currentPoint, distance);
            break;
        }
    }
    return pointIDs;
}

void TrajectoryColorsValidator::resetEdgeHasGradient()
{
    if (nullptr == m_figure)
        return;
    auto& edges = m_figure->get_edges();
    auto fes = FigureEditorSettings::instance();
    for (auto edge : edges)
    {
        auto trajectory = qobject_cast<LaserTrajectory*>(edge);
        if (trajectory->hasGradient())
        {
            trajectory->setHasGradient(false);
            double power = qobject_cast<LaserPoint*>(trajectory->getSource())->laserPower();
            auto style = edge->getItem()->getStyle();
            style->setLineColor(fes->colorFromValue(power));
        }
    }
}

// todo: inefficient - only look for attached edges
std::vector<QVector2D> TrajectoryColorsValidator::castOrderPositionToVector(const std::vector<RTC6::seamFigure::command::Order>& orders)
{
    std::vector<QVector2D> points;
    for (const auto& order : orders)
    {
        points.push_back(QVector2D{static_cast<float>(order.endPosition.first), static_cast<float>(order.endPosition.second)});
    }

    return points;
}

} // namepsace wobbleFigureEditor
} // namepsace components
} // namepsace scantracker
} // namepsace precitec
