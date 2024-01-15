#pragma once

#include "LaserPoint.h"
#include "LaserTrajectory.h"
#include "WobbleFigure.h"
#include "WobbleFigureEditor.h"
#include "selectionHandler.h"

namespace precitec
{

namespace scantracker
{

namespace components
{

namespace wobbleFigureEditor
{
class WobbleFigure;

class AttributeController : public QObject
{
  // TODO: test this class
  Q_OBJECT
  Q_PROPERTY(QObject* selection READ selection WRITE setSelection NOTIFY selectionChanged)
  Q_PROPERTY(QObject* selectionHandler READ selectionHandler WRITE setSelectionHandler NOTIFY selectionHandlerChanged)

  Q_PROPERTY(int figureScale READ figureScale WRITE setFigureScale NOTIFY figureScaleChanged)

  Q_PROPERTY(LaserPoint* selectedPoint READ selectedPoint WRITE setSelectedPoint NOTIFY selectedPointChanged)
  Q_PROPERTY(LaserTrajectory* selectedTrajectory READ selectedTrajectory WRITE setSelectedTrajectory NOTIFY selectedTrajectoryChanged)
  Q_PROPERTY(precitec::scantracker::components::wobbleFigureEditor::WobbleFigure* figure READ figure WRITE setFigure NOTIFY figureChanged)
  Q_PROPERTY(QString laserPointID READ laserPointID NOTIFY laserPointIDChanged)
  Q_PROPERTY(QString laserPointCoordinateX READ laserPointPositionX WRITE setLaserPointPositionX NOTIFY laserPointCoordinateXChanged)
  Q_PROPERTY(QString laserPointCoordinateY READ laserPointPositionY WRITE setLaserPointPositionY NOTIFY laserPointCoordinateYChanged)
  Q_PROPERTY(QString laserPointLaserPower READ laserPointLaserPower WRITE setLaserPointLaserPower NOTIFY laserPointLaserPowerChanged)
  Q_PROPERTY(QString currentlaserPointLaserPower READ currentlaserPointLaserPower NOTIFY currentlaserPointLaserPowerChanged)
  Q_PROPERTY(QString laserPointRingPower READ laserPointRingPower WRITE setLaserPointRingPower NOTIFY laserPointRingPowerChanged)
  Q_PROPERTY(QString currentlaserPointRingPower READ currentlaserPointRingPower NOTIFY currentlaserPointRingPowerChanged)
  Q_PROPERTY(QString laserPointVelocity READ laserPointVelocity WRITE setLaserPointVelocity NOTIFY laserPointVelocityChanged)
  Q_PROPERTY(QString currentlaserPointVelocity READ currentlaserPointVelocity NOTIFY currentlaserPointVelocityChanged)

public:
  explicit AttributeController( QObject* parent = nullptr);
  ~AttributeController() override;

  QObject* selection() const;
  void setSelection(QObject* object);
  QObject* selectionHandler() const;
  void setSelectionHandler(QObject* object);

  int figureScale() const;
  void setFigureScale(int newFactor);

  LaserPoint* selectedPoint() const;
  void setSelectedPoint(LaserPoint* point);
  LaserTrajectory* selectedTrajectory() const;
  void setSelectedTrajectory(LaserTrajectory* trajectory);

  void setFigure( WobbleFigure* newFigure);
  precitec::scantracker::components::wobbleFigureEditor::WobbleFigure* figure() const;

  QString laserPointID() const;
  QString laserPointPositionX() const;
  void setLaserPointPositionX(QString value);
  QString laserPointPositionY() const;
  void setLaserPointPositionY(QString value);
  QString laserPointLaserPower() const;
  void setLaserPointLaserPower(QString value);
  QString currentlaserPointLaserPower() const;
  QString laserPointRingPower() const;
  void setLaserPointRingPower(QString value);
  QString currentlaserPointRingPower() const;
  QString laserPointVelocity() const;
  void setLaserPointVelocity(QString value);
  QString currentlaserPointVelocity() const;

  Q_INVOKABLE bool isLaserPointCurrentLaserPowerDependOnThePreviousPoint();
  Q_INVOKABLE bool isLaserPointCurrentRingPowerDependOnThePreviousPoint();
  Q_INVOKABLE bool isLaserPointCurrentVelocityDependOnThePreviousPoint();
  Q_INVOKABLE bool isLastPointOfFigure() const;
  Q_INVOKABLE void emitSignalsToUpdatePointPosition();
  Q_INVOKABLE void emitSignalsToUpdatePointProperties();

  Q_INVOKABLE void resetSelection();

  void changeLaserPointIDFromRampTrajectory();

Q_SIGNALS:
  void selectionChanged();
  void selectionHandlerChanged();

  void figureScaleChanged();

  void selectedPointChanged();
  void selectedTrajectoryChanged();

  void updated(QObject* object);

  void figureChanged();

  void laserPointIDChanged();
  void laserPointCoordinateXChanged();
  void laserPointCoordinateYChanged();
  void laserPointLaserPowerChanged();
  void currentlaserPointLaserPowerChanged();
  void laserPointRingPowerChanged();
  void currentlaserPointRingPowerChanged();
  void laserPointVelocityChanged();
  void currentlaserPointVelocityChanged();

  void loadRampDialog();

private:
  // TODO: outsource in a utils class or something else
  QString convertfloatTo2PointedQString(float value) const;
  QString convertfloatTo0PointedQString(float value) const;
  float convertQStringToFloat(QString value) const;

  LaserPoint* sourcePointFromTrajectory();
  LaserPoint* destinationPointFromTrajectory();

private:
  QObject* m_selection = nullptr;
  SelectionHandler* m_selectionHandler = nullptr;
  WobbleFigure* m_figure = nullptr;
  WobbleFigureEditor* m_wobbleFigureEditor = nullptr;

  int m_figureScale = 1000;

  LaserPoint* m_selectedPoint = nullptr;
  LaserTrajectory* m_selectedTrajectory = nullptr;
};

}
}
}
}
