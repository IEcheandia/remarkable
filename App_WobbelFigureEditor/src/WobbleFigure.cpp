#include "WobbleFigure.h"

#include "LaserPoint.h"
#include "LaserTrajectory.h"

namespace precitec
{
namespace scantracker
{
namespace components
{
namespace wobbleFigureEditor
{

WobbleFigure::~WobbleFigure() = default;

QString WobbleFigure::name() const
{
    return m_name;
}

void WobbleFigure::setName(const QString& name)
{
    if (m_name != name)
    {
        m_name = name;
        emit nameChanged();
    }
}

int WobbleFigure::ID() const
{
    return m_ID;
}

void WobbleFigure::setID(int id)
{
    if (m_ID != id)
    {
        m_ID = id;
        emit IDChanged();
    }
}

QString WobbleFigure::description() const
{
    return m_description;
}

void WobbleFigure::setDescription(const QString& description)
{
    if (m_description != description)
    {
        m_description = description;
        emit descriptionChanged();
    }
}

bool WobbleFigure::analogPower() const
{
    return m_analogPower;
}

void WobbleFigure::setAnalogPower(bool isAnalogPower)
{
    if (m_analogPower == isAnalogPower)
    {
        return;
    }

    m_analogPower = isAnalogPower;
    emit analogPowerChanged();
}

LaserPoint* WobbleFigure::searchPoint(int ID)
{
    auto foundPoint = std::find_if(get_nodes().begin(), get_nodes().end(), [ID](auto currentPoint)
    {
        return qobject_cast<LaserPoint*> (currentPoint)->ID() == ID;
    });
    if (foundPoint == get_nodes().end())
    {
        return {};
    }

    return qobject_cast<LaserPoint*> (*foundPoint);
}

void WobbleFigure::setLaserPoint(LaserPoint* actualPoint)
{
    m_laserPoints.push_back(actualPoint);
}

LaserPoint * WobbleFigure::searchLaserPoint(int ID)
{
    auto foundPoint = std::find_if(m_laserPoints.begin(), m_laserPoints.end(), [ID](LaserPoint* actualPoint){return actualPoint->ID() == ID;});
    if (foundPoint != m_laserPoints.end())
    {
        return *foundPoint;
    }
    return {};
}

void WobbleFigure::setLaserTrajectory(LaserTrajectory* actualConnection)
{
    m_laserTrajectories.push_back(actualConnection);
}

LaserTrajectory * WobbleFigure::searchLaserTrajectory(int ID)
{
    auto foundTrajectory = std::find_if(m_laserTrajectories.begin(), m_laserTrajectories.end(), [ID](LaserTrajectory* actualTrajectory){return actualTrajectory->ID() == ID;});
    if (foundTrajectory != m_laserTrajectories.end())
    {
        return *foundTrajectory;
    }
    return {};
}

void WobbleFigure::resetLaserPoints()
{
  m_laserPoints.clear();
}

std::vector<LaserPoint*> WobbleFigure::getLaserPoints()
{
    resetLaserPoints();
    for (const auto& node : get_nodes())
    {
        if (const auto& point = qobject_cast<LaserPoint*>(node))
        {
            if (!point->isRampPoint())
            {
                m_laserPoints.emplace_back(point);
            }
        }
    }
    return m_laserPoints;
}

std::vector<LaserPoint*> WobbleFigure::rampPoints()
{
    std::vector<LaserPoint*> rampPoints;

    for (const auto& node : get_nodes())
    {
        if (const auto& point = qobject_cast<LaserPoint*>(node))
        {
            if (point->isRampPoint())
            {
                rampPoints.emplace_back(point);
            }
        }
    }

    return rampPoints;
}

qan::Node* WobbleFigure::insertLaserPoint(QQmlComponent* nodeComponent)
{
  auto laserPoint = insertNode<LaserPoint>(nodeComponent);
  setLaserPoint(dynamic_cast<LaserPoint*>(laserPoint));
  m_laserPointsQml.emplace_back(nodeComponent);
  return laserPoint;
}

qan::Node* WobbleFigure::insertLaserPointExact(const QPointF &position, int scale)
{
    auto counter = get_node_count();
    auto node = insertNode<LaserPoint>(nullptr);
    node->getItem()->setMinimumSize({15,15});
    node->getItem()->setRect({(position.x() * scale) - (15 * 0.5), (position.y() * scale) - (15 * 0.5), 15, 15});
    auto laserPoint = dynamic_cast<LaserPoint*>(node);
    laserPoint->setID(counter);
    laserPoint->initConnectionToNodeItem();
    return node;
}

qan::Edge * WobbleFigure::insertConnection(qan::Node* source, qan::Node* destination, QQmlComponent* edgeComponent)
{
    const auto engine = qmlEngine(this);
    if (source != nullptr && destination != nullptr && engine != nullptr)
    {
        return insertEdge<LaserTrajectory>(*source, destination, edgeComponent);
    }
    return {};
}

}
}
}
}
