#include "AttributeController.h"

using namespace precitec::scantracker::components::wobbleFigureEditor;

AttributeController::AttributeController(QObject* parent) : QObject(parent)
{
    connect(this, &AttributeController::selectedTrajectoryChanged, this, &AttributeController::changeLaserPointIDFromRampTrajectory);
}

AttributeController::~AttributeController() = default;

QObject * AttributeController::selection() const
{
  return m_selection;
}

void AttributeController::setSelection(QObject* object)
{
  m_selection = object;
  LaserPoint* lp = dynamic_cast<LaserPoint*>(object);
  LaserTrajectory* lt = dynamic_cast<LaserTrajectory*>(object);
  if (nullptr != lp)
  {
    setSelectedPoint(lp);
    setSelectedTrajectory(nullptr);
  }
  else if (lt)
  {
    setSelectedPoint(nullptr);
    setSelectedTrajectory(lt);
  }
  else
  {
    setSelectedPoint(nullptr);
    setSelectedTrajectory(nullptr);
  }
  emit selectionChanged();
}

void AttributeController::setSelectionHandler(QObject *object)
{
  if(m_selectionHandler != object)
  {
    m_selectionHandler = dynamic_cast<precitec::scantracker::components::wobbleFigureEditor::SelectionHandler*>(object);
  }
  emit selectionHandlerChanged();
}

QObject* AttributeController::selectionHandler() const
{
  return m_selectionHandler;
}

int AttributeController::figureScale() const
{
  return m_figureScale;
}

void AttributeController::setFigureScale(int newFactor)
{
  if (m_figureScale == newFactor)
  {
    return;
  }

  m_figureScale = newFactor;
  emit figureScaleChanged();
}

LaserPoint * AttributeController::selectedPoint() const
{
  return m_selectedPoint;
}

void AttributeController::setSelectedPoint(LaserPoint* point)
{
  if (m_selectedPoint != point)
  {
    m_selectedPoint = point;
    emit selectedPointChanged();
  }
}

LaserTrajectory * AttributeController::selectedTrajectory() const
{
  return m_selectedTrajectory;
}

void AttributeController::setSelectedTrajectory(LaserTrajectory* trajectory)
{
  if (m_selectedTrajectory != trajectory)
  {
    m_selectedTrajectory = trajectory;
    emit selectedTrajectoryChanged();
  }
}

void AttributeController::setFigure(WobbleFigure* newFigure)
{
  if (m_figure == newFigure)
  {
    return;
  }
  m_figure = newFigure;

  emit figureChanged();
}

WobbleFigure *AttributeController::figure() const
{
  return m_figure;
}

QString AttributeController::laserPointID() const
{
  return QString().setNum(m_selectedPoint->ID());
}

QString AttributeController::laserPointPositionX() const
{
  float value = m_selectedPoint->center().x() / m_figureScale;
  return convertfloatTo2PointedQString(value);
}

void AttributeController::setLaserPointPositionX(QString positionX)
{
  m_selectedPoint->setEditable(true);
  auto newPosition = QPointF(convertQStringToFloat(positionX) * m_figureScale, m_selectedPoint->center().y());
  m_selectedPoint->setCenter(newPosition);
  emit updated(m_selectedPoint);
}

QString AttributeController::laserPointPositionY() const
{
  float value = -1 * m_selectedPoint->center().y() / m_figureScale;
  return convertfloatTo2PointedQString(value);
}

void AttributeController::setLaserPointPositionY(QString positionY)
{
  m_selectedPoint->setEditable(true);
  auto scaledPosition = QPointF(m_selectedPoint->center().x(), -1 * convertQStringToFloat(positionY) * m_figureScale);
  m_selectedPoint->setCenter(scaledPosition);
  emit updated(m_selectedPoint);
}

QString AttributeController::laserPointLaserPower() const
{
  if(nullptr != m_selectedPoint && nullptr != m_selectionHandler )
  {
    int currentPointlaserPower = (int)m_selectedPoint->laserPower();
    if(currentPointlaserPower > -1)
    {
      return convertfloatTo0PointedQString(currentPointlaserPower);
    }

    for(int i = m_selectedPoint->ID(); i > -1; i--)
    {
      double currentPower = m_selectionHandler->getLaserPower(i);
      if(currentPower > -1)
      {
        return convertfloatTo0PointedQString(currentPower);
      }
    }
  }
  return convertfloatTo0PointedQString(-1);
}

void AttributeController::setLaserPointLaserPower(QString value)
{
  // TODO: workaround (-2) - if the fist point have no own definition -> 'use the definition of product' is also set for all other points
  m_selectedPoint->setEditable(true);
  m_selectedPoint->setLaserPower(convertQStringToFloat(value));
  emit laserPointLaserPowerChanged();
  emit currentlaserPointLaserPowerChanged();
  emit updated(m_selectedPoint);
}

QString AttributeController::currentlaserPointLaserPower() const
{
  if(nullptr != m_selectedPoint && nullptr != m_selectionHandler )
  {
    int currentPointlaserPower = (int)m_selectedPoint->laserPower();
    if(currentPointlaserPower > -1)
    {
      return QString("current Power: ") + convertfloatTo0PointedQString(currentPointlaserPower);
    }

    for(int i = m_selectedPoint->ID(); i > -1; i--)
    {
      double currentPower = m_selectionHandler->getLaserPower(i);
      if(currentPower > -1)
      {
        return QString("current Power: ") + convertfloatTo0PointedQString(currentPower);
      }
    }
    return QString("current power: use definition of the product");

  }
  return QString();
}

QString AttributeController::laserPointRingPower() const
{
  if(nullptr != m_selectedPoint && nullptr != m_selectionHandler )
  {
    int currentPointRingPower = m_selectedPoint->ringPower();
    if(currentPointRingPower > -1)
    {
      return convertfloatTo0PointedQString(currentPointRingPower);
    }

    for(int i = m_selectedPoint->ID(); i > -1; i--)
    {
      double currentPower = m_selectionHandler->getLaserRingPower(i);
      if(currentPower > -1)
      {
        return convertfloatTo0PointedQString(currentPower);
      }
    }
  }
  return convertfloatTo0PointedQString(-1);
}

void AttributeController::setLaserPointRingPower(QString value)
{
  m_selectedPoint->setEditable(true);
  m_selectedPoint->setRingPower(convertQStringToFloat(value));

  emit laserPointRingPowerChanged();
  emit currentlaserPointRingPowerChanged();
  emit updated(m_selectedPoint);
}

QString AttributeController::currentlaserPointRingPower() const
{
  if(nullptr != m_selectedPoint && nullptr != m_selectionHandler )
  {
    int currentPointRingPower = (int)m_selectedPoint->ringPower();
    if(currentPointRingPower > -1)
    {
      return QString("current Power: ") + convertfloatTo0PointedQString(currentPointRingPower);
    }

    for(int i = m_selectedPoint->ID(); i > -1; i--)
    {
      double currentPower = m_selectionHandler->getLaserRingPower(i);
      if(currentPower > -1)
      {
        return QString("current Power: ") + convertfloatTo0PointedQString(currentPower);
      }
    }
    return QString("current power: use definition of the product");

  }
  return QString();
}

QString AttributeController::laserPointVelocity() const
{
  if(nullptr != m_selectedPoint && nullptr != m_selectionHandler )
  {
    int currentPointVelocity = m_selectedPoint->velocity();
    if(currentPointVelocity > -1)
    {
      return convertfloatTo0PointedQString(currentPointVelocity);
    }

    for(int i = m_selectedPoint->ID(); i > -1; i--)
    {
      double currentVelocity = m_selectionHandler->getVelocity(i);
      if(currentVelocity > -1)
      {
        return convertfloatTo0PointedQString(currentVelocity);
      }
    }
  }
  return convertfloatTo0PointedQString(-1);
}

void AttributeController::setLaserPointVelocity(QString value)
{
  m_selectedPoint->setEditable(true);
  m_selectedPoint->setVelocity(convertQStringToFloat(value));

  emit laserPointVelocityChanged();
  emit currentlaserPointVelocityChanged();
  emit updated(m_selectedPoint);
}

QString AttributeController::currentlaserPointVelocity() const
{
  if(nullptr != m_selectedPoint && nullptr != m_selectionHandler )
  {
    int currentPointVelocity = (int)m_selectedPoint->velocity();
    if(currentPointVelocity > -1)
    {
      return QString("current velocity: ") + convertfloatTo0PointedQString(currentPointVelocity);
    }

    for(int i = m_selectedPoint->ID(); i > -1; i--)
    {
      double currentVelocity = m_selectionHandler->getVelocity(i);
      if(currentVelocity > -1)
      {
        return QString("current velocity: ") + convertfloatTo0PointedQString(currentVelocity);
      }
    }
    return QString("current velocity: use definition of the product");

  }
  return QString();
}

bool AttributeController::isLaserPointCurrentLaserPowerDependOnThePreviousPoint()
{
  if(nullptr != m_selectedPoint && 0 > m_selectedPoint->laserPower())
  {
    return true;
  }
  return false;
}

bool AttributeController::isLaserPointCurrentRingPowerDependOnThePreviousPoint()
{
  if(nullptr != m_selectedPoint && 0 > m_selectedPoint->ringPower())
  {
    return true;
  }
  return false;
}

bool AttributeController::isLaserPointCurrentVelocityDependOnThePreviousPoint()
{
  if(nullptr != m_selectedPoint && 0 > m_selectedPoint->velocity())
  {
    return true;
  }
  return false;
}

bool AttributeController::isLastPointOfFigure() const
{
  if(nullptr != m_selectedPoint && nullptr != m_figure)
  {
    const auto &laserPoints = m_figure->get_nodes();
    LaserPoint* point = dynamic_cast<LaserPoint*>(laserPoints.getContainer().back());
    if(point->ID() == m_selectedPoint->ID())
    {
      return true;
    }
  }
  return false;
}

void AttributeController::resetSelection()
{
    setSelection(nullptr);
}

void AttributeController::changeLaserPointIDFromRampTrajectory()
{
    if (!m_selectedTrajectory || !m_selectedTrajectory->isRampEdge())
    {
        return;
    }

    auto sourcePoint = sourcePointFromTrajectory();
    if (sourcePoint && !sourcePoint->isRampPoint())
    {
        setSelectedPoint(sourcePoint);
        emit loadRampDialog();
        return;
    }
}

void AttributeController::emitSignalsToUpdatePointPosition()
{
  if(nullptr != m_selectedPoint && nullptr != m_figure)
  {
    emit laserPointCoordinateXChanged();
    emit laserPointCoordinateYChanged();
  }
}

void AttributeController::emitSignalsToUpdatePointProperties()
{
    if(nullptr != m_selectedPoint && nullptr != m_figure)
    {
      emit laserPointLaserPowerChanged();
      emit currentlaserPointLaserPowerChanged();
      emit laserPointRingPowerChanged();
      emit currentlaserPointRingPowerChanged();
      emit laserPointVelocityChanged();
      emit currentlaserPointVelocityChanged();
    }
}

QString AttributeController::convertfloatTo2PointedQString(float value) const
{
  return QLocale(QLocale()).toString(value, 'f', 2);
}

QString AttributeController::convertfloatTo0PointedQString(float value) const
{
  QString valueString(QLocale(QLocale()).toString(value, 'f', 0));
  valueString.remove(".");
  valueString.remove(",");
  return valueString;
}

float AttributeController::convertQStringToFloat(QString value) const
{
  if(value.isEmpty())
  {
    return 0;
  }
  bool conversionSuccess;
  float convertedValue = QLocale().toFloat(value, &conversionSuccess);
  if(!conversionSuccess)
  {
    QString errorCode ("conversion to float fails with value: " + value);
    throw std::logic_error(errorCode.toStdString());
  }
  return convertedValue;
}

LaserPoint* AttributeController::sourcePointFromTrajectory()
{
    if (auto sourcePoint = m_selectedTrajectory->getSource())
    {
        if (auto laserPoint = qobject_cast<LaserPoint*>(sourcePoint))
        {
            return laserPoint;
        }
    }
    return {};
}

LaserPoint* AttributeController::destinationPointFromTrajectory()
{
    if (auto destinationPoint = m_selectedTrajectory->getDestination())
    {
        if (auto laserPoint = qobject_cast<LaserPoint*>(destinationPoint))
        {
            return laserPoint;
        }
    }
    return {};
}
