#include "WobbleFigureEditor.h"
#include "LaserPoint.h"
#include "LaserTrajectory.h"
#include <math.h>
#include <algorithm>
#include <iterator>

#include "figureEditorSettings.h"
#include "actionPointPositionSeamFigure.h"
#include "actionPointPositionWobbleFigure.h"
#include "actionPointPositionOverlay.h"
#include "actionPointPropertieSeamLaserPower.h"
#include "actionPointPropertieWobbleLaserPower.h"
#include "actionPointPropertieSeamRingPower.h"
#include "actionPointPropertieWobbleRingPower.h"
#include "actionPointPropertieSeamVelocity.h"
#include "actionPointPositionSeamFigure.h"
#include "actionPointRampSeamFigure.h"
#include "actionPointCreateSeamFigure.h"
#include "trajectoryColorsValidator.h"

#include "laserPointController.h"

using precitec::scanmaster::components::wobbleFigureEditor::FigureEditorSettings;
using precitec::scantracker::components::wobbleFigureEditor::ActionPointPositionSeamFigure;
using precitec::scantracker::components::wobbleFigureEditor::ActionPointPositionWobbleFigure;
using precitec::scantracker::components::wobbleFigureEditor::ActionPointPositionOverlay;
using precitec::scantracker::components::wobbleFigureEditor::ActionPointPropertieSeamLaserPower;
using precitec::scantracker::components::wobbleFigureEditor::ActionPointPropertieWobbleLaserPower;
using precitec::scantracker::components::wobbleFigureEditor::ActionPointPropertieSeamRingPower;
using precitec::scantracker::components::wobbleFigureEditor::ActionPointPropertieWobbleRingPower;
using precitec::scantracker::components::wobbleFigureEditor::ActionPointPropertieSeamVelocity;
using precitec::scantracker::components::wobbleFigureEditor::ActionPointRampSeamFigure;
using precitec::scantracker::components::wobbleFigureEditor::ActionPointCreateSeamFigure;
using precitec::scanmaster::components::wobbleFigureEditor::LaserPointController;
using precitec::scantracker::components::wobbleFigureEditor::TrajectoryColorsValidator;

namespace precitec
{
namespace scantracker
{
namespace components
{
namespace wobbleFigureEditor
{

WobbleFigureEditor::WobbleFigureEditor(QObject* parent)
    : QObject(parent)
{
    connect(this, &WobbleFigureEditor::figureCleared, this, &WobbleFigureEditor::microVectorFactorChanged);
}

WobbleFigureEditor::~WobbleFigureEditor() = default;

WobbleFigure* WobbleFigureEditor::figure() const
{
    return m_figure;
}

void WobbleFigureEditor::setFigure(WobbleFigure* wobbleFigure)
{
    if (m_figure == wobbleFigure)
    {
        return;
    }
    disconnect(m_figureDestroyedConnection);
    m_figure = wobbleFigure;
    if (m_figure)
    {
        m_figureDestroyedConnection = connect(m_figure, &QObject::destroyed, this, std::bind(&WobbleFigureEditor::setFigure, this, nullptr));
    }
    else
    {
        m_figureDestroyedConnection = {};
    }
    emit figureChanged();
}

FileModel* WobbleFigureEditor::fileModel() const
{
    return m_fileModel;
}

void WobbleFigureEditor::setFileModel(FileModel* model)
{
    if (m_fileModel == model)
    {
        return;
    }
    disconnect(m_fileModelDestroyedConnection);
    m_fileModel = model;
    if (m_fileModel)
    {
        m_fileModelDestroyedConnection = connect(m_fileModel, &QObject::destroyed, this, std::bind(&WobbleFigureEditor::setFileModel, this, nullptr));
        connect(m_fileModel, &FileModel::figureLoaded, this, &WobbleFigureEditor::setFileType);
    }
    else
    {
        m_fileModelDestroyedConnection = {};
    }
    emit fileModelChanged();
}

FigureCreator* WobbleFigureEditor::figureCreator() const
{
    return m_figureCreator;
}

void WobbleFigureEditor::setFigureCreator(FigureCreator* creator)
{
    if (m_figureCreator == creator)
    {
        return;
    }
    disconnect(m_figureCreatorDestroyedConnection);
    m_figureCreator = creator;
    if (m_figureCreator)
    {
        m_figureCreatorDestroyedConnection = connect(m_figureCreator, &QObject::destroyed, this, std::bind(&WobbleFigureEditor::setFigureCreator, this, nullptr));
    }
    else
    {
        m_figureCreatorDestroyedConnection = {};
    }
    emit figureCreatorChanged();
}

void WobbleFigureEditor::setLaserPointController(LaserPointController* pointController)
{
    if (m_laserPointController == pointController)
    {
        return;
    }
    disconnect(m_laserPointControllerDestroyedConnection);
    m_laserPointController = pointController;
    if (m_laserPointController)
    {
        m_laserPointControllerDestroyedConnection = connect(m_laserPointController, &QObject::destroyed, this, std::bind(&WobbleFigureEditor::setLaserPointController, this, nullptr));
    }
    else
    {
        m_laserPointControllerDestroyedConnection = {};
    }
    emit laserPointControllerChanged();
}

int WobbleFigureEditor::figureScale() const
{
    return m_figureScale;
}

void WobbleFigureEditor::setFigureScale(int newScaleFactor)
{
    if (m_figureScale == newScaleFactor)
    {
        return;
    }

    m_oldFigureScale = m_figureScale;
    m_figureScale = newScaleFactor;
    emit figureScaleChanged();
}

void WobbleFigureEditor::setMicroVectorFactor(unsigned int newFactor)
{
    if (m_wobbelFigure.microVectorFactor == newFactor)
    {
        return;
    }
    figurePropertiesChanged();
    m_wobbelFigure.microVectorFactor = newFactor;
    emit microVectorFactorChanged();
}

void WobbleFigureEditor::setPowerModulationMode(PowerModulationMode newMode)
{
    auto mode = static_cast<unsigned int>(newMode);
    if (m_wobbelFigure.powerModulationMode == mode)
    {
        return;
    }
    figurePropertiesChanged();
    m_wobbelFigure.powerModulationMode = mode;
    emit powerModulationModeChanged();
}

void WobbleFigureEditor::setNumberOfPoints(unsigned int newPoints)
{
    if (m_numberOfPoints == newPoints)
    {
        return;
    }

    m_numberOfPoints = newPoints;
    emit numberOfPointsChanged();
}

void WobbleFigureEditor::setSeamFigure(RTC6::seamFigure::SeamFigure figure)
{
    m_seamFigure = figure;
    bool isAnalogPower = checkIfPowerIsAnalog();
    m_figure->resetLaserPoints();
    createLaserPointsFromSeamFigure(isAnalogPower);
    m_figure->setAnalogPower(isAnalogPower);
    setNumberOfPoints(m_seamFigure.figure.size());
    emit seamPointsChanged();
    emit rampsChanged();
    emit nodePositionUpdated();
}

void WobbleFigureEditor::setRamps(const std::vector<Ramp>& currentRamps)
{
    if (m_type != FileType::Seam || m_seamFigure.ramps == currentRamps)
    {
        return;
    }
    precitec::interface::ActionInterface_sp action =
        std::make_shared<ActionPointRampSeamFigure>(
            *this,
            m_seamFigure,
            m_seamFigure.ramps,
            currentRamps);
    m_commandManager->addAction(action);
    figurePropertiesChanged();
    m_seamFigure.ramps = currentRamps;
    TrajectoryColorsValidator validator;
    validator.setSeamFigure(&m_seamFigure);
    validator.setFigure(m_figure);
    validator.calculateColors();
    emit rampsChanged();
}

void WobbleFigureEditor::updatePosition(QObject* node)
{
    if (auto laserPoint = dynamic_cast<LaserPoint*>(node))
    {
        LaserPoint savePoint = LaserPoint(laserPoint);
        std::pair<double, double> const newPosition = {laserPoint->center().x() / m_figureScale, -(laserPoint->center().y() / m_figureScale)};

        switch (laserPoint->type())
        {
        case FileType::Seam:
        {
            std::pair<double, double> oldPosition = m_seamFigure.figure.at(laserPoint->ID()).endPosition;
            m_seamFigure.figure.at(laserPoint->ID()).endPosition = newPosition;

            m_commandManager->addAction(getUndoActionPosition(
                m_seamFigure,
                oldPosition,
                newPosition,
                laserPoint));
            figurePropertiesChanged();

            emit seamPointsChanged();
            emit nodePositionUpdated();
            break;
        }
        case FileType::Wobble:
        {
            std::pair<double, double> oldPosition = m_wobbelFigure.figure.at(laserPoint->ID()).endPosition;
            m_wobbelFigure.figure.at(laserPoint->ID()).endPosition = newPosition;

            m_commandManager->addAction(getUndoActionPosition(
                m_wobbelFigure,
                oldPosition,
                newPosition,
                laserPoint));
            figurePropertiesChanged();

            emit nodePositionUpdated();
            break;
        }
        case FileType::Overlay:
        {
            std::pair<double, double> oldPosition = m_overlayFigure.functionValues.at(laserPoint->ID());
            m_overlayFigure.functionValues.at(laserPoint->ID()) = newPosition;

            m_commandManager->addAction(getUndoActionPosition(
                m_overlayFigure,
                oldPosition,
                newPosition,
                laserPoint));
            figurePropertiesChanged();

            emit nodePositionUpdated();
            break;
        }
        default:
            return;
        }
    }
    TrajectoryColorsValidator validator;
    validator.setSeamFigure(&m_seamFigure);
    validator.setFigure(m_figure);
    validator.calculateColors();
    dbgCheckInvariants();
}

void WobbleFigureEditor::updateProperties(QObject* node)
{
    if (auto laserPoint = dynamic_cast<LaserPoint*>(node))
    {
        // Todo: only change properties if there changed
        switch (laserPoint->type())
        {
        case FileType::Seam:
        {
            auto order = &m_seamFigure.figure.at(laserPoint->ID());
            std::pair<double, double> oldPosition = order->endPosition;
            order->endPosition = std::make_pair(laserPoint->center().x() / m_figureScale, -1 * (laserPoint->center().y() / m_figureScale));
            std::pair<double, double> newPosition = order->endPosition;
            if (oldPosition != newPosition)
            {
                m_commandManager->addAction(getUndoActionPosition(
                    m_seamFigure,
                    oldPosition,
                    newPosition,
                    laserPoint));
                figurePropertiesChanged();
            }
            double oldRingPower = order->ringPower;
            double oldLaserPower = order->power;
            if (m_figure->analogPower())
            {
                if (qFuzzyCompare(laserPoint->ringPower(), -1.0))
                {
                    order->ringPower = -1.0;
                }
                else
                {
                    order->ringPower = laserPoint->ringPower() / 100.0;
                }
                if (oldRingPower != order->ringPower)
                {
                    precitec::interface::ActionInterface_sp action =
                        std::make_shared<ActionPointPropertieSeamRingPower>(
                            *this,
                            m_seamFigure,
                            laserPoint->ID(),
                            oldRingPower,
                            order->ringPower);
                    m_commandManager->addAction(action);
                    figurePropertiesChanged();
                }
                order->power = qFuzzyCompare(laserPoint->laserPower(), -1.0) ? -1.0 : 0.01 * laserPoint->laserPower();
            }
            else
            {
                order->power = laserPoint->laserPower();
            }
            double oldVelocity = order->velocity;
            order->velocity = laserPoint->velocity();
            if (oldVelocity != order->velocity)
            {
                precitec::interface::ActionInterface_sp action =
                    std::make_shared<ActionPointPropertieSeamVelocity>(
                        *this,
                        m_seamFigure,
                        laserPoint->ID(),
                        oldVelocity,
                        order->velocity);
                m_commandManager->addAction(action);
                figurePropertiesChanged();
            }

            if (oldLaserPower != order->power)
            {
                precitec::interface::ActionInterface_sp action =
                    std::make_shared<ActionPointPropertieSeamLaserPower>(
                        *this,
                        m_seamFigure,
                        laserPoint->ID(),
                        oldLaserPower,
                        order->power);
                m_commandManager->addAction(action);
                figurePropertiesChanged();
            }

            //Update qml trajectory (length and time)
            //If first or last point change just one trajectory
            //Else update trajectory before and after!
            if (laserPoint->ID() == 0)
            {
                auto trajectory = dynamic_cast<LaserTrajectory*>(m_figure->get_edges().at(laserPoint->ID()));
                auto end = dynamic_cast<LaserPoint*>(m_figure->get_nodes().at(laserPoint->ID() + 1));
                trajectory->setTrajectoryLength(calculateLengthBetweenTwoNodes(laserPoint, end));
                trajectory->setTime(trajectory->trajectoryLength() / trajectory->speed());
                //emit nodePositionUpdated();
                break;
                ;
            }
            if (std::size_t(laserPoint->ID() + 1) == m_figure->get_node_count())
            {
                auto trajectory = dynamic_cast<LaserTrajectory*>(m_figure->get_edges().at(laserPoint->ID() - 1));
                auto start = dynamic_cast<LaserPoint*>(m_figure->get_nodes().at(laserPoint->ID() - 1));
                trajectory->setTrajectoryLength(calculateLengthBetweenTwoNodes(start, laserPoint));
                trajectory->setTime(trajectory->trajectoryLength() / trajectory->speed());
                //emit nodePositionUpdated();
                break;
            }
            //Update first trajectory
            auto trajectory = dynamic_cast<LaserTrajectory*>(m_figure->get_edges().at(laserPoint->ID() - 1));
            auto seamPoint = dynamic_cast<LaserPoint*>(m_figure->get_nodes().at(laserPoint->ID() - 1));
            trajectory->setTrajectoryLength(calculateLengthBetweenTwoNodes(seamPoint, laserPoint));
            trajectory->setTime(trajectory->trajectoryLength() / trajectory->speed());
            //Update second trajectory
            trajectory = dynamic_cast<LaserTrajectory*>(m_figure->get_edges().at(laserPoint->ID()));
            seamPoint = dynamic_cast<LaserPoint*>(m_figure->get_nodes().at(laserPoint->ID() + 1));
            trajectory->setTrajectoryLength(calculateLengthBetweenTwoNodes(laserPoint, seamPoint));
            trajectory->setTime(trajectory->trajectoryLength() / trajectory->speed());
            emit seamPointsChanged();
            break;
        }
        case FileType::Wobble:
        {
            auto& wobblePoint = m_wobbelFigure.figure.at(laserPoint->ID());
            std::pair<double, double> newPosition = std::make_pair(laserPoint->center().x() / m_figureScale, -1 * (laserPoint->center().y() / m_figureScale));
            if (wobblePoint.endPosition != newPosition)
            {
                m_commandManager->addAction(getUndoActionPosition(
                    m_wobbelFigure,
                    wobblePoint.endPosition,
                    newPosition,
                    laserPoint));
                wobblePoint.endPosition = newPosition;
                figurePropertiesChanged();
            }
            double newLaserPower = 0.01 * laserPoint->laserPower();
            if (wobblePoint.power != newLaserPower)
            {
                precitec::interface::ActionInterface_sp action =
                    std::make_shared<ActionPointPropertieWobbleLaserPower>(
                        *this,
                        m_wobbelFigure,
                        laserPoint->ID(),
                        wobblePoint.power,
                        newLaserPower);
                m_commandManager->addAction(action);
                figurePropertiesChanged();
                wobblePoint.power = newLaserPower;
            }
            double oldRingPower = wobblePoint.ringPower;
            wobblePoint.ringPower = 0.01 * laserPoint->ringPower();
            if (oldRingPower != wobblePoint.ringPower)
            {
                precitec::interface::ActionInterface_sp action =
                    std::make_shared<ActionPointPropertieWobbleRingPower>(
                        *this,
                        m_wobbelFigure,
                        laserPoint->ID(),
                        oldRingPower,
                        wobblePoint.ringPower);
                m_commandManager->addAction(action);
                figurePropertiesChanged();
            }
            break;
        }
        case FileType::Overlay:
        {
            auto& overlayPoint = m_overlayFigure.functionValues.at(laserPoint->ID());
            std::pair<double, double> oldPosition = overlayPoint;
            overlayPoint.first = laserPoint->center().x() / m_figureScale;
            overlayPoint.second = -1 * laserPoint->center().y() / m_figureScale;
            if (overlayPoint != oldPosition)
            {
                m_commandManager->addAction(getUndoActionPosition(
                    m_overlayFigure,
                    oldPosition,
                    overlayPoint,
                    laserPoint));
                figurePropertiesChanged();
            }
            break;
        }
        default:
            return;
        }
        emit nodePositionUpdated();
        emit nodePropertiesUpdated();
    }
    TrajectoryColorsValidator validator;
    validator.setSeamFigure(&m_seamFigure);
    validator.setFigure(m_figure);
    validator.calculateColors();
}

QString WobbleFigureEditor::exportFigure(const QString& outDir)
{
    if (!m_fileModel)
    {
        return {};
    }

    QDir{}.mkpath(outDir);

    QString const fullPath = outDir + "/" + m_fileModel->filename();

    bool ok = false;
    switch (m_type)
    {
    case FileType::Seam:
        ok = FileModel::saveFigureToFile(m_seamFigure, fullPath);
        break;
    case FileType::Wobble:
        ok = FileModel::saveFigureToFile(m_wobbelFigure, fullPath);
        break;
    case FileType::Overlay:
        ok = FileModel::saveFigureToFile(m_overlayFigure, fullPath);
        break;
    default:
        Q_UNREACHABLE();
        return {};
    }

    if (ok)
    {
        return fullPath;
    }

    return {};
    dbgCheckInvariants();
}

void WobbleFigureEditor::figurePropertiesChanged()
{
    m_figureChangedState = FigureChangedState::Changed;
}

void WobbleFigureEditor::resetFigurePropertiesChange()
{
    m_figureChangedState = FigureChangedState::NoChanges;
}

bool WobbleFigureEditor::isFigurePropertieChanged()
{
    return (m_figureChangedState == FigureChangedState::Changed);
}

void WobbleFigureEditor::saveFigure()
{
    dbgCheckInvariants();
    if (!m_fileModel)
    {
        return;
    }

    switch (m_type)
    {
    case FileType::Seam:
        m_fileModel->saveFigure(m_seamFigure);
        m_figureChangedState = FigureChangedState::Saved;
        m_commandManager->clearStack();
        break;
    case FileType::Wobble:
        m_fileModel->saveFigure(m_wobbelFigure);
        m_figureChangedState = FigureChangedState::Saved;
        m_commandManager->clearStack();
        break;
    case FileType::Overlay:
        m_fileModel->saveFigure(m_overlayFigure);
        m_figureChangedState = FigureChangedState::Saved;
        m_commandManager->clearStack();
        break;
    default:
        break;
    }

    emit canDeleteFigureChanged();
}

void WobbleFigureEditor::saveAsFigure(const QString& fileName, FileType type)
{
    dbgCheckInvariants();
    if (!m_fileModel)
    {
        return;
    }

    auto id = iDFromFileName(fileName, type);

    switch (type)
    {
    case FileType::Seam:
        m_seamFigure.ID = std::to_string(id);
        m_fileModel->saveFigure(m_seamFigure, fileName);
        m_figureChangedState = FigureChangedState::Saved;
        m_commandManager->clearStack();
        break;
    case FileType::Wobble:
        m_wobbelFigure.ID = std::to_string(id);
        m_fileModel->saveFigure(m_wobbelFigure, fileName);
        m_figureChangedState = FigureChangedState::Saved;
        m_commandManager->clearStack();
        break;
    case FileType::Overlay:
        m_overlayFigure.ID = std::to_string(id);
        m_fileModel->saveFigure(m_overlayFigure, fileName);
        m_figureChangedState = FigureChangedState::Saved;
        m_commandManager->clearStack();
        break;
    case FileType::Basic:
        m_wobbelFigure.ID = std::to_string(id);
        m_fileModel->saveFigure(m_wobbelFigure, fileName);
        m_figureChangedState = FigureChangedState::Saved;
        m_commandManager->clearStack();
    default:
        return;
    }

    emit canDeleteFigureChanged();
}

bool WobbleFigureEditor::canDeleteFigure()
{
    if (!m_fileModel)
        return false;

    switch (m_fileModel->fileType())
    {
    case FileType::Seam:
    case FileType::Wobble:
    case FileType::Overlay:
        return m_fileModel->fileExists();
    default:
        return false;
    }
}

void WobbleFigureEditor::deleteFigure()
{
    if (!m_fileModel)
    {
        return;
    }

    m_fileModel->deleteFigure();
    emit canDeleteFigureChanged();
}

void WobbleFigureEditor::newFigure()
{
    resetFigure();
}

void WobbleFigureEditor::resetFigure()
{
    if (!m_figure)
    {
        return;
    }
    //QML
    m_figure->clear();
    m_figure->setID(0);
    m_figure->setName({tr("Figure name")});
    m_figure->setDescription({});
    m_figure->setAnalogPower(true);
    //Cpp
    m_seamFigure = {};
    m_wobbelFigure = {};
    m_overlayFigure = {};
    m_figure->resetLaserPoints();
    m_type = FileType::None;
    setNumberOfPoints(0);
    commandManager()->clearStack();
    emit figureCleared();
    emit fileTypeChanged();
    emit canDeleteFigureChanged();
    dbgCheckInvariants();
}

void WobbleFigureEditor::showSeamFigure()
{
    if (!m_figure || !m_fileModel)
    {
        return;
    }
    m_seamFigure = m_fileModel->seamFigure();

    bool isPowerAnalog = checkIfPowerIsAnalog();

    createLaserPointsFromSeamFigure(isPowerAnalog);

    changeFigureProperties(QString::fromStdString(m_seamFigure.name), iDFromFileName(m_fileModel->filename(), FileType::Seam), QString::fromStdString(m_seamFigure.description));

    m_figure->setAnalogPower(isPowerAnalog);
    setNumberOfPoints(m_seamFigure.figure.size());

    TrajectoryColorsValidator validator;
    validator.setSeamFigure(&m_seamFigure);
    validator.setFigure(m_figure);
    validator.calculateColors();

    emit seamPointsChanged();
    emit rampsChanged();
    dbgCheckInvariants();
}

void WobbleFigureEditor::showWobbleFigure()
{
    if (!m_figure)
    {
        return;
    }
    m_wobbelFigure = m_fileModel->wobbleFigure();

    LaserPoint* lastPoint = nullptr;
    LaserPoint* currentPoint = nullptr;

    auto counter = m_figure->get_node_count();
    m_figure->resetLaserPoints();

    for (const auto& point : m_wobbelFigure.figure)
    {
        auto node = dynamic_cast<LaserPoint*>(m_figure->insertLaserPoint());
        if (node)
        {
            //Insert points
            auto laserPoint = dynamic_cast<LaserPoint*>(node);
            laserPoint->setID(counter);
            laserPoint->getItem()->setMinimumSize({15, 15});
            laserPoint->getItem()->setResizable(false);
            laserPoint->getItem()->setRect({(point.endPosition.first * m_figureScale) - (15 * 0.5), -1 * ((point.endPosition.second * m_figureScale) + (15 * 0.5)), 15, 15});
            laserPoint->initConnectionToNodeItem();
            if (!qFuzzyCompare(point.power, 0.0))
            {
                laserPoint->setEditable(true);
            }
            else
            {
                laserPoint->setEditable(false);
            }
            laserPoint->setLaserPower(point.power * 100);
            laserPoint->setRingPower(point.ringPower * 100);
            laserPoint->setType(FileType::Wobble);
            counter = m_figure->get_node_count();
        }
        if (!lastPoint)
        {
            lastPoint = node;
        }
        else
        {
            if (currentPoint)
            {
                lastPoint = currentPoint;
            }
            currentPoint = node;
        }
        if (lastPoint && currentPoint)
        {
            auto connection = m_figure->insertConnection(lastPoint, currentPoint);
            if (connection)
            {
                auto trajectory = dynamic_cast<LaserTrajectory*>(connection);
                trajectory->setID(counter - 1); //ID is the ID of the start point
                trajectory->setEditable(true);  //FIXME, check if speed from lastPoint to currentPoint changed
                trajectory->setSpeed(30.0);
                trajectory->setTrajectoryLength(calculateLengthBetweenTwoNodes(lastPoint, currentPoint));
                trajectory->setTime(trajectory->trajectoryLength() / trajectory->speed());
                trajectory->setGroup(0);
                trajectory->setType(0);
                lastPoint->modifyAttachedEdge();
            }
        }
    }

    changeFigureProperties(QString::fromStdString(m_wobbelFigure.name), iDFromFileName(m_fileModel->filename(), FileType::Wobble), QString::fromStdString(m_wobbelFigure.description));
    setNumberOfPoints(m_wobbelFigure.figure.size());
    emit microVectorFactorChanged();
    emit powerModulationModeChanged();
}

void WobbleFigureEditor::showOverlayFigure()
{
    if (!m_figure)
    {
        return;
    }
    m_overlayFigure = m_fileModel->overlayFigure();

    qan::Node* lastPoint = nullptr;
    qan::Node* currentPoint = nullptr;

    auto counter = m_figure->get_node_count();

    m_figure->resetLaserPoints();

    for (const auto& point : m_overlayFigure.functionValues)
    {
        auto overlayPoint = m_figure->insertLaserPoint();
        if (overlayPoint)
        {
            auto laserPoint = dynamic_cast<LaserPoint*>(overlayPoint);
            laserPoint->setID(counter);
            laserPoint->setType(FileType::Overlay);
            laserPoint->setEditable(true);
            laserPoint->getItem()->setMinimumSize({15, 15});
            laserPoint->getItem()->setRect({(point.first * m_figureScale) - (15 * 0.5), -1 * ((point.second * m_figureScale) + (15 * 0.5)), 15, 15});
            laserPoint->initConnectionToNodeItem();
            counter = m_figure->get_node_count();
        }
        if (!lastPoint)
        {
            lastPoint = overlayPoint;
        }
        else
        {
            if (currentPoint)
            {
                lastPoint = currentPoint;
            }
            currentPoint = overlayPoint;
        }
        if (lastPoint && currentPoint)
        {
            m_figure->insertConnection(lastPoint, currentPoint);
        }
    }

    changeFigureProperties(QString::fromStdString(m_overlayFigure.name), m_overlayFigure.ID.empty() ? 0 : std::stoi(m_overlayFigure.ID), QString::fromStdString(m_overlayFigure.description));
    setNumberOfPoints(m_overlayFigure.functionValues.size());
}

void WobbleFigureEditor::showBasicFigure()
{
    if (!m_laserPointController || !m_fileModel)
    {
        return;
    }

    m_wobbelFigure = m_fileModel->wobbleFigure();
    m_laserPointController->drawBasicFigure(m_wobbelFigure);
    setNumberOfPoints(m_wobbelFigure.figure.size());
}

void WobbleFigureEditor::showFigure(int type)
{
    dbgCheckInvariants();
    if (!m_figure)
    {
        return;
    }
    //Clear anything
    m_figure->clear();
    m_seamFigure = {};
    m_wobbelFigure = {};
    m_overlayFigure = {};
    m_type = FileType::None;

    switch (type)
    {
    case static_cast<int>(FileType::Seam):
        m_type = FileType::Seam;
        showSeamFigure();
        break;
    case static_cast<int>(FileType::Wobble):
        m_type = FileType::Wobble;
        showWobbleFigure();
        break;
    case static_cast<int>(FileType::Overlay):
        m_type = FileType::Overlay;
        showOverlayFigure();
        break;
    case static_cast<int>(FileType::Basic):
        m_type = FileType::Basic;
        showBasicFigure();
        break;
    default:
        return;
    }

    emit fileChanged();
    emit canDeleteFigureChanged();
    emit fileTypeChanged();
    dbgCheckInvariants();
}

void WobbleFigureEditor::showFromFigureCreator()
{
    dbgCheckInvariants();
    if (!m_figureCreator)
    {
        return;
    }

    RTC6::seamFigure::SeamFigure oldFigure;

    if (m_figureCreator->fileType() != fileType())
    {
        switch (m_type)
        {
        case FileType::Wobble:
            break;
        case FileType::Overlay:
            break;
        case FileType::None:
            break;
        case FileType::Seam:
            oldFigure = m_seamFigure;
            break;
        default:
            return;
        }

        resetFigure();
        m_type = figureCreator()->fileType();
        emit fileTypeChanged();
        dbgCheckInvariants();
    }

    auto& points = *m_figureCreator->getFigure();

    createFromFigureCreator(points);

    precitec::interface::ActionInterface_sp action =
        std::make_shared<ActionPointCreateSeamFigure>(
            *this,
            *m_fileModel,
            oldFigure,
            m_seamFigure);
    m_commandManager->addAction(action);

    emit nodePositionUpdated();
    qDebug() << "new Figure";
}

RTC6::seamFigure::SeamFigure* WobbleFigureEditor::getSeamFigure()
{
    return &m_seamFigure;
}

RTC6::wobbleFigure::Figure WobbleFigureEditor::wobbleFigure()
{
    return m_wobbelFigure;
}

RTC6::function::OverlayFunction WobbleFigureEditor::overlay()
{
    return m_overlayFigure;
}

std::vector<QPointF> WobbleFigureEditor::getVelocityInformation()
{
    std::vector<QPointF> information;
    if (!m_seamFigure.figure.empty())
    {
        QPointF point;
        int i = 1;
        for (const auto& element : m_seamFigure.figure)
        {
            point.setX(i);
            point.setY(element.velocity);
            information.push_back(point);
            i++;
        }
        return information;
    }
    return {};
}

void WobbleFigureEditor::addOffsetToFigure()
{
    dbgCheckInvariants();
    if (m_seamFigure.figure.size() == 0 || (m_offset.x() == 0 && m_offset.y() == 0) || m_figure->get_node_count() == 0)
    {
        return;
    }

    for (auto point : m_figure->get_nodes())
    {
        if (auto laserPoint = dynamic_cast<LaserPoint*>(point))
        {
            laserPoint->setCenter(laserPoint->center() + m_offset * m_figureScale);
        }
    }

    for (auto& point : m_seamFigure.figure)
    {
        point.endPosition.first = point.endPosition.first + m_offset.x();
        point.endPosition.second = point.endPosition.second + m_offset.y();
    }

    dbgCheckInvariants();
}

QPointF WobbleFigureEditor::offset() const
{
    return m_offset;
}

void WobbleFigureEditor::setOffset(const QPointF newOffset)
{
    if (m_offset == newOffset)
    {
        return;
    }

    m_offset = newOffset;
    emit offsetChanged();
}

void WobbleFigureEditor::deletePoint(int id)
{
    deletePointCPP(id);
    deletePointQML(id);
    connectPoints(id);
    setNewIDs(id);
}

void WobbleFigureEditor::deletePointCPP(int position)
{
    if (position >= 0 && std::size_t(position) < m_seamFigure.figure.size())
    {
        m_seamFigure.figure.erase(m_seamFigure.figure.begin() + position);
    }
    setNumberOfPoints(m_seamFigure.figure.size());
    emit seamPointsChanged();
}

void WobbleFigureEditor::deletePointQML(int id)
{
    auto point = std::find_if(m_figure->get_nodes().begin(), m_figure->get_nodes().end(), [id](const auto& currentPoint)
                              { return dynamic_cast<LaserPoint*>(currentPoint)->ID() == id; });
    if (point != m_figure->get_nodes().end())
    {
        m_figure->removeNode(*point);
    }
}

void WobbleFigureEditor::setNewIDs(int position)
{
    if (position >= 0 && position < m_figure->get_nodes().size())
    {
        unsigned int i = 0;
        if (position != 0 && dynamic_cast<LaserPoint*>(m_figure->get_nodes().at(position - 1)))
        {
            i = dynamic_cast<LaserPoint*>(m_figure->get_nodes().at(position - 1))->ID() + 1;
        }
        for (auto start = m_figure->get_nodes().begin() + position; start < m_figure->get_nodes().end(); start++)
        {
            if (auto laserPoint = dynamic_cast<LaserPoint*>(*start))
            {
                laserPoint->setID(i);
                const auto& trajectories = laserPoint->get_out_edges();
                if (trajectories.size() != 0)
                {
                    if (auto laserTrajectory = dynamic_cast<LaserTrajectory*>(trajectories.at(0)))
                    {
                        laserTrajectory->setID(i);
                    }
                }
            }
            i++;
        }
    }
}

void WobbleFigureEditor::connectPoints(int position)
{
    auto start = dynamic_cast<LaserPoint*>(m_figure->get_nodes().at(position - 1));
    auto end = dynamic_cast<LaserPoint*>(m_figure->get_nodes().at(position));
    createDefaultTrajectoryBetweenPoints(start, end);
}

void WobbleFigureEditor::setFigureProperties(const QString& name, int ID, const QString& description)
{
    if (!m_figure)
    {
        return;
    }
    changeFigureProperties(name, ID, description);

    switch (m_type)
    {
    case FileType::Seam:
        m_seamFigure.name = name.toStdString();
        m_seamFigure.ID = std::to_string(ID);
        m_seamFigure.description = description.toStdString();
        return;
    case FileType::Wobble:
        m_wobbelFigure.name = name.toStdString();
        m_wobbelFigure.ID = std::to_string(ID);
        m_wobbelFigure.description = description.toStdString();
        return;
    case FileType::Overlay:
        m_overlayFigure.name = name.toStdString();
        m_overlayFigure.ID = std::to_string(ID);
        m_overlayFigure.description = description.toStdString();
        return;
    default:
        return;
    }
}

void WobbleFigureEditor::setFileType(FileType type)
{
    dbgCheckInvariants();
    if (type == m_type)
    {
        return;
    }

    resetFigure();

    m_type = type;

    switch (type)
    {
    case FileType::Seam:
        showSeamFigure();
        break;
    case FileType::Wobble:
        showWobbleFigure();
        break;
    case FileType::Overlay:
        showOverlayFigure();
        break;
    default:
        m_type = FileType::None;
        Q_UNREACHABLE();
        return;
    }

    emit fileTypeChanged();
    dbgCheckInvariants();
}

void WobbleFigureEditor::setNewStartPoint(int id)
{
    dbgCheckInvariants();
    if (!m_figure || id == 0 || static_cast<std::size_t>(id) >= m_seamFigure.figure.size() || m_seamFigure.figure.empty())
    {
        return;
    }

    //First check if figure is closed or open. Just check if start and end point is at the same position. Open figure loose the part from the old beginning to the new. Closed figure is like rotate the whole figure.
    auto firstNode = m_figure->get_nodes().at(0);
    auto secondNode = m_figure->get_nodes().at(m_figure->get_node_count() - 1);

    const auto& length = calculateLengthBetweenTwoNodes(firstNode, secondNode);

    if (length < 0.05) //Closed figure
    {
        m_seamFigure.figure.insert(m_seamFigure.figure.end(), std::make_move_iterator(std::next(m_seamFigure.figure.begin(), 1)), std::make_move_iterator(std::next(m_seamFigure.figure.begin(), id)));
        m_seamFigure.figure.erase(m_seamFigure.figure.begin(), std::next(m_seamFigure.figure.begin(), id));
        m_seamFigure.figure.emplace_back(m_seamFigure.figure.front());

        m_figure->clear();
        createLaserPointsFromSeamFigure(m_figure->analogPower());
    }
    else //Open figure
    {
        for (int i = 0; i < id; i++)
        {
            deletePointQML(i);
        }
        m_seamFigure.figure.erase(m_seamFigure.figure.begin(), m_seamFigure.figure.begin() + id);
        setNewIDs(0);
    }
    setNumberOfPoints(m_seamFigure.figure.size());
    emit dataChanged();
    dbgCheckInvariants();
}

void WobbleFigureEditor::reverseOrder()
{
    dbgCheckInvariants();
    if (!m_seamFigure.figure.size())
    {
        return;
    }

    std::reverse(m_seamFigure.figure.begin(), m_seamFigure.figure.end());
    dbgCheckInvariants();
}

void WobbleFigureEditor::mirrorYPosition()
{
    dbgCheckInvariants();
    if (!m_figure)
    {
        return;
    }

    const auto& laserPoints = m_figure->get_nodes();

    for (auto& point : laserPoints)
    {
        auto laserpoint = dynamic_cast<LaserPoint*>(point);
        laserpoint->setCenter({laserpoint->center().x(), -laserpoint->center().y()});
    }

    switch (m_type)
    {
    case FileType::Seam:
        std::for_each(m_seamFigure.figure.begin(), m_seamFigure.figure.end(), [](auto& point)
                      { point.endPosition.second = -1 * point.endPosition.second; });
        break;
    case FileType::Wobble:
        std::for_each(m_wobbelFigure.figure.begin(), m_wobbelFigure.figure.end(), [](auto& point)
                      { point.endPosition.second = -1 * point.endPosition.second; });
        break;
    case FileType::Overlay:
        std::for_each(m_overlayFigure.functionValues.begin(), m_overlayFigure.functionValues.end(), [](auto& point)
                      { point.second = -1 * point.second; });
        break;
    default:
        return;
    }
    dbgCheckInvariants();
}

void WobbleFigureEditor::setSeam(const RTC6::seamFigure::SeamFigure& seam)
{
    m_seamFigure = seam;
    setNumberOfPoints(m_seamFigure.figure.size());
    emit seamPointsChanged();
}

double WobbleFigureEditor::calculateLengthBetweenTwoNodes(qan::Node* firstNode, qan::Node* secondNode)
{
    if (!firstNode || !secondNode)
    {
        return 0.0;
    }

    QPointF firstPoint{firstNode->getItem()->x(), firstNode->getItem()->y()};
    QPointF secondPoint{secondNode->getItem()->x(), secondNode->getItem()->y()};

    const QVector2D firstSecondVector{secondPoint - firstPoint};

    return firstSecondVector.length() / m_figureScale;
}

double WobbleFigureEditor::calculateLengthBetweenOneNodeAndPosition(qan::Node* node, const QPointF& position)
{
    if (!node)
    {
        return {};
    }

    QPointF firstPoint{node->getItem()->x() + (node->getItem()->width() * 0.5), (-node->getItem()->y()) + (node->getItem()->height() * 0.5)};

    QVector2D nodePositionVector{position - firstPoint};

    return nodePositionVector.length() / m_figureScale;
}

void WobbleFigureEditor::createDefaultTrajectoryBetweenPoints(LaserPoint* sourcePoint, LaserPoint* destinationPoint)
{
    if (!sourcePoint || !destinationPoint)
    {
        return;
    }

    if (sourcePoint == destinationPoint)
    {
        return;
    }

    if (!m_figure)
    {
        return;
    }

    auto connection = m_figure->insertConnection(sourcePoint, destinationPoint);
    if (connection)
    {
        auto trajectory = dynamic_cast<LaserTrajectory*>(connection);
        trajectory->setID(sourcePoint->ID());
        trajectory->setEditable(true);
        trajectory->setSpeed(-1.0); //default value take from Scanmaster
        trajectory->setTrajectoryLength(calculateLengthBetweenTwoNodes(sourcePoint, destinationPoint));
        if (trajectory->speed() != -1.0)
        {
            trajectory->setTime(trajectory->trajectoryLength() / trajectory->speed());
        }
        else
        {
            trajectory->setTime(0);
        }
        trajectory->setGroup(0);
        trajectory->setType(0);
        if (m_type == FileType::Seam)
        {
            sourcePoint->modifyAttachedEdge();
        }
    }
}

void WobbleFigureEditor::changeFigureProperties(const QString& name, int ID, const QString& description)
{
    m_figure->setName(name);
    m_figure->setID(ID);
    m_figure->setDescription(description);
}

int WobbleFigureEditor::iDFromFileName(const QString& filename, FileType type)
{
    auto filenameForID = filename;

    switch (type)
    {
    case FileType::Seam:
        return filenameForID.remove("weldingSeam").remove(".json").toInt();
    case FileType::Wobble:
        return filenameForID.remove("figureWobble").remove(".json").toInt();
    case FileType::Overlay:
        return filenameForID.remove("overlayFunction").remove(".json").toInt();
    case FileType::Basic:
        return filenameForID.remove("figureWobble").remove(".json").toInt();
    default:
        return 0;
    }
}

bool WobbleFigureEditor::checkIfPowerIsAnalog()
{
    return std::none_of(m_seamFigure.figure.begin(), m_seamFigure.figure.end(), [](const auto& actualPoint)
                        { return actualPoint.power > 1.0; });
}

void WobbleFigureEditor::createLaserPointsFromSeamFigure(bool isPowerAnalog)
{
    LaserPoint* lastPoint = nullptr;
    LaserPoint* currentPoint = nullptr;
    auto counter = m_figure->get_node_count();

    m_figure->resetLaserPoints();

    for (const auto& point : m_seamFigure.figure)
    {
        auto node = m_figure->insertLaserPoint();
        if (node)
        {
            //Insert points
            auto laserPoint = dynamic_cast<LaserPoint*>(node);
            laserPoint->setID(counter);
            laserPoint->getItem()->setMinimumSize({15, 15});
            laserPoint->getItem()->setResizable(false);
            laserPoint->getItem()->setRect({(point.endPosition.first * m_figureScale) - (15 * 0.5), -((point.endPosition.second * m_figureScale) + (15 * 0.5)), 15, 15});
            laserPoint->initConnectionToNodeItem();
            laserPoint->setType(FileType::Seam);
            laserPoint->setVelocity(point.velocity);
            if (!qFuzzyCompare(point.power, -1.0))
            {
                if (isPowerAnalog)
                {
                    laserPoint->setEditable(true);
                    laserPoint->setLaserPower(point.power * 100);
                }
                else
                {
                    laserPoint->setEditable(true);
                    laserPoint->setLaserPower(point.power);
                }
            }
            else
            {
                laserPoint->setEditable(false);
                laserPoint->setLaserPower(point.power);
            }
            if (!qFuzzyCompare(point.ringPower, -1.0))
            {
                laserPoint->setRingPower(point.ringPower * 100);
            }
            else
            {
                laserPoint->setRingPower(point.ringPower);
            }
            counter++;

            if (!lastPoint)
            {
                lastPoint = laserPoint;
            }
            else
            {
                if (currentPoint)
                {
                    lastPoint = currentPoint;
                }
                currentPoint = laserPoint;
            }
            if (lastPoint && currentPoint)
            {
                //Insert trajectories between points and get them the properties they need.
                auto connection = m_figure->insertConnection(lastPoint, currentPoint);
                if (connection)
                {
                    auto trajectory = dynamic_cast<LaserTrajectory*>(connection);
                    trajectory->setID(counter - 1); //ID is the ID of the start point
                    trajectory->setEditable(true);  //FIXME, check if speed from lastPoint to currentPoint changed
                    trajectory->setSpeed(30.0);
                    trajectory->setTrajectoryLength(calculateLengthBetweenTwoNodes(lastPoint, currentPoint));
                    trajectory->setTime(trajectory->trajectoryLength() / trajectory->speed());
                    trajectory->setGroup(0);
                    trajectory->setType(0);
                    lastPoint->modifyAttachedEdge();
                }
            }
        }
    }
    emit nodePositionUpdated();
}

void WobbleFigureEditor::createFromFigureCreator(std::vector<QPointF>& points)
{
    dbgCheckInvariants();
    unsigned int i = 0;
    LaserPoint* oldPoint = nullptr;

    if (m_figure->get_node_count()) //Insert new figure even if there are already other points or figures.
    {
        i = m_figure->get_node_count();
        oldPoint = dynamic_cast<LaserPoint*>(m_figure->get_nodes().at(i - 1));

        if (calculateLengthBetweenOneNodeAndPosition(oldPoint, {points.begin()->x() * m_figureScale, points.begin()->y() * m_figureScale}) < 0.05)
        {
            points.erase(points.begin());
        }

        for (const auto& point : points)
        {
            if (auto laserPoint = dynamic_cast<LaserPoint*>(m_figure->insertLaserPoint()))
            {
                laserPoint->setID(i);
                laserPoint->setEditable(false);
                laserPoint->getItem()->setMinimumSize({15, 15});
                laserPoint->getItem()->setResizable(false);
                laserPoint->getItem()->setRect({(point.x() * m_figureScale) - (15 * 0.5), -1 * ((point.y() * m_figureScale) + (15 * 0.5)), 15, 15});
                laserPoint->initConnectionToNodeItem();
                laserPoint->setType(m_type);
                double power = m_type == FileType::Seam ? -1.0 : 0.0;
                laserPoint->setLaserPower(power);
                laserPoint->setRingPower(power);
                if (oldPoint && laserPoint && i != 0)
                {
                    createDefaultTrajectoryBetweenPoints(oldPoint, laserPoint);
                }
                oldPoint = laserPoint;
                i++;
            }
        }

        switch (m_type)
        {
        case FileType::Seam:
            for (const auto& point : points)
            {
                RTC6::seamFigure::command::Order newOrder;
                newOrder.endPosition = std::make_pair(point.x(), point.y());
                newOrder.power = -1.0;
                newOrder.ringPower = -1.0;
                newOrder.velocity = -1.0;
                m_seamFigure.figure.push_back(newOrder);
                emit seamPointsChanged();
            }
            setNumberOfPoints(m_seamFigure.figure.size());
            break;
        case FileType::Wobble:
            for (const auto& point : points)
            {
                RTC6::wobbleFigure::command::Order newOrder;
                newOrder.endPosition = std::make_pair(point.x(), point.y());
                newOrder.power = 0.0;
                newOrder.ringPower = 0.0;
                m_wobbelFigure.figure.push_back(newOrder);
            }
            setNumberOfPoints(m_wobbelFigure.figure.size());
            break;
        case FileType::Overlay:
            for (const auto& point : points)
            {
                m_overlayFigure.functionValues.emplace_back(point.x(), point.y());
            }
            setNumberOfPoints(m_overlayFigure.functionValues.size());
            break;
        default:
            break;
        }
        emit nodePositionUpdated();
        return;
    }

    for (const auto& point : points)
    {
        if (auto laserPoint = dynamic_cast<LaserPoint*>(m_figure->insertLaserPoint()))
        {
            laserPoint->setID(i);
            laserPoint->setEditable(false);
            laserPoint->getItem()->setMinimumSize({15, 15});
            laserPoint->getItem()->setResizable(false);
            laserPoint->getItem()->setRect({(point.x() * m_figureScale) - (15 * 0.5), -1 * ((point.y() * m_figureScale) + (15 * 0.5)), 15, 15});
            laserPoint->initConnectionToNodeItem();
            laserPoint->setType(m_type);
            double power = m_type == FileType::Seam ? -1.0 : 0.0;
            laserPoint->setLaserPower(power);
            laserPoint->setRingPower(power);
            if (oldPoint && laserPoint && i != 0)
            {
                createDefaultTrajectoryBetweenPoints(oldPoint, laserPoint);
            }
            oldPoint = laserPoint;
            i++;
        }
    }

    //cpp side
    switch (m_type)
    {
    case FileType::Seam:
        for (const auto& point : points)
        {
            RTC6::seamFigure::command::Order newOrder;
            newOrder.endPosition = std::make_pair(point.x(), point.y());
            newOrder.power = -1.0;
            newOrder.ringPower = -1.0;
            newOrder.velocity = -1.0;
            m_seamFigure.figure.push_back(newOrder);
        }
        m_seamFigure.name = "New figure";
        m_seamFigure.ID = "0";
        m_seamFigure.description = "No description";
        changeFigureProperties(QString::fromStdString(m_seamFigure.name), m_seamFigure.ID.empty() ? 0 : std::stoi(m_seamFigure.ID), QString::fromStdString(m_seamFigure.description));
        emit seamPointsChanged();
        break;
    case FileType::Wobble:
        for (const auto& point : points)
        {
            RTC6::wobbleFigure::command::Order newOrder;
            newOrder.endPosition = std::make_pair(point.x(), point.y());
            newOrder.power = 0.0;
            newOrder.ringPower = 0.0;
            m_wobbelFigure.figure.push_back(newOrder);
        }
        m_wobbelFigure.name = "New figure";
        m_wobbelFigure.ID = "0";
        m_wobbelFigure.description = "No description";
        m_wobbelFigure.microVectorFactor = 1;
        m_wobbelFigure.powerModulationMode = 1;
        changeFigureProperties(QString::fromStdString(m_wobbelFigure.name), m_wobbelFigure.ID.empty() ? 0 : std::stoi(m_wobbelFigure.ID), QString::fromStdString(m_wobbelFigure.description));
        break;
    case FileType::Overlay:
        for (const auto& point : points)
        {
            m_overlayFigure.functionValues.emplace_back(point.x(), point.y());
        }
        break;
        m_overlayFigure.name = "New figure";
        m_overlayFigure.ID = "0";
        m_overlayFigure.description = "No description";
        changeFigureProperties(QString::fromStdString(m_overlayFigure.name), m_overlayFigure.ID.empty() ? 0 : std::stoi(m_overlayFigure.ID), QString::fromStdString(m_overlayFigure.description));
    default:
        break;
    }
    setNumberOfPoints(points.size());
    dbgCheckInvariants();
}

// Checks invariants with assertions. Does nothing if NDEBUG is defined.
void WobbleFigureEditor::dbgCheckInvariants() const
{
#ifndef NDEBUG
    // inactive figure types should be unused, number of points should reflect pointcount of active figure type
    Q_ASSERT(m_wobbelFigure.figure.size() == (m_type == FileType::Wobble || m_type == FileType::Basic ? numberOfPoints() : 0));
    Q_ASSERT(m_seamFigure.figure.size() == (m_type == FileType::Seam ? numberOfPoints() : 0));
    Q_ASSERT(m_overlayFigure.functionValues.size() == (m_type == FileType::Overlay ? numberOfPoints() : 0));

    if (m_figure)
    {
        // positions of points in QML and cpp should be in sync
        for (unsigned int id = 0; id < numberOfPoints(); ++id)
        {
            LaserPoint const* lp = const_cast<WobbleFigure*>(m_figure)->searchLaserPoint(id);
            Q_ASSERT(lp->ID() == static_cast<int>(id));
            QPointF const qmlCenter = lp->center();

            std::pair<double, double> const endPos = [&]
            {
                switch (m_type)
                {
                case FileType::Wobble:
                case FileType::Basic:
                    return m_wobbelFigure.figure.at(id).endPosition;
                case FileType::Seam:
                    return m_seamFigure.figure.at(id).endPosition;
                case FileType::Overlay:
                    return m_overlayFigure.functionValues.at(id);
                default:
                    Q_UNREACHABLE();
                }
            }();

            QPointF const cppCenter(endPos.first, endPos.second);
            QPointF const qml2cpp(qmlCenter.x() / m_figureScale, -qmlCenter.y() / m_figureScale);
            auto const diff = QVector2D(qml2cpp - cppCenter);
            qreal const dist = diff.length();

            Q_ASSERT(qFuzzyIsNull(dist));
            Q_ASSERT(qFuzzyIsNull(diff.x()));
            Q_ASSERT(qFuzzyIsNull(diff.y()));
        }
    }
#endif
}

interface::ActionInterface_sp WobbleFigureEditor::getUndoActionPosition(RTC6::seamFigure::SeamFigure& figure, std::pair<double, double> const& oldPosition, std::pair<double, double> const& newPosition, LaserPoint* laserPoint)
{
    precitec::interface::ActionInterface_sp action =
        std::make_shared<ActionPointPositionSeamFigure>(
            *this,
            figure,
            oldPosition,
            newPosition,
            laserPoint->ID(),
            m_figureScale);
    return action;
}

interface::ActionInterface_sp WobbleFigureEditor::getUndoActionPosition(RTC6::wobbleFigure::Figure& figure, std::pair<double, double> const& oldPosition, std::pair<double, double> const& newPosition, LaserPoint* laserPoint)
{
    precitec::interface::ActionInterface_sp action =
        std::make_shared<ActionPointPositionWobbleFigure>(
            *this,
            figure,
            oldPosition,
            newPosition,
            laserPoint->ID(),
            m_figureScale);
    return action;
}

interface::ActionInterface_sp WobbleFigureEditor::getUndoActionPosition(RTC6::function::OverlayFunction& figure, std::pair<double, double> const& oldPosition, std::pair<double, double> const& newPosition, LaserPoint* laserPoint)
{
    precitec::interface::ActionInterface_sp action =
        std::make_shared<ActionPointPositionOverlay>(
            *this,
            figure,
            oldPosition,
            newPosition,
            laserPoint->ID(),
            m_figureScale);
    return action;
}

// NOTE: This function should be kept in sync with the text that is shown by LaserPointProperties.qml for the selected point.
QStringList WobbleFigureEditor::getLaserPointTooltip(int id) const
{
    dbgCheckInvariants();
    if (!m_figure)
    {
        return {};
    }

    LaserPoint const* lp = const_cast<WobbleFigure*>(m_figure)->searchLaserPoint(id);

    if (!lp || lp->isRampPoint())
    {
        return {};
    }

    double laserPower = lp->laserPower();
    double ringPower = lp->ringPower();
    double velocity = lp->velocity();

    for (int i = lp->ID() - 1; i >= 0 && (laserPower < 0 || ringPower < 0); --i)
    {
        auto update = [](double& dst, double src)
        {
            if (dst < 0 && src >= 0)
            {
                dst = src;
            }
        };

        LaserPoint const* p = const_cast<WobbleFigure*>(m_figure)->searchLaserPoint(i);
        update(laserPower, p->laserPower());
        update(ringPower, p->ringPower());
        update(velocity, p->velocity());
    }

    QLocale locale;
    auto dblStr = [&](double value)
    {
        return locale.toString(value, 'f', 2);
    };

    auto intStr = [&](double value)
    {
        return locale.toString(value, 'f', 0);
    };

    QString const prod = tr("Product definition");

    QStringList ret;

    ret.push_back(tr("ID:"));
    ret.push_back(locale.toString(id));

    ret.push_back(m_type == FileType::Wobble || m_type == FileType::Basic ? tr("Parallel/Vertical:") : tr("X/Y:"));
    ret.push_back(QString(tr("%1/%2 mm")).arg(dblStr(lp->center().x() / m_figureScale)).arg(dblStr(-lp->center().y() / m_figureScale)));

    if (m_type != FileType::Basic)
    {
        ret.push_back(tr("Power (Center):"));
        ret.push_back(laserPower < 0 ? prod : intStr(laserPower) + " %");

        // Condition taken from ringPowerLabel in LaserPointProperties.qml
        if (m_figure->analogPower() && FigureEditorSettings::instance()->dualChannelLaser() && fileType() != FileType::Overlay)
        {
            ret.push_back(tr("Power (Ring):"));
            ret.push_back(ringPower < 0 ? prod : intStr(ringPower) + " %");
        }

        if (m_type != FileType::Wobble)
        {
            ret.push_back("Velocity:");
            ret.push_back(velocity < 0 ? prod : intStr(velocity) + " mm/s");
        }
    }

    return ret;
}

QPointF WobbleFigureEditor::getLastPosition() const
{
    dbgCheckInvariants();
    if (auto lp = m_figure->searchLaserPoint(m_numberOfPoints - 1))
    {
        QPointF c = lp->center() / m_figureScale;
        return {c.x(), -c.y()};
    }

    Q_ASSERT(!"this function doesn't provide a meaningful value when there are no points!");
    return QPointF(0, 0);
}

}
}
}
}
