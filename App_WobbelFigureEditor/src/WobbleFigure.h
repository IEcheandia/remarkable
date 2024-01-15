#pragma once

#include <QuickQanava.h>
#include <QString>

namespace precitec
{

namespace scantracker
{

namespace components
{

namespace wobbleFigureEditor
{
class LaserPoint;
class LaserTrajectory;

class WobbleFigure : public qan::Graph
{
    Q_OBJECT
    Q_PROPERTY(QString name READ name WRITE setName NOTIFY nameChanged)
    Q_PROPERTY(int ID READ ID WRITE setID NOTIFY IDChanged)
    Q_PROPERTY(QString description READ description WRITE setDescription NOTIFY descriptionChanged)
    Q_PROPERTY(bool analogPower READ analogPower WRITE setAnalogPower NOTIFY analogPowerChanged)

public:
    explicit WobbleFigure( QQuickItem* parent = nullptr) : qan::Graph(parent) {}
    ~WobbleFigure() override;

    QString name() const;
    void setName(const QString &name);
    int ID() const;
    void setID(int id);
    QString description() const;
    void setDescription(const QString &description);
    bool analogPower() const;
    void setAnalogPower(bool isAnalogPower);

    Q_INVOKABLE precitec::scantracker::components::wobbleFigureEditor::LaserPoint* searchPoint(int ID);

    void setLaserPoint(LaserPoint* actualPoint);
    LaserPoint* searchLaserPoint(int ID);
    void setLaserTrajectory(LaserTrajectory* actualConnection);
    LaserTrajectory* searchLaserTrajectory(int ID);
    void resetLaserPoints();                            //Used by selection handler, wobble figure editor, wobble figure view (qml) and main view (qml)
    std::vector<LaserPoint*> getLaserPoints();          //Used by wobble figure data model
    std::vector<LaserPoint*> rampPoints();

    Q_INVOKABLE qan::Node* insertLaserPoint(QQmlComponent* nodeComponent = nullptr);        //Used to insert laser points.
    Q_INVOKABLE qan::Node* insertLaserPointExact(const QPointF &position, int scale);
    Q_INVOKABLE qan::Edge* insertConnection(qan::Node* source, qan::Node* destination, QQmlComponent* edgeComponent = nullptr);

Q_SIGNALS:
    void nameChanged();
    void IDChanged();
    void descriptionChanged();
    void analogPowerChanged();
    void mouseEnteredPoint(int id);
    void mouseExitedPoint(int id);

private:
    QString m_name{tr("Figure name")};
    int m_ID{0};
    QString m_description;
    bool m_analogPower{true};

    std::vector<LaserPoint*> m_laserPoints;
    std::vector<QQmlComponent*> m_laserPointsQml;
    std::vector<LaserTrajectory*> m_laserTrajectories;
};

}
}
}
}
QML_DECLARE_TYPE(precitec::scantracker::components::wobbleFigureEditor::WobbleFigure)
