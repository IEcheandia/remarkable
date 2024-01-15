#pragma once

#include <QObject>

namespace precitec
{

namespace scantracker
{

namespace components
{

namespace wobbleFigureEditor
{
class WobbleFigure;
class LaserPoint;

class SelectionHandler : public QObject
{
    Q_OBJECT
    Q_PROPERTY(precitec::scantracker::components::wobbleFigureEditor::WobbleFigure* figure READ figure WRITE setFigure NOTIFY figureChanged)

public:
    explicit SelectionHandler(QObject* parent = nullptr);
    ~SelectionHandler() override;

    WobbleFigure* figure() const
    {
        return m_figure;
    }
    void setFigure( WobbleFigure* newFigure);

    Q_INVOKABLE void addLaserPoint(QObject* newLaserPoint);
    Q_INVOKABLE void resetLaserPoints();
    // TODO: test this methods
    double getLaserPower(int ID);
    double getLaserRingPower (int ID);
    double getVelocity(int ID);

    std::pair<LaserPoint*, LaserPoint*> getPoints();
    int getMaxNumberOfPoints();

Q_SIGNALS:
    void figureChanged();
    void selectionChanged();

private:
    WobbleFigure* m_figure = nullptr;
    QMetaObject::Connection m_figureDestroyedConnection;
    std::pair<LaserPoint*, LaserPoint*> m_startAndEndPoints;
};

}
}
}
}

