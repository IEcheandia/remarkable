#include "LaserTrajectory.h"
#include <QDebug>

namespace precitec
{
namespace scantracker
{
namespace components
{
namespace wobbleFigureEditor
{

LaserTrajectory::LaserTrajectory(QObject* parent)
    : qan::Edge()
{
    this->sm_isRampEdge = false;
}

LaserTrajectory::~LaserTrajectory() = default;

int LaserTrajectory::ID() const
{
    return m_ID;
}

void LaserTrajectory::setID(const int id)
{
    if (m_ID != id)
    {
        m_ID = id;
        IDChanged();
    }
}

bool LaserTrajectory::editable() const
{
    return m_editable;
}

void LaserTrajectory::setEditable(bool edit)
{
    if (m_editable != edit)
    {
        m_editable = edit;
        editableChanged();
    }
}

double LaserTrajectory::speed() const
{
    return m_speed;
}

void LaserTrajectory::setSpeed(const double& scannerSpeed)
{
    if (m_speed != scannerSpeed)
    {
        m_speed = scannerSpeed;
        speedChanged();
    }
}

double LaserTrajectory::trajectoryLength() const
{
    return m_length;
}

void LaserTrajectory::setTrajectoryLength(const double& length)
{
    if (m_length != length)
    {
        m_length = length;
        emit trajectoryLengthChanged();
    }
}

double LaserTrajectory::time() const
{
    return m_time;
}

void LaserTrajectory::setTime(const double& newTime)
{
    if (m_time != newTime)
    {
        m_time = newTime;
        emit timeChanged();
    }
}

int LaserTrajectory::group() const
{
    return m_group;
}

void LaserTrajectory::setGroup(int newGroup)
{
    if (m_group != newGroup)
    {
        m_group = newGroup;
        emit groupChanged();
    }
}

int LaserTrajectory::type() const
{
    return m_type;
}

void LaserTrajectory::setType(int newVectorType)
{
    if (m_type != newVectorType)
    {
        m_type = newVectorType;
        emit typeChanged();
    }
}

bool LaserTrajectory::isRampEdge() const
{

    return m_isRampEdge;
}

void LaserTrajectory::setIsRampEdge(bool isRamp)
{
    if (m_isRampEdge == isRamp)
    {
        return;
    }

    m_isRampEdge = isRamp;
    sm_isRampEdge = m_isRampEdge;
    emit isRampEdgeChanged();
}

bool LaserTrajectory::hasGradient() const
{
    return m_hasGradient;
}

void LaserTrajectory::setHasGradient(bool hasNewGradient)
{
    m_hasGradient = hasNewGradient;
    emit hasGradientChanged();
}

QColor LaserTrajectory::startColor() const
{
    return m_startColor;
}

void LaserTrajectory::setStartColor(QColor newStartColor)
{
    m_startColor = newStartColor;
    emit startColorChanged();
}

QColor LaserTrajectory::endColor() const
{
    return m_endColor;
}

void LaserTrajectory::setEndColor(QColor newEndColor)
{
    m_endColor = newEndColor;
    emit endColorChanged();
}

double LaserTrajectory::startPowerPosition() const
{
    return m_startPowerPosition;
}

void LaserTrajectory::setStartPowerPosition(double newStartPowerPosition)
{
    if (m_startPowerPosition == newStartPowerPosition)
        return;
    m_startPowerPosition = newStartPowerPosition;
    emit startPowerPositionChanged();
}

double LaserTrajectory::endPowerPosition() const
{
    return m_endPowerPosition;
}

void LaserTrajectory::setEndPowerPosition(double newEndPowerPosition)
{
    if (m_endPowerPosition == newEndPowerPosition)
        return;
    m_endPowerPosition = newEndPowerPosition;
    emit endPowerPositionChanged();
}

double LaserTrajectory::recStrength() const
{
    return m_recStrength;
}

void LaserTrajectory::setRecStrength(double newRecStrength)
{
    if (m_recStrength == newRecStrength)
        return;
    m_recStrength = newRecStrength;
    emit recStrengthChanged();
}

QQmlComponent* LaserTrajectory::delegate(QQmlEngine& engine, QObject* parent)
{
    static std::unique_ptr<QQmlComponent> delegate;
    if (!delegate && sm_isRampEdge)
    {
        delegate = std::make_unique<QQmlComponent>(&engine, "qrc:/figureeditor/LaserTrajectory.qml",
                                                   QQmlComponent::PreferSynchronous, parent);
    }
    return delegate.get();
}
}
}
}
}
