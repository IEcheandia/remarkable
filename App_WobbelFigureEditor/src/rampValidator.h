#pragma once

#include <QObject>
#include <QtGlobal>

#include "WobbleFigureEditor.h"
#include "laserPointController.h"

#include "editorDataTypes.h"

using precitec::scantracker::components::wobbleFigureEditor::WobbleFigureEditor;
using precitec::scanmaster::components::wobbleFigureEditor::LaserPointController;
using RTC6::seamFigure::Ramp;

class RampValidatorTest;

namespace precitec
{
namespace scanmaster
{
namespace components
{
namespace wobbleFigureEditor
{

/**
 *  The ramp validator calculates how long a new or existing ramp can be without overlapping another ramp.
 *  The second task is to calculate the end or start point of the ramps to visualize them.
 **/
class RampValidator : public QObject
{
    Q_OBJECT
    /**
     * Wobble figure editor holds all necessary data which are used to check if the ramp is valid and to visualize the ramps. (Necessary data are seam points and ramp information)
     **/
    Q_PROPERTY(precitec::scantracker::components::wobbleFigureEditor::WobbleFigureEditor* figureEditor READ figureEditor WRITE setFigureEditor NOTIFY figureEditorChanged)
    /**
     *  Contains at which point the ramp starts.
     **/
    Q_PROPERTY(precitec::scanmaster::components::wobbleFigureEditor::LaserPointController* laserPointController READ laserPointController WRITE setLaserPointController NOTIFY laserPointControllerChanged)
    /**
     *  Contains at which point the ramp starts.
     **/
    Q_PROPERTY(int startPointID READ startPointID WRITE setStartPointID NOTIFY startPointIDChanged)
    /**
     *  Returns if the selected point is already in a ramp and cannot be used to start a new one.
     **/
    Q_PROPERTY(bool isPointAlreadyInRamp READ isPointAlreadyInRamp WRITE setIsPointAlreadyInRamp NOTIFY isPointAlreadyInRampChanged)
    /**
     *  Returns if the selected point is a start point of a ramp. If so then the ramp dialog will be opened to get the possibility to modify or delete the selected ramp.
     **/
    Q_PROPERTY(bool isPointAStartPoint READ isPointAStartPoint WRITE setIsPointAStartPoint NOTIFY isPointAStartPointChanged)
    /**
     *  Contains the max ramp length from the selected point to the next point where a ramp begins.
     **/
    Q_PROPERTY(double maxRampLength READ maxRampLength WRITE setMaxRampLength NOTIFY maxRampLengthChanged)
    /**
     *  Contains if the length of the ramp is entered in millimeters or in degree.
     **/
    Q_PROPERTY(LengthType enterLengthInDegree READ enterLengthInDegree WRITE setEnterLengthInDegree NOTIFY enterLengthInDegreeChanged)

    Q_PROPERTY(QString lengthLabel READ lengthLabel NOTIFY lengthLabelChanged)

public:
    explicit RampValidator(QObject* parent = nullptr);
    ~RampValidator() override;

    enum LengthType
    {
        Millimeter,
        Degree,
        Percent
    };
    Q_ENUMS(LengthType)

    WobbleFigureEditor* figureEditor() const
    {
        return m_figureEditor;
    }
    void setFigureEditor(WobbleFigureEditor* newFigureEditor);

    LaserPointController* laserPointController() const
    {
        return m_laserPointController;
    }
    void setLaserPointController(LaserPointController* newPointController);

    int startPointID() const
    {
        return m_startPointID;
    }
    void setStartPointID(int newID);

    bool isPointAlreadyInRamp() const
    {
        return m_isPointAlreadyInRamp;
    }
    void setIsPointAlreadyInRamp(bool isPointInOneRamp);

    bool isPointAStartPoint() const
    {
        return m_isPointAStartPoint;
    }
    void setIsPointAStartPoint(bool isStartPointOfARamp);

    double maxRampLength() const;

    void setMaxRampLength(double maxLengthForCurrentRamp);

    LengthType enterLengthInDegree() const
    {
        return m_enterLengthInDegree;
    }
    QString lengthLabel() const;
    QString unitLabel() const;

    void setEnterLengthInDegree(LengthType enterInDegree);

    std::vector<QVector2D> seamPoints() const
    {
        return m_seamPoints;
    }
    void setSeamPoints(const std::vector<QVector2D>& points);

    std::vector<Ramp> ramps() const
    {
        return m_ramps;
    }
    void setRamps(const std::vector<Ramp>& currentRamps);

    std::vector<QVector3D> endPoints() const
    {
        return m_endPoints;
    }

    Q_INVOKABLE double convertLengthToDegree(double value) const;
    Q_INVOKABLE double convertDegreeToLength(double value) const;
    Q_INVOKABLE double convertLengthToPercent(double value) const;
    Q_INVOKABLE double getRampLenght(double value) const;
    Q_INVOKABLE double getValueInMillimeter(double value) const;

Q_SIGNALS:
    void figureEditorChanged();
    void laserPointControllerChanged();
    void startPointIDChanged();
    void isPointAlreadyInRampChanged();
    void isPointAStartPointChanged();
    void maxRampLengthChanged();
    void enterLengthInDegreeChanged();
    void lengthLabelChanged();

    void seamPointsChanged();
    void rampsChanged();
    void endPointsChanged();

private:
    void takeSeamPoints();
    std::vector<QVector2D> castOrderPositionToVector(const std::vector<RTC6::seamFigure::command::Order>& orders);
    void takeRamps();
    bool necessaryInfoAvailable();
    void clearEndPoints();
    QVector3D calculateEndPoint(std::size_t startPointID, double rampLength);
    QVector3D calculateEndPointPosition(std::size_t startPointID, double rampLength);
    void calculateEndPoints();
    void hasRampOut();
    void initRamps();

    bool findPointInRamps();
    bool checkPointIsInRampOut();
    bool checkPointIsInRamp();
    void checkPointAlreadyInRamp();
    bool checkPointIsStartPoint(int toBeCheckedID);

    void updateMaxRampLength();
    std::size_t searchNextRamp(std::size_t startPointID);
    bool checkIDBelongsToRampOut(std::size_t checkedID);
    double lengthFromRamp(std::size_t startPointID);
    double calculateLengthPointToPoint(std::size_t startPointID, std::size_t targetPointID);

    void calculateWholeLength();

    void drawRamps();

    int m_startPointID{-1};
    bool m_isPointAlreadyInRamp{false};
    bool m_isPointAStartPoint{false};
    bool m_rampsHasRampOut{false};
    double m_maxRampLength{0.0};
    LengthType m_enterLengthInDegree{LengthType::Millimeter};
    double m_wholeLength{0.0};

    std::vector<QVector2D> m_seamPoints;
    std::vector<Ramp> m_ramps;
    std::vector<QVector3D> m_endPoints;

    WobbleFigureEditor* m_figureEditor = nullptr;
    QMetaObject::Connection m_figureEditorDestroyedConnection;
    LaserPointController* m_laserPointController = nullptr;
    QMetaObject::Connection m_laserPointControllerDestroyedConnection;

    friend RampValidatorTest;
};

}
}
}
}
