#pragma once

#include "WobbleFigure.h"
#include "FileModel.h"
#include "FigureCreator.h"
#include "powerModulationMode.h"
#include "commandManager.h"

using precitec::scanmaster::components::wobbleFigureEditor::powerModulationMode::PowerModulationMode;
using precitec::scantracker::components::wobbleFigureEditor::FileType;
using precitec::scantracker::components::wobbleFigureEditor::CommandManager;
using RTC6::seamFigure::Ramp;

class WobbleFigureEditorTest;
class FigureAnalyzerTest;

namespace precitec
{
namespace scanmaster
{
namespace components
{
namespace wobbleFigureEditor
{
class LaserPointController;
}
}
}
namespace scantracker
{

namespace components
{

namespace wobbleFigureEditor
{

using precitec::scanmaster::components::wobbleFigureEditor::LaserPointController;

class WobbleFigureEditor : public QObject
{
    Q_OBJECT
    Q_PROPERTY(precitec::scantracker::components::wobbleFigureEditor::WobbleFigure* figure READ figure WRITE setFigure NOTIFY figureChanged)
    Q_PROPERTY(precitec::scantracker::components::wobbleFigureEditor::FileModel* fileModel READ fileModel WRITE setFileModel NOTIFY fileModelChanged)
    Q_PROPERTY(precitec::scantracker::components::wobbleFigureEditor::FigureCreator* figureCreator READ figureCreator WRITE setFigureCreator NOTIFY figureCreatorChanged)

    Q_PROPERTY(precitec::scanmaster::components::wobbleFigureEditor::LaserPointController* laserPointController READ laserPointController WRITE setLaserPointController NOTIFY laserPointControllerChanged)

    Q_PROPERTY(precitec::scantracker::components::wobbleFigureEditor::FileType fileType READ fileType NOTIFY fileTypeChanged)
    Q_PROPERTY(precitec::scantracker::components::wobbleFigureEditor::CommandManager* commandManager READ commandManager WRITE setCommandManager NOTIFY commandManagerChanged);

    //Scale
    Q_PROPERTY(int figureScale READ figureScale WRITE setFigureScale NOTIFY figureScaleChanged)

    //Offset
    Q_PROPERTY(QPointF offset READ offset WRITE setOffset NOTIFY offsetChanged)

    //Wobblefiguremodel
    Q_PROPERTY(unsigned int microVectorFactor READ microVectorFactor WRITE setMicroVectorFactor NOTIFY microVectorFactorChanged)
    Q_PROPERTY(precitec::scanmaster::components::wobbleFigureEditor::powerModulationMode::PowerModulationMode powerModulationMode READ powerModulationMode WRITE setPowerModulationMode NOTIFY powerModulationModeChanged)

    Q_PROPERTY(unsigned int numberOfPoints READ numberOfPoints WRITE setNumberOfPoints NOTIFY numberOfPointsChanged)

public:
    explicit WobbleFigureEditor(QObject* parent = nullptr);
    ~WobbleFigureEditor() override;

    enum FigureChangedState
    {
        NoChanges,
        Changed,
        Saved
    };

    WobbleFigure* figure() const;
    void setFigure(WobbleFigure* wobbleFigure);
    FileModel* fileModel() const;
    void setFileModel(FileModel* model);
    FigureCreator* figureCreator() const;
    void setFigureCreator(FigureCreator* creator);

    LaserPointController* laserPointController() const
    {
        return m_laserPointController;
    }
    void setLaserPointController(LaserPointController* pointController);

    FileType fileType() const
    {
        return m_type;
    }

    CommandManager* commandManager() const
    {
        return m_commandManager;
    };
    void setCommandManager(CommandManager* commandManager)
    {
        m_commandManager = commandManager;
        emit commandManagerChanged();
    };

    int figureScale() const;
    void setFigureScale(int newScaleFactor);

    unsigned int microVectorFactor() const
    {
        return m_wobbelFigure.microVectorFactor;
    }
    void setMicroVectorFactor(unsigned int newFactor);

    PowerModulationMode powerModulationMode() const
    {
        return static_cast<PowerModulationMode>(m_wobbelFigure.powerModulationMode);
    }
    void setPowerModulationMode(PowerModulationMode newMode);

    unsigned int numberOfPoints() const
    {
        return m_numberOfPoints;
    }
    void setNumberOfPoints(unsigned int newPoints);

    std::vector<RTC6::seamFigure::command::Order> seamPoints() const
    {
        return m_seamFigure.figure;
    }
    void setSeamFigure(RTC6::seamFigure::SeamFigure figure);
    void createLaserPointsFromSeamFigure(bool isPowerAnalog);

    std::vector<Ramp> ramps() const
    {
        return m_seamFigure.ramps;
    }
    void setRamps(const std::vector<Ramp>& currentRamps);

    Q_INVOKABLE void updatePosition(QObject* node);
    Q_INVOKABLE void updateProperties(QObject* node);
    Q_INVOKABLE void figurePropertiesChanged();
    Q_INVOKABLE void resetFigurePropertiesChange();
    Q_INVOKABLE bool isFigurePropertieChanged();

    Q_INVOKABLE void saveFigure();
    Q_INVOKABLE void saveAsFigure(const QString& fileName, precitec::scantracker::components::wobbleFigureEditor::FileType type);
    Q_INVOKABLE QString exportFigure(const QString& outDir);

    Q_PROPERTY(bool canDeleteFigure READ canDeleteFigure NOTIFY canDeleteFigureChanged);
    bool canDeleteFigure();
    Q_INVOKABLE void deleteFigure();

    Q_INVOKABLE void newFigure();
    void resetFigure();

    void setSeam(const RTC6::seamFigure::SeamFigure& seam);

    void showSeamFigure();
    void showWobbleFigure();
    void showOverlayFigure();
    void showBasicFigure();
    Q_INVOKABLE void showFigure(int type);

    Q_INVOKABLE void showFromFigureCreator();
    void createFromFigureCreator(std::vector<QPointF>& points);

    RTC6::seamFigure::SeamFigure* getSeamFigure();
    RTC6::wobbleFigure::Figure wobbleFigure();
    RTC6::function::OverlayFunction overlay();

    std::vector<QPointF> getVelocityInformation();

    Q_INVOKABLE void addOffsetToFigure();
    QPointF offset() const;
    void setOffset(const QPointF newOffset);

    Q_INVOKABLE void deletePoint(int id);
    void deletePointCPP(int position);
    void deletePointQML(int id);
    void setNewIDs(int position);
    void connectPoints(int position);

    Q_INVOKABLE void setFigureProperties(const QString& name, int ID, const QString& description);

    Q_INVOKABLE void setFileType(precitec::scantracker::components::wobbleFigureEditor::FileType type);

    Q_INVOKABLE void setNewStartPoint(int id);

    Q_INVOKABLE void reverseOrder();

    Q_INVOKABLE void mirrorYPosition();

    Q_INVOKABLE QStringList getLaserPointTooltip(int id) const;

    Q_INVOKABLE QPointF getLastPosition() const;

Q_SIGNALS:
    void fileChanged();
    void figureCleared();
    void canDeleteFigureChanged();

    void figureChanged();
    void fileModelChanged();
    void figureCreatorChanged();
    void laserPointControllerChanged();
    void scannerChanged();

    void fileTypeChanged();
    void commandManagerChanged();

    void figureScaleChanged();

    void offsetChanged();
    void microVectorFactorChanged();
    void powerModulationModeChanged();

    void numberOfPointsChanged();
    void seamPointsChanged();
    void rampsChanged();
    // TODO: test this signal
    void nodePositionUpdated();
    void nodePropertiesUpdated();
    void dataChanged();
private:
    double calculateLengthBetweenTwoNodes(qan::Node* firstNode, qan::Node* secondNode);
    double calculateLengthBetweenOneNodeAndPosition(qan::Node* node, const QPointF& position);
    void changeFigureScale();
    void createDefaultTrajectoryBetweenPoints(LaserPoint* sourcePoint, LaserPoint* destinationPoint);
    void changeFigureProperties(const QString& name, int ID, const QString& description);
    inline int iDFromFileName(const QString& filename, FileType type);
    bool checkIfPowerIsAnalog();
    precitec::interface::ActionInterface_sp getUndoActionPosition(RTC6::seamFigure::SeamFigure& figure,
                                                                  std::pair<double, double> const& oldPosition,
                                                                  std::pair<double, double> const& newPosition,
                                                                  LaserPoint* laserPoint);
    precitec::interface::ActionInterface_sp getUndoActionPosition(RTC6::wobbleFigure::Figure& figure,
                                                                  std::pair<double, double> const& oldPosition,
                                                                  std::pair<double, double> const& newPosition,
                                                                  LaserPoint* laserPoint);
    precitec::interface::ActionInterface_sp getUndoActionPosition(RTC6::function::OverlayFunction& figure,
                                                                  std::pair<double, double> const& oldPosition,
                                                                  std::pair<double, double> const& newPosition,
                                                                  LaserPoint* laserPoint);
    void dbgCheckInvariants() const;

    FigureChangedState m_figureChangedState{FigureChangedState::NoChanges};
    WobbleFigure* m_figure = nullptr;
    QMetaObject::Connection m_figureDestroyedConnection;
    FileModel* m_fileModel = nullptr;
    QMetaObject::Connection m_fileModelDestroyedConnection;
    FigureCreator* m_figureCreator = nullptr;
    QMetaObject::Connection m_figureCreatorDestroyedConnection;
    LaserPointController* m_laserPointController = nullptr;
    QMetaObject::Connection m_laserPointControllerDestroyedConnection;

    RTC6::seamFigure::SeamFigure m_seamFigure;
    RTC6::wobbleFigure::Figure m_wobbelFigure;
    RTC6::function::OverlayFunction m_overlayFigure;

    FileType m_type{FileType::None};
    CommandManager* m_commandManager = nullptr;
    int m_oldFigureScale = 1;
    int m_figureScale = 1000;
    unsigned int m_numberOfPoints{0};

    QPointF m_offset{0, 0};

    friend WobbleFigureEditorTest;
    friend FigureAnalyzerTest;
};

}
}
}
}
