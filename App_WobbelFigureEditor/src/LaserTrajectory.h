#pragma once

#include <QuickQanava.h>

namespace precitec
{

namespace scantracker
{

namespace components
{

namespace wobbleFigureEditor
{

class LaserTrajectory : public qan::Edge
{
    Q_OBJECT
    Q_PROPERTY(int ID READ ID WRITE setID NOTIFY IDChanged)
    Q_PROPERTY(bool editable READ editable WRITE setEditable NOTIFY editableChanged)
    Q_PROPERTY(double speed READ speed WRITE setSpeed NOTIFY speedChanged)
    Q_PROPERTY(double trajectoryLength READ trajectoryLength WRITE setTrajectoryLength NOTIFY trajectoryLengthChanged)
    Q_PROPERTY(double time READ time WRITE setTime NOTIFY timeChanged)
    Q_PROPERTY(int group READ group WRITE setGroup NOTIFY groupChanged)
    Q_PROPERTY(int type READ type WRITE setType NOTIFY typeChanged)

    Q_PROPERTY(bool isRampEdge READ isRampEdge WRITE setIsRampEdge NOTIFY isRampEdgeChanged)
    Q_PROPERTY(bool hasGradient READ hasGradient WRITE setHasGradient NOTIFY hasGradientChanged)
    Q_PROPERTY(QColor startColor READ startColor WRITE setStartColor NOTIFY startColorChanged)
    Q_PROPERTY(QColor endColor READ endColor WRITE setEndColor NOTIFY endColorChanged)
    Q_PROPERTY(double startPowerPosition READ startPowerPosition WRITE setStartPowerPosition NOTIFY startPowerPositionChanged)
    Q_PROPERTY(double endPowerPosition READ endPowerPosition WRITE setEndPowerPosition NOTIFY endPowerPositionChanged)
    Q_PROPERTY(double recStrength READ recStrength WRITE setRecStrength NOTIFY recStrengthChanged)


public:
    explicit LaserTrajectory(QObject* parent = nullptr);
    ~LaserTrajectory() override;

    int ID() const;
    void setID(const int id);
    bool editable() const;
    void setEditable(bool edit);
    double speed() const;
    void setSpeed(const double& scannerSpeed);
    double trajectoryLength() const;
    void setTrajectoryLength(const double& length);
    double time() const;
    void setTime(const double& newTime);
    int group() const;
    void setGroup(int newGroup);
    int type() const;
    void setType(int newVectorType);

    bool isRampEdge() const;
    void setIsRampEdge(bool isRamp);

    bool hasGradient() const;
    void setHasGradient(bool hasNewGradient);

    QColor startColor() const;
    void setStartColor(QColor newStartColor);
    QColor endColor() const;
    void setEndColor(QColor newEndColor);
    double startPowerPosition() const;
    void setStartPowerPosition(double newStartPowerPosition);
    double endPowerPosition() const;
    void setEndPowerPosition(double newEndPowerPosition);
    double recStrength() const;
    void setRecStrength(double newRecStrength);

    static QQmlComponent* delegate(QQmlEngine& engine, QObject* parent = nullptr);

Q_SIGNALS:
    void IDChanged();
    void editableChanged();
    void speedChanged();
    void trajectoryLengthChanged();
    void timeChanged();
    void groupChanged();
    void typeChanged();
    void isRampEdgeChanged();
    void hasGradientChanged();
    void startColorChanged();
    void endColorChanged();
    void startPowerPositionChanged();
    void endPowerPositionChanged();
    void recStrengthChanged();


private:
    QString m_name;
    int m_ID;
    bool m_editable = false;
    double m_speed;
    double m_length;
    double m_time;
    int m_group;
    int m_type;
    bool m_isRampEdge{false};
    inline static bool sm_isRampEdge;
    QColor m_startColor;
    QColor m_endColor;
    double m_startPowerPosition{0.0};
    double m_endPowerPosition{1.0};
    bool m_hasGradient{false};
    double m_recStrength{4};
};
}
}
}
}
QML_DECLARE_TYPE(precitec::scantracker::components::wobbleFigureEditor::LaserTrajectory)
