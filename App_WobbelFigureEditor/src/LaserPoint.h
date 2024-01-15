#pragma once

#include <QuickQanava.h>
#include "fileType.h"
#include <QLinearGradient>

namespace precitec
{

namespace scantracker
{

namespace components
{

namespace wobbleFigureEditor
{

class LaserPoint : public qan::Node
{
    Q_OBJECT
    Q_PROPERTY(int ID READ ID WRITE setID NOTIFY IDChanged)
    Q_PROPERTY(bool editable READ editable WRITE setEditable NOTIFY editableChanged)
    Q_PROPERTY(double laserPower READ laserPower WRITE setLaserPower NOTIFY laserPowerChanged)
    Q_PROPERTY(double ringPower READ ringPower WRITE setRingPower NOTIFY ringPowerChanged)
    Q_PROPERTY(double velocity READ velocity WRITE setVelocity NOTIFY velocityChanged)
    Q_PROPERTY(FileType type READ type WRITE setType NOTIFY typeChanged)
    Q_PROPERTY(QColor color READ color WRITE setColor NOTIFY colorChanged)
    Q_PROPERTY(QPointF center READ center WRITE setCenter NOTIFY centerChanged)

    Q_PROPERTY(bool isRampPoint READ isRampPoint WRITE setIsRampPoint NOTIFY isRampPointChanged)

public:
    explicit LaserPoint(QObject* parent = nullptr);
    ~LaserPoint() override;

    int ID() const;
    void setID(const int id);
    bool editable() const;
    void setEditable(bool edit);
    double laserPower() const;
    void setLaserPower(double power);
    double ringPower() const
    {
        return m_ringPower;
    }
    void setRingPower(double newPower);

    double velocity() const
    {
        return m_velocity;
    }
    void setVelocity(double newVelocity);

    FileType type() const
    {
        return m_type;
    }
    void setType(FileType newType);

    QColor color() const;
    void setColor(const QColor& newColor);
    QPointF center() const;
    void setCenter(const QPointF& center);

    bool isRampPoint() const
    {
        return m_isRampPoint;
    }
    void setIsRampPoint(bool rampEndPoint);

    //Position and Size = Geometry
    QRectF geometry() const;
    void setGeometry(const QRectF& geometry);

    void initConnectionToNodeItem();
    void modifyAttachedEdge(bool startModifyingFollowedPoint = false, bool modifingFromRamp = false);
    void startModifyIngAttachedEdge();

    void setLineStyle(bool dashedLine = false, QColor lineColor = Qt::black);

    static QQmlComponent* delegate(QQmlEngine& engine);

Q_SIGNALS:
    void IDChanged();
    void editableChanged();
    void laserPowerChanged();
    void ringPowerChanged();
    void velocityChanged();
    void typeChanged();
    void colorChanged();
    void centerChanged();
    void isRampPointChanged();

private:
    void setEditableDependingOnLaserPower();
    void setEdgeColorLikeEdgeBefore();
    void setEdgeStyleOfAllFollowingNodes();

    int m_ID = -1;
    bool m_editable = false;

    double m_laserPower = -1.0;
    double m_ringPower = -1.0;
    double m_velocity = -1.0;
    FileType m_type{FileType::None};
    QColor m_color = Qt::black;

    // EdgeStyle
    QLinearGradient m_gradient;
    qan::EdgeStyle m_ownEdgeStyle;
    bool m_isRampPoint{false};
};

}
}
}
}
QML_DECLARE_TYPE(precitec::scantracker::components::wobbleFigureEditor::LaserPoint)
