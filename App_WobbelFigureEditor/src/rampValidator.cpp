#include "rampValidator.h"

namespace precitec
{
namespace scanmaster
{
namespace components
{
namespace wobbleFigureEditor
{

RampValidator::RampValidator(QObject* parent)
    : QObject(parent)
{
    connect(this, &RampValidator::startPointIDChanged, this, &RampValidator::checkPointAlreadyInRamp);
    connect(this, &RampValidator::startPointIDChanged, this, &RampValidator::updateMaxRampLength);
}

RampValidator::~RampValidator() = default;

void RampValidator::setFigureEditor(WobbleFigureEditor* newFigureEditor)
{
    if (m_figureEditor == newFigureEditor)
    {
        return;
    }
    disconnect(m_figureEditorDestroyedConnection);

    if (m_figureEditor)
    {
        disconnect(m_figureEditor, &WobbleFigureEditor::seamPointsChanged, this, &RampValidator::takeSeamPoints);
        disconnect(m_figureEditor, &WobbleFigureEditor::rampsChanged, this, &RampValidator::initRamps);
    }
    m_figureEditor = newFigureEditor;

    if (m_figureEditor)
    {
        m_figureEditorDestroyedConnection = connect(m_figureEditor, &QObject::destroyed, this, std::bind(&RampValidator::setFigureEditor, this, nullptr));
        connect(m_figureEditor, &WobbleFigureEditor::seamPointsChanged, this, &RampValidator::takeSeamPoints);
        connect(m_figureEditor, &WobbleFigureEditor::rampsChanged, this, &RampValidator::initRamps);
    }
    else
    {
        m_figureEditorDestroyedConnection = {};
    }
    emit figureEditorChanged();
}

void RampValidator::setLaserPointController(LaserPointController* newPointController)
{
    if (m_laserPointController == newPointController)
    {
        return;
    }
    disconnect(m_laserPointControllerDestroyedConnection);
    m_laserPointController = newPointController;

    if (m_laserPointController)
    {
        m_laserPointControllerDestroyedConnection = connect(m_laserPointController, &QObject::destroyed, this, std::bind(&RampValidator::setLaserPointController, this, nullptr));
    }
    else
    {
        m_laserPointControllerDestroyedConnection = {};
    }
    emit laserPointControllerChanged();
}

void RampValidator::setStartPointID(int newID)
{
    if (m_startPointID == newID)
    {
        return;
    }

    m_startPointID = newID;
    emit startPointIDChanged();
}

void RampValidator::setIsPointAlreadyInRamp(bool isPointInOneRamp)
{
    if (m_isPointAlreadyInRamp == isPointInOneRamp)
    {
        return;
    }

    m_isPointAlreadyInRamp = isPointInOneRamp;
    emit isPointAlreadyInRampChanged();
}

void RampValidator::setIsPointAStartPoint(bool isStartPointOfARamp)
{
    if (m_isPointAStartPoint == isStartPointOfARamp)
    {
        return;
    }

    m_isPointAStartPoint = isStartPointOfARamp;
    emit isPointAStartPointChanged();
}

double RampValidator::maxRampLength() const
{
    switch (m_enterLengthInDegree)
    {
    case LengthType::Millimeter:
        return m_maxRampLength;
    case LengthType::Degree:
        return convertLengthToDegree(m_maxRampLength);
    case LengthType::Percent:
        return convertLengthToPercent(m_maxRampLength);
    default:
        return 0.0;
    }
}

void RampValidator::setMaxRampLength(double maxLengthForCurrentRamp)
{
    if (m_maxRampLength == maxLengthForCurrentRamp)
    {
        return;
    }

    m_maxRampLength = maxLengthForCurrentRamp;
    emit maxRampLengthChanged();
}

QString RampValidator::lengthLabel() const
{
    switch (m_enterLengthInDegree)
    {
    case LengthType::Millimeter:
        return QString("Length [mm]:");
    case LengthType::Degree:
        return QString("Degree [°]");
    case LengthType::Percent:
        return QString("Percent [%]");
    default:
        return {};
    }
}

void RampValidator::setEnterLengthInDegree(LengthType enterInDegree)
{
    if (m_enterLengthInDegree == enterInDegree)
    {
        return;
    }

    m_enterLengthInDegree = enterInDegree;
    emit enterLengthInDegreeChanged();
    emit lengthLabelChanged();
    emit maxRampLengthChanged();
}

void RampValidator::setSeamPoints(const std::vector<QVector2D>& points)
{
    if (m_seamPoints == points)
    {
        return;
    }

    m_seamPoints = points;
    emit seamPointsChanged();
}

void RampValidator::setRamps(const std::vector<Ramp>& currentRamps)
{
    if (m_ramps == currentRamps)
    {
        return;
    }

    m_ramps = currentRamps;
    emit rampsChanged();
}

double RampValidator::convertLengthToDegree(double value) const
{
    if (qFuzzyIsNull(m_wholeLength))
    {
        return value;
    }
    return (value / m_wholeLength) * 360.0;
}

double RampValidator::convertDegreeToLength(double value) const
{
    return (value / 360.0) * m_wholeLength;
}

double RampValidator::convertLengthToPercent(double value) const
{
    return (value / m_wholeLength) * 100.0;
}

double RampValidator::getRampLenght(double value) const
{
    switch (m_enterLengthInDegree)
    {
    case LengthType::Millimeter:
        return value;
    case LengthType::Degree:
        return convertLengthToDegree(value);
    case LengthType::Percent:
        return convertLengthToPercent(value);
    default:
        return 0.0;
    }
}

double RampValidator::getValueInMillimeter(double value) const
{
    switch (m_enterLengthInDegree)
    {
    case LengthType::Millimeter:
        return value;
    case LengthType::Degree:
        return convertDegreeToLength(value);
    case LengthType::Percent:
        return (value / 100.0) * m_wholeLength;
    default:
        return 0.0;
    }
}

void RampValidator::takeSeamPoints()
{
    if (!m_figureEditor)
    {
        return;
    }

    setSeamPoints(castOrderPositionToVector(m_figureEditor->seamPoints()));
    calculateWholeLength();
    initRamps();
}

std::vector<QVector2D> RampValidator::castOrderPositionToVector(const std::vector<RTC6::seamFigure::command::Order>& orders)
{
    std::vector<QVector2D> points;
    for (const auto& order : orders)
    {
        points.push_back(QVector2D{static_cast<float>(order.endPosition.first), static_cast<float>(order.endPosition.second)});
    }

    return points;
}

void RampValidator::takeRamps()
{
    if (!m_figureEditor)
    {
        return;
    }
    setRamps(m_figureEditor->ramps());
}

bool RampValidator::necessaryInfoAvailable()
{
    if (m_seamPoints.empty() || m_ramps.empty())
    {
        return false;
    }
    return true;
}

void RampValidator::clearEndPoints()
{
    m_endPoints.clear();
    m_endPoints.reserve(m_ramps.size());

    emit endPointsChanged();
}

QVector3D RampValidator::calculateEndPoint(std::size_t startPointID, double rampLength)
{
    if (m_seamPoints.empty())
    {
        return {};
    }

    if (startPointID == (m_seamPoints.size() - 1))
    {
        std::reverse(m_seamPoints.begin(), m_seamPoints.end());
        auto startPointRampOut = calculateEndPointPosition(0, rampLength);
        std::reverse(m_seamPoints.begin(), m_seamPoints.end());
        startPointRampOut.setZ(startPointID);
        return startPointRampOut;
    }

    if (startPointID >= m_seamPoints.size())
    {
        return {};
    }

    return calculateEndPointPosition(startPointID, rampLength);
}

QVector3D RampValidator::calculateEndPointPosition(std::size_t startPointID, double rampLength)
{
    auto lastPoint = m_seamPoints[startPointID];
    double length = rampLength;
    std::size_t currentPoint = startPointID + 1;

    while (currentPoint < m_seamPoints.size())
    {
        const auto& targetPoint = m_seamPoints[currentPoint];
        auto vector = targetPoint - lastPoint;
        const auto distance = vector.length();
        vector.normalize();

        if (distance < length)
        {
            length -= distance;
            lastPoint = targetPoint;
            currentPoint++;
        }
        else
        {
            lastPoint = lastPoint + (vector * length);
            break;
        }
    }

    return QVector3D{lastPoint, static_cast<float>(startPointID)};
}

void RampValidator::calculateEndPoints()
{
    if (m_rampsHasRampOut)
    {
        const auto& rampOutStartPoint = calculateEndPoint(m_ramps.back().startPointID, m_ramps.back().length);

        for (std::size_t i = 0; i < (m_ramps.size() - 1); i++)
        {
            const auto& currentRamp = m_ramps[i];
            m_endPoints.push_back(calculateEndPoint(currentRamp.startPointID, currentRamp.length));
        }
        m_endPoints.push_back(rampOutStartPoint);
    }
    else
    {
        for (const auto& ramp : m_ramps)
        {
            m_endPoints.push_back(calculateEndPoint(ramp.startPointID, ramp.length));
        }
    }

    emit endPointsChanged();
}

void RampValidator::hasRampOut()
{
    if (m_seamPoints.empty())
    {
        m_rampsHasRampOut = false;
    }

    m_rampsHasRampOut = std::any_of(m_ramps.begin(), m_ramps.end(), [&](const auto& currentRamp)
                                    { return currentRamp.startPointID == (m_seamPoints.size() - 1); });
}

void RampValidator::initRamps()
{
    takeRamps();
    hasRampOut();
    updateMaxRampLength();

    if (!necessaryInfoAvailable())
    {
        clearEndPoints();
        drawRamps();
        return;
    }

    clearEndPoints();
    calculateEndPoints();
    drawRamps();
}

void RampValidator::checkPointAlreadyInRamp()
{
    if (!necessaryInfoAvailable())
    {
        setIsPointAStartPoint(false);
        setIsPointAlreadyInRamp(false);
        return;
    }

    if (auto startPointFound = checkPointIsStartPoint(m_startPointID))
    {
        setIsPointAStartPoint(startPointFound);
        setIsPointAlreadyInRamp(startPointFound);
        return;
    }

    setIsPointAStartPoint(false);
    setIsPointAlreadyInRamp(findPointInRamps());
}

bool RampValidator::checkPointIsStartPoint(int toBeCheckedID)
{
    const auto& checkedID = static_cast<std::size_t>(toBeCheckedID);
    return std::any_of(m_ramps.begin(), m_ramps.end(), [checkedID](const auto& currentRamp)
                       { return currentRamp.startPointID == checkedID; });
}

bool RampValidator::findPointInRamps()
{
    if (m_ramps.size() == 0 || (m_startPointID == 0 && !m_rampsHasRampOut))
    {
        return false;
    }

    if (m_startPointID == 0)
    {
        return checkPointIsInRampOut();
    }

    if (checkPointIsInRamp())
    {
        return true;
    }

    if (m_rampsHasRampOut)
    {
        return checkPointIsInRampOut();
    }

    return false;
}

bool RampValidator::checkPointIsInRampOut()
{
    std::reverse(m_seamPoints.begin(), m_seamPoints.end());
    const auto& lengthEndToPoint = calculateLengthPointToPoint(0, m_seamPoints.size() - (1 + m_startPointID));
    std::reverse(m_seamPoints.begin(), m_seamPoints.end());

    if (lengthEndToPoint <= m_ramps.back().length)
    {
        return true;
    }
    return false;
}

bool RampValidator::checkPointIsInRamp()
{
    for (const auto& ramp : m_ramps)
    {
        if (ramp.startPointID > static_cast<std::size_t>(m_startPointID))
        {
            continue;
        }

        const auto& lengthToPoint = calculateLengthPointToPoint(ramp.startPointID, m_startPointID);
        if (lengthToPoint <= ramp.length)
        {
            return true;
        }
    }
    return false;
}

void RampValidator::updateMaxRampLength()
{
    if (m_seamPoints.empty() && m_startPointID > -1)
    {
        return;
    }

    if (checkIDBelongsToRampOut(m_startPointID))
    {
        if (m_ramps.empty() || (m_ramps.size() == 1 && m_ramps.back().startPointID == static_cast<unsigned int>(m_startPointID)))
        {
            setMaxRampLength(calculateLengthPointToPoint(0, m_startPointID));
            return;
        }
        if (m_ramps.back().startPointID == static_cast<unsigned int>(m_startPointID))
        {
            const auto& rampBeforeRampOut = m_ramps.at(m_ramps.size() - 2);
            setMaxRampLength(calculateLengthPointToPoint(rampBeforeRampOut.startPointID, m_startPointID) - rampBeforeRampOut.length);
            return;
        }
        else
        {
            const auto& rampBeforeRampOut = m_ramps.at(m_ramps.size() - 1);
            setMaxRampLength(calculateLengthPointToPoint(rampBeforeRampOut.startPointID, m_startPointID) - rampBeforeRampOut.length);
            return;
        }
    }

    auto nextRampID = searchNextRamp(m_startPointID);
    if (checkIDBelongsToRampOut(nextRampID))
    {
        const auto& rampOutLength = lengthFromRamp(nextRampID);
        setMaxRampLength(calculateLengthPointToPoint(m_startPointID, nextRampID) - rampOutLength);
        return;
    }

    setMaxRampLength(calculateLengthPointToPoint(m_startPointID, nextRampID));
}

std::size_t RampValidator::searchNextRamp(std::size_t startPointID)
{
    auto it = std::find_if(m_ramps.begin(), m_ramps.end(), [startPointID](const auto& currentRamp)
                           { return currentRamp.startPointID > startPointID; });
    if (it == m_ramps.end())
    {
        return (m_seamPoints.size() - 1);
    }

    return it->startPointID;
}

bool RampValidator::checkIDBelongsToRampOut(std::size_t checkedID)
{
    if (m_seamPoints.empty() || checkedID != m_seamPoints.size() - 1)
    {
        return false;
    }

    return true;
}

double RampValidator::lengthFromRamp(std::size_t startPointID)
{
    if (m_ramps.empty())
    {
        return 0.0;
    }

    auto it = std::find_if(m_ramps.begin(), m_ramps.end(), [startPointID](const auto& currentRamp)
                           { return currentRamp.startPointID == startPointID; });
    if (it == m_ramps.end())
    {
        return 0.0;
    }

    return it->length;
}

double RampValidator::calculateLengthPointToPoint(std::size_t startPointID, std::size_t targetPointID)
{
    auto length = 0.0;
    if (startPointID > targetPointID || startPointID >= (m_seamPoints.size() - 1) || targetPointID > (m_seamPoints.size() - 1))
    {
        return length;
    }

    auto id = startPointID;
    while ((id + 1) <= targetPointID && id < m_seamPoints.size())
    {
        length += QVector2D(m_seamPoints[id] - m_seamPoints[id + 1]).length();
        id++;
    }

    return length;
}

void RampValidator::calculateWholeLength()
{
    if (m_seamPoints.empty())
    {
        m_wholeLength = 0.0;
        return;
    }

    m_wholeLength = calculateLengthPointToPoint(0, m_seamPoints.size() - 1);
}

void RampValidator::drawRamps()
{
    if (!m_laserPointController)
    {
        return;
    }

    m_laserPointController->drawRamps(m_endPoints);
}

}
}
}
}
