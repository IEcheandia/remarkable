#pragma once

#include <QObject>

#include "WobbleFigureEditor.h"

#include "editorDataTypes.h"

using precitec::scantracker::components::wobbleFigureEditor::WobbleFigureEditor;
using RTC6::seamFigure::Ramp;

class RampModelTest;

namespace precitec
{
namespace scanmaster
{
namespace components
{
namespace wobbleFigureEditor
{

/**
 *  The ramp model stores all properties to create or delete a ramp.
 **/
class RampModel : public QObject
{
    Q_OBJECT
    /**
     *  Wobble figure editor holds the ramp data. After adding, updating or removing a ramp the wobble figure editor has to be updated.
     **/
    Q_PROPERTY(precitec::scantracker::components::wobbleFigureEditor::WobbleFigureEditor* figureEditor READ figureEditor WRITE setFigureEditor NOTIFY figureEditorChanged)
    /**
     *  Returns at which point the ramp starts.
     **/
    Q_PROPERTY(int startPointID READ startPointID WRITE setStartPointID NOTIFY startPointIDChanged)
    /**
     *  This property holds the length of the ramp.
     **/
    Q_PROPERTY(double rampLength READ rampLength WRITE setRampLength NOTIFY rampLengthChanged)
    /**
     *  Contains the power at the first point.
     **/
    Q_PROPERTY(double startPower READ startPower WRITE setStartPower NOTIFY startPowerChanged)
    /**
     *  Contains the power at the last point.
     **/
    Q_PROPERTY(double endPower READ endPower WRITE setEndPower NOTIFY endPowerChanged)
    /**
     *  Contains the ring power at the first point.
     **/
    Q_PROPERTY(double startPowerRing READ startPowerRing WRITE setStartPowerRing NOTIFY startPowerRingChanged)
    /**
     *  Contains the ring power at the last point.
     **/
    Q_PROPERTY(double endPowerRing READ endPowerRing WRITE setEndPowerRing NOTIFY endPowerRingChanged)

public:
    explicit RampModel(QObject* parent = nullptr);
    ~RampModel() override;

    WobbleFigureEditor* figureEditor() const
    {
        return m_figureEditor;
    }
    void setFigureEditor(WobbleFigureEditor* newFigureEditor);

    int startPointID() const
    {
        return m_startPointID;
    }
    void setStartPointID(int newStartPointID);

    double rampLength() const
    {
        return m_rampLength;
    }
    void setRampLength(double newRampLength);

    double startPower() const
    {
        return m_startPower;
    }
    void setStartPower(double newStartPower);

    double endPower() const
    {
        return m_endPower;
    }
    void setEndPower(double newEndPower);

    double startPowerRing() const
    {
        return m_startPowerRing;
    }
    void setStartPowerRing(double newStartRingPower);

    double endPowerRing() const
    {
        return m_endPowerRing;
    }
    void setEndPowerRing(double newEndRingPower);

    std::vector<Ramp> ramps() const
    {
        return m_ramps;
    }
    void setRamps(const std::vector<Ramp>& currentRamps);

    Q_INVOKABLE void updateRamps();
    Q_INVOKABLE void eraseRamp();

Q_SIGNALS:
    void figureEditorChanged();
    void startPointIDChanged();
    void rampLengthChanged();
    void startPowerChanged();
    void endPowerChanged();
    void startPowerRingChanged();
    void endPowerRingChanged();
    void rampsChanged();

private:
    void takeRamps();
    void giveBackRamps();
    void reset();
    void updateRampModel();
    std::vector<Ramp>::iterator searchRamp();
    bool foundRamp(std::vector<Ramp>::iterator rampIt);
    void updateRampModelProperties(const Ramp& InfoToUpdated);
    void setRampAlreadyExists(bool exists);
    void appendRamp();
    void sortRamps();
    void updateFoundRamp(std::vector<Ramp>::iterator foundRampIt);
    bool isStartPointIDValid();

    int m_startPointID {-1};
    double m_rampLength {0.0};
    double m_startPower {0.0};
    double m_endPower {0.0};
    double m_startPowerRing {0.0};
    double m_endPowerRing {0.0};

    std::vector<Ramp> m_ramps;
    WobbleFigureEditor* m_figureEditor = nullptr;
    QMetaObject::Connection m_figureEditorDestroyedConnection;

    friend RampModelTest;
};

}
}
}
}
