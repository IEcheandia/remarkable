#include "selectionHandler.h"

#include "WobbleFigure.h"
#include "LaserPoint.h"

namespace precitec
{
namespace scantracker
{
namespace components
{
namespace wobbleFigureEditor
{

SelectionHandler::SelectionHandler(QObject* parent) : QObject(parent)
{ }

SelectionHandler::~SelectionHandler() = default;

void SelectionHandler::setFigure(WobbleFigure* newFigure)
{
    if (m_figure == newFigure)
    {
        return;
    }

    disconnect(m_figureDestroyedConnection);
    m_figure = newFigure;

    if (m_figure)
    {
        m_figureDestroyedConnection = connect(m_figure, &QObject::destroyed, this, std::bind(&SelectionHandler::setFigure, this, nullptr));
    }
    else
    {
        m_figureDestroyedConnection = {};
    }

    emit figureChanged();
}

void SelectionHandler::addLaserPoint(QObject* newLaserPoint)
{
    if (!newLaserPoint || (newLaserPoint == m_startAndEndPoints.first) || (newLaserPoint == m_startAndEndPoints.second))
    {
        return;
    }

    if (!dynamic_cast<LaserPoint*> (newLaserPoint))
    {
        return;
    }

    auto laserPoint = dynamic_cast<LaserPoint*> (newLaserPoint);

    if (!m_startAndEndPoints.first || (m_startAndEndPoints.first && m_startAndEndPoints.second))
    {
        m_startAndEndPoints.second = m_startAndEndPoints.first;
        m_startAndEndPoints.first = laserPoint;
        emit selectionChanged();
        return;
    }
    if (!m_startAndEndPoints.second && m_startAndEndPoints.first)
    {
        m_startAndEndPoints.second = laserPoint;
        emit selectionChanged();
    }
}

void SelectionHandler::resetLaserPoints()
{
    m_startAndEndPoints.first = nullptr;
    m_startAndEndPoints.second = nullptr;
    emit selectionChanged();
}

double SelectionHandler::getLaserPower(int ID)
{
    if (!m_figure || ID < 0 || std::size_t(ID) > m_figure->get_node_count())
    {
        return -1.0;
    }

    const auto &laserPoints = m_figure->get_nodes();
    auto laserPoint = std::find_if(laserPoints.begin(), laserPoints.end(), [ID](const auto &currentLaserPoint){return dynamic_cast<LaserPoint*> (currentLaserPoint)->ID() == ID;});
    if (laserPoint == laserPoints.end())
    {
        return -1.0;
    }

    return dynamic_cast<LaserPoint*> (*laserPoint)->laserPower();
}

double SelectionHandler::getLaserRingPower(int ID)
{
    if (!m_figure || ID < 0 || std::size_t(ID) > m_figure->get_node_count())
    {
        return -1.0;
    }

    const auto &laserPoints = m_figure->get_nodes();
    auto laserPoint = std::find_if(laserPoints.begin(), laserPoints.end(), [ID](const auto &currentLaserPoint){return dynamic_cast<LaserPoint*> (currentLaserPoint)->ID() == ID;});
    if (laserPoint == laserPoints.end())
    {
        return -1.0;
    }

    return dynamic_cast<LaserPoint*> (*laserPoint)->ringPower();
}

double SelectionHandler::getVelocity(int ID)
{
  // TODO: duplicated Code with the methods above
  if (!m_figure || ID < 0 || std::size_t(ID) > m_figure->get_node_count())
  {
      return -1.0;
  }

  const auto &laserPoints = m_figure->get_nodes();
  auto laserPoint = std::find_if(laserPoints.begin(), laserPoints.end(), [ID](const auto &currentLaserPoint){return dynamic_cast<LaserPoint*> (currentLaserPoint)->ID() == ID;});
  if (laserPoint == laserPoints.end())
  {
      return -1.0;
  }

  return dynamic_cast<LaserPoint*> (*laserPoint)->velocity();
}

std::pair<LaserPoint*, LaserPoint*> SelectionHandler::getPoints()
{
    return m_startAndEndPoints;
}

int SelectionHandler::getMaxNumberOfPoints()
{
    if (!m_figure)
    {
        return 0;
    }

    return m_figure->get_node_count() - 1;
}

}
}
}
}
