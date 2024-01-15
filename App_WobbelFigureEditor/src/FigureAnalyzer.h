#pragma once

#include <QObject>
#include <QPointF>
#include "editorDataTypes.h"

class FigureAnalyzerTest;
class PlausibilityCheckerTest;

namespace precitec
{
namespace scanmaster
{
namespace components
{
namespace wobbleFigureEditor
{
class SimulationController;
}
}
}
namespace scantracker
{
namespace components
{
namespace wobbleFigureEditor
{

class WobbleFigureEditor;

/**
* Class that analyzes the properties of the seam, wobble or overlay figure.
**/
class FigureAnalyzer : public QObject
{
    Q_OBJECT

    /**
     * Wobble figure editor which contains the information of the loaded file.
     * Is used to trigger the figure analyzer for example to update the properties.
     **/
    Q_PROPERTY(precitec::scantracker::components::wobbleFigureEditor::WobbleFigureEditor* figureEditor READ figureEditor WRITE setFigureEditor NOTIFY figureEditorChanged)

    /**
     * Simulation controller which contains the information of the simulated figure.
     * Is used to trigger the figure analyzer for example to update the properties.
     **/
    Q_PROPERTY(precitec::scanmaster::components::wobbleFigureEditor::SimulationController* simulationController READ simulationController WRITE setSimulationController NOTIFY simulationControllerChanged)

    /**
     * Number of points from selected or simulated figure.
     **/
    Q_PROPERTY(unsigned int pointCount READ pointCount NOTIFY pointCountChanged)

    /**
     * Length of selected or simulated figure.
     **/
    Q_PROPERTY(double routeLength READ routeLength NOTIFY routeLengthChanged)

    /**
     * Start position of selected or simulated figure.
     **/
    Q_PROPERTY(QPointF startPoint READ startPoint NOTIFY startPointChanged)

    /**
     * End position of selected or simulated figure.
     **/
    Q_PROPERTY(QPointF endPoint READ endPoint NOTIFY endPointChanged)

    /**
     * Width (X-Axis) of selected or simulated figure.
     **/
    Q_PROPERTY(double figureWidth READ figureWidth NOTIFY figureWidthChanged)

    /**
     * Height (Y-Axis) of selected or simulated figure.
     **/
    Q_PROPERTY(double figureHeight READ figureHeight NOTIFY figureHeightChanged)

    /**
     * How long the selected or simulated figure takes to execute.
     * The speed is taken from the file or the global simulation speed from the figure editor settings.
     **/
    Q_PROPERTY(double figureTime READ figureTime NOTIFY figureTimeChanged)

    /**
     * How far apart two different sweep/wobble periods are. Formular is gap = frequency / speed.
     **/
    Q_PROPERTY(double wobblePointDistance READ wobblePointDistance NOTIFY wobblePointDistanceChanged)

    Q_PROPERTY(double minFocusSpeed READ minFocusSpeed NOTIFY focusSpeedChanged)

    Q_PROPERTY(double maxFocusSpeed READ maxFocusSpeed NOTIFY focusSpeedChanged)

    Q_PROPERTY(double averageFocusSpeed READ averageFocusSpeed NOTIFY focusSpeedChanged)

public:
    explicit FigureAnalyzer( QObject* parent = nullptr);
    ~FigureAnalyzer() override;

    struct FigureInformation
    {
        std::pair<double, double> coordinates;
        double speed;
    };

    struct FigurePartInformation
    {
        double length;
        double speed;
        double time;
    };

    WobbleFigureEditor* figureEditor() const
    {
        return m_figureEditor;
    }
    void setFigureEditor(WobbleFigureEditor* newFigureEditor);

    precitec::scanmaster::components::wobbleFigureEditor::SimulationController* simulationController() const
    {
        return m_simulationController;
    }
    void setSimulationController(precitec::scanmaster::components::wobbleFigureEditor::SimulationController* newSimulationController);

    unsigned int pointCount() const
    {
        return m_pointCount;
    }

    double routeLength() const
    {
        return m_routeLength;
    }

    QPointF startPoint() const
    {
        return m_startPoint;
    }

    QPointF endPoint() const
    {
        return m_endPoint;
    }

    double figureWidth() const
    {
        return m_figureWidth;
    }

    double figureHeight() const
    {
        return m_figureHeight;
    }

    double figureTime() const
    {
        return m_figureTime;
    }

    std::vector<FigurePartInformation> routeParts() const
    {
        return m_routeParts;
    }

    double wobblePointDistance() const
    {
        return m_wobblePointDistance;
    }

    double minFocusSpeed() const
    {
        return m_minFocusSpeed;
    }

    double maxFocusSpeed() const
    {
        return m_maxFocusSpeed;
    }

    double averageFocusSpeed() const
    {
        return m_averageFocusSpeed;
    }

    Q_INVOKABLE void updateProperties();

Q_SIGNALS:
    void figureEditorChanged();
    void simulationControllerChanged();
    void pointCountChanged();
    void routeLengthChanged();
    void startPointChanged();
    void endPointChanged();
    void figureWidthChanged();
    void figureHeightChanged();
    void figureTimeChanged();
    void wobblePointDistanceChanged();
    void focusSpeedChanged();

    void reset();
    void fileChanged();

private:
    void resetInformation();
    void updateSimulationProperties();

    bool getInformationFromFigureEditor();
    void castSeamToRouteInformation(const std::vector<RTC6::seamFigure::command::Order>& figure);
    void castWobbleToRouteInformation(const std::vector<RTC6::wobbleFigure::command::Order>& figure);
    void castOverlayToRouteInformation(const std::vector<std::pair<double, double>>& figure);
    bool getInformationFromSimulationController();

    void updatePointCount();
    void updateStartPoint();
    void updateEndPoint();
    void updateFigureDimensions();
    void updateRouteProperties();
    void updateWobblePointDistance();
    void updateFocusSpeed();

    double calculateEuclideanDistance(const QPointF &firstPoint, const QPointF &secondPoint);
    double threeDigitLimitation(double value);

    WobbleFigureEditor* m_figureEditor = nullptr;
    QMetaObject::Connection m_figureEditorDestroyedConnection;
    scanmaster::components::wobbleFigureEditor::SimulationController* m_simulationController = nullptr;
    QMetaObject::Connection m_simulationControllerDestroyedConnection;

    unsigned int m_pointCount{0};
    double m_routeLength{0.0};
    QPointF m_startPoint{0.0, 0.0};
    QPointF m_endPoint{0.0, 0.0};
    double m_figureWidth{0.0};
    double m_figureHeight{0.0};
    double m_figureTime{0.0};
    bool m_isWobbleFigure{false};
    double m_wobblePointDistance{0.0};
    unsigned int m_wobbleFigurePointCount{0};
    unsigned int m_wobbleFigurePointSize{0};
    unsigned int m_microVectorFactor{0};

    double m_minFocusSpeed{0.0};
    double m_maxFocusSpeed{0.0};
    double m_averageFocusSpeed{0.0};

    std::vector<FigurePartInformation> m_routeParts;
    std::vector<FigureInformation> m_routeInformation;

    friend FigureAnalyzerTest;
    friend PlausibilityCheckerTest;
};

}
}
}
}

Q_DECLARE_METATYPE(precitec::scantracker::components::wobbleFigureEditor::FigureAnalyzer::FigurePartInformation);
Q_DECLARE_METATYPE(precitec::scantracker::components::wobbleFigureEditor::FigureAnalyzer::FigureInformation)    //Avoid type isn't declared in figureanalyzerTest.cpp at line 349
Q_DECLARE_METATYPE(precitec::scantracker::components::wobbleFigureEditor::FigureAnalyzer*)
