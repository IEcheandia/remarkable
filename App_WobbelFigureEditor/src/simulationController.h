#pragma once

#include <QObject>
#include <QtMath>

#include "fileType.h"
#include "editorDataTypes.h"

class SimulationControllerTest;
class FigureAnalyzerTest;

namespace precitec
{

namespace scantracker
{
namespace components
{
namespace wobbleFigureEditor
{
class FileModel;
}
}
}
namespace scanmaster
{
namespace components
{
namespace wobbleFigureEditor
{

class LaserPointController;

using precitec::scantracker::components::wobbleFigureEditor::FileType;

/**
 * The controller performs all necessary tasks in the simulation.
 * A simulation is the interference of a seam with a wobble figure and describes the real path of the scanner or laser on the work piece.
 **/
class SimulationController : public QObject
{
    Q_OBJECT
    /**
     * File model
     * Is used to get the information (positions and powers) of the seam and wobble figure.
     * The two figures are used to generate the superimposed simulated figure.
     **/
    Q_PROPERTY(precitec::scantracker::components::wobbleFigureEditor::FileModel* fileModel READ fileModel WRITE setFileModel NOTIFY fileModelChanged)

    /**
     * Laser point controller object
     * Is used to create the correctly figure from the cpp figure information.
     **/
    Q_PROPERTY(precitec::scanmaster::components::wobbleFigureEditor::LaserPointController* laserPointController READ laserPointController WRITE setLaserPointController NOTIFY laserPointControllerChanged)

    /**
     * Simulation mode
     * Contains the current state of the simulation mode. The GUI changes if the simulation mode is on,
     * because some functions aren't supported in the simulation mode.
     * If the simulation mode changes then the current displayed figure is cleared.
     **/
    Q_PROPERTY(bool simulationMode READ simulationMode WRITE setSimulationMode NOTIFY simulationModeChanged)

    /**
     * Loop count
     * Simulating a superimposed figure can result in a very large number of points which needs to be created. To be able
     * to understand or at least see some periods of the simulated figure the loop count property is used. The loop count is used
     * to determine the number of periods that are drawn in the figure editor.
     **/
    Q_PROPERTY(unsigned int loopCount READ loopCount WRITE setLoopCount NOTIFY loopCountChanged)

    /**
     * 10us factor
     * This factor is used to be able to display even very large simulated figures. 10 us is the clock of the wobble figure.
     * To achieve the highest resolution of the simulated figure the temporal basis of the simulation is the clock of the wobble figure.
     * That is why the number of points can increase very quickly with a long seam. With this property a multiple of the 10us is specified.
     **/
    Q_PROPERTY(int tenMicroSecondsFactor READ tenMicroSecondsFactor WRITE setTenMicroSecondsFactor NOTIFY tenMicroSecondsFactorChanged)

    /**
     * Ready
     * This property is true if the simulated figure is calculated and no information are missing. The parameter indicates whether then
     * figure can be drawn with the information passed or if the number of points are less than the limit (maxPointCountForSimulation).
     * The Ok-Button is just enabled if this property is true.
     **/
    Q_PROPERTY(bool ready READ ready WRITE setReady NOTIFY readyChanged)

    /**
     * Number of points from the simulated figure which will be drawn
     * Gives the number of points from the simulated figure which will be drawn. If loop count is not equal to zero then
     * the number of points which are used to draw the periods of the wobble figure is set.
     **/
    Q_PROPERTY(int pointCountSimulationFigure READ pointCountSimulationFigure NOTIFY pointCountSimulationFigureChanged)

    /**
     * Maximum point count for simulation
     * Contains the limit of the number of points which can be drawn in the figure editor without any performance losses.
     * The "pointCountSimulationFigure" is compared with this limit.
     **/
    Q_PROPERTY(int maxPointCountForSimulation READ maxPointCountForSimulation CONSTANT)

public:
    explicit SimulationController( QObject* parent = nullptr);
    ~SimulationController() override;

    enum LoopCount {
        WholeFigure = 0
    };

    precitec::scantracker::components::wobbleFigureEditor::FileModel* fileModel() const
    {
        return m_fileModel;
    }
    void setFileModel(precitec::scantracker::components::wobbleFigureEditor::FileModel* model);

    precitec::scanmaster::components::wobbleFigureEditor::LaserPointController* laserPointController() const
    {
        return m_laserPointController;
    }
    void setLaserPointController(precitec::scanmaster::components::wobbleFigureEditor::LaserPointController* newLaserPointController);

    bool simulationMode() const
    {
        return m_simulationMode;
    }
    void setSimulationMode(bool isSimulation);

    unsigned int loopCount() const
    {
        return m_loopCount;
    }
    void setLoopCount(unsigned int newLoopCount);

    int tenMicroSecondsFactor() const
    {
        return m_tenMicroSecondsFactor;
    }
    void setTenMicroSecondsFactor(int newTenMicroSecondsFactor);

    bool ready() const
    {
        return m_ready;
    }
    void setReady(bool isReady);

    int pointCountSimulationFigure() const
    {
        return m_loopCount == LoopCount::WholeFigure ? m_simulatedSeamFigure.figure.size() / m_tenMicroSecondsFactor : qCeil(static_cast<double> (m_wobbleFigure.figure.size() * m_wobbleFigure.microVectorFactor * m_loopCount) / static_cast<double> (m_tenMicroSecondsFactor));
    }

    int maxPointCountForSimulation() const
    {
        return m_pointCountVisualizationLimit;
    }

    Q_INVOKABLE void getFigureForSimulation(const QString& filename, precitec::scantracker::components::wobbleFigureEditor::FileType type);

    Q_INVOKABLE void showSimulatedFigure();

    Q_INVOKABLE void clear();

    unsigned int wobbleFigurePointSize() const
    {
        return qCeil(static_cast<double> (m_wobbleFigure.microVectorFactor * m_wobbleFigure.figure.size()) / static_cast<double> (m_tenMicroSecondsFactor));
    }

    std::vector<RTC6::seamFigure::command::Order> simulatedSeamFigure() const
    {
        return m_simulatedSeamFigure.figure;
    }

    const std::vector<double>& simulationFocusSpeed() const
    {
        return m_focusSpeed;
    }

    unsigned int microVectorFactor() const
    {
        return m_wobbleFigure.microVectorFactor;
    }

    unsigned int wobbleFigurePointCount() const
    {
        return m_wobbleFigure.figure.size();
    }

Q_SIGNALS:
    void fileModelChanged();
    void laserPointControllerChanged();

    void simulationModeChanged();
    void loopCountChanged();
    void tenMicroSecondsFactorChanged();
    void readyChanged();
    void pointCountSimulationFigureChanged();

    void figureForSimulationChanged();
    void simulationFigureCalculated();

private:
    void startSimulationFigure();
    bool checkOneFigureMissing();
    void calculateSimulationFigure();
    void clearSimulationFigure();
    double firstValidSpeed();
    std::vector<QVector2D> createSeamVelocities();
    std::vector<QVector3D> createWobbleVelocities();
    std::pair<unsigned int, int> calculateVisualizationInformation();
    bool checkTooManyPoints();
    void checkSimulatedFigureReady();
    double calculateFocusSpeed(const QVector2D& velocity);
    double angleRadFromXAxis(const QVector2D& vector);
    QVector2D rotateVectorRad(const QVector2D& vector, double angleRad);

    precitec::scantracker::components::wobbleFigureEditor::FileModel* m_fileModel = nullptr;
    QMetaObject::Connection m_fileModelDestroyedConnection;
    precitec::scanmaster::components::wobbleFigureEditor::LaserPointController* m_laserPointController = nullptr;
    QMetaObject::Connection m_laserPointControllerDestroyedConnection;

    bool m_simulationMode{false};
    unsigned int m_loopCount{LoopCount::WholeFigure};
    int m_tenMicroSecondsFactor{1};

    std::pair<unsigned int, int> m_visualizationInformation{0, 1};

    bool m_ready{false};

    double m_secondsIn10us{0.00001};
    unsigned int m_pointCountVisualizationLimit{3000};

    RTC6::seamFigure::SeamFigure m_seamFigure;
    RTC6::wobbleFigure::Figure m_wobbleFigure;
    RTC6::seamFigure::SeamFigure m_simulatedSeamFigure;

    std::vector<double> m_focusSpeed;

    friend SimulationControllerTest;
    friend FigureAnalyzerTest;
};

}
}
}
}


