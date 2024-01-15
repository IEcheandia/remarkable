#include "laserPointController.h"

#include "WobbleFigure.h"
#include "LaserPoint.h"
#include "editorDataTypes.h"
#include "LaserTrajectory.h"

using precitec::scantracker::components::wobbleFigureEditor::WobbleFigure;
using precitec::scantracker::components::wobbleFigureEditor::LaserPoint;
using precitec::scantracker::components::wobbleFigureEditor::LaserTrajectory;
using precitec::scantracker::components::wobbleFigureEditor::FileType;

namespace precitec
{
namespace scanmaster
{
namespace components
{
namespace wobbleFigureEditor
{

LaserPointController::LaserPointController(QObject* parent) : QObject(parent)
{
    connect(this, &LaserPointController::figureScaleChanged, this, &LaserPointController::changeFigureScale);
    connect(this, &LaserPointController::visualizeRampsChanged, this, &LaserPointController::changeRampVisibility);
}

LaserPointController::~LaserPointController() = default;

void LaserPointController::setFigure(WobbleFigure* wobbleFigure)
{
    if (m_figure == wobbleFigure)
    {
        return;
    }

    disconnect(m_figureDestroyedConnection);
    m_figure = wobbleFigure;

    if (m_figure)
    {
        m_figureDestroyedConnection = connect(m_figure, &QObject::destroyed, this, std::bind(&LaserPointController::setFigure, this, nullptr));
    }
    else
    {
        m_figureDestroyedConnection = {};
    }
    emit figureChanged();
}

void LaserPointController::setFigureScale(int newScaleFactor)
{
    if (m_figureScale == newScaleFactor)
    {
        return;
    }

    m_oldFigureScale = m_figureScale;
    m_figureScale = newScaleFactor;
    emit figureScaleChanged();
}

void LaserPointController::setFileType(precitec::scantracker::components::wobbleFigureEditor::FileType type)
{
    if (m_fileType == type)
    {
        return;
    }

    m_fileType = type;
    emit fileTypeChanged();
}

void LaserPointController::setPowerAnalog(bool isPowerAnalog)
{
    if (m_powerAnalog == isPowerAnalog)
    {
        return;
    }

    m_powerAnalog = isPowerAnalog;
    emit powerAnalogChanged();
}

void LaserPointController::setVisualizateRamps(bool visualize)
{
    if (m_visualizeRamps == visualize)
    {
        return;
    }

    m_visualizeRamps = visualize;
    emit visualizeRampsChanged();
}

void LaserPointController::drawSeamFigure(const RTC6::seamFigure::SeamFigure& seam)
{
    if (!m_figure || seam.figure.empty())
    {
        return;
    }

    clearFigure();

    m_figure->setAnalogPower(m_powerAnalog);
    LaserPoint* lastPoint = nullptr;
    auto counter = m_figure->get_node_count();

    for (const auto &point : seam.figure)
    {
        if (auto node = m_figure->insertLaserPoint(m_qmlComponent))
        {
            if (auto laserPoint = qobject_cast<LaserPoint*>(node))
            {
                setItemInformation(laserPoint);
                setLaserPointInformation(laserPoint, counter, point.endPosition);
                transferSeamInformationToLaserPoint(laserPoint, point);
                createDefaultTrajectory(lastPoint, laserPoint);

                lastPoint = laserPoint;
                counter++;
            }
        }
    }
    setFigureProperties(seam);
}

void LaserPointController::drawWobbleFigure(const RTC6::wobbleFigure::Figure& wobble)
{
    if (!m_figure || wobble.figure.empty())
    {
        return;
    }

    clearFigure();

    LaserPoint* lastPoint = nullptr;
    auto counter = m_figure->get_node_count();

    for (const auto& point : wobble.figure)
    {
        if (auto node = m_figure->insertLaserPoint(m_qmlComponent))
        {
            if (auto laserPoint = qobject_cast<LaserPoint*>(node))
            {
                setItemInformation(laserPoint);
                setLaserPointInformation(laserPoint, counter, point.endPosition);
                transferWobbleInformationToLaserPoint(laserPoint, point);
                createDefaultTrajectory(lastPoint, laserPoint);

                lastPoint = laserPoint;
                counter++;
            }
        }
    }
    setFigureProperties(wobble);
}

void LaserPointController::drawOverlayFigure(const RTC6::function::OverlayFunction& overlay)
{
    if (!m_figure || overlay.functionValues.empty())
    {
        return;
    }

    clearFigure();

    LaserPoint* lastPoint = nullptr;
    auto counter = m_figure->get_node_count();

    for (auto &point : overlay.functionValues)
    {
        if (auto overlayPoint = m_figure->insertLaserPoint(m_qmlComponent))
        {
            if (auto laserPoint = qobject_cast<LaserPoint*>(overlayPoint))
            {
                setItemInformation(laserPoint);

                setLaserPointInformation(laserPoint, counter, point);

                createDefaultTrajectory(lastPoint, laserPoint);

                lastPoint = laserPoint;
                counter++;
            }
        }
    }
    setFigureProperties(overlay);
}

void LaserPointController::drawBasicFigure(const RTC6::wobbleFigure::Figure& basic)
{
    if (!m_figure || basic.figure.empty())
    {
        return;
    }

    clearFigure();

    LaserPoint* lastPoint = nullptr;
    auto counter = m_figure->get_node_count();

    for (auto &point : basic.figure)
    {
        if (auto basicPoint = m_figure->insertLaserPoint(m_qmlComponent))
        {
            if (auto laserPoint = qobject_cast<LaserPoint*>(basicPoint))
            {
                setItemInformation(laserPoint);
                setBasicItemInformation(laserPoint);
                setLaserPointInformation(laserPoint, counter, point.endPosition);
                createDefaultTrajectory(lastPoint, laserPoint);

                lastPoint = laserPoint;
                counter++;
            }
        }
    }
    setFigureProperties(basic);
}

void LaserPointController::drawSimulatedFigure(const RTC6::seamFigure::SeamFigure& simulatedFigure, const std::pair<unsigned int, int>& visualizationInformation)
{
    if (!m_figure || simulatedFigure.figure.empty())
    {
        return;
    }

    clearFigure();

    m_figure->setAnalogPower(m_powerAnalog);
    LaserPoint* lastPoint = nullptr;
    auto counter = m_figure->get_node_count();
    auto loopCountInPoints = visualizationInformation.first;
    auto deltaTFactorOf10us = visualizationInformation.second;

    for (std::size_t i = 0; i < simulatedFigure.figure.size();)
    {
        if (auto node = m_figure->insertLaserPoint(m_qmlComponent))
        {
            if (auto laserPoint = qobject_cast<LaserPoint*>(node))
            {
                const auto& point = simulatedFigure.figure.at(i);
                setItemInformation(laserPoint);
                setLaserPointInformation(laserPoint, counter, point.endPosition);
                transferSeamInformationToLaserPoint(laserPoint, point);
                createDefaultTrajectory(lastPoint, laserPoint);

                lastPoint = laserPoint;
                counter++;
            }
        }
        if (loopCountInPoints != 0 && i >= loopCountInPoints)
        {
            auto offset = calculateFigureCenter();
            offset.setX(offset.x() * -1.0);
            applyOffset(offset / m_figureScale);
            return;
        }
        i += deltaTFactorOf10us;
    }

    setFigureProperties(simulatedFigure);
}

void LaserPointController::drawRamps(const std::vector<QVector3D>& ramps)
{
    if (!m_figure)
    {
        return;
    }

    deleteRampPoints();

    for (const auto& ramp : ramps)
    {
        if (auto laserPoint = createRampPoint(ramp))
        {
            auto rampStart = m_figure->searchPoint(ramp.z());
            createRampTrajectory(rampStart, laserPoint);
        }
    }
}

void LaserPointController::drawFigureFromCreator(const std::vector<QPointF>& figure)
{
    if (!m_figure)
    {
        return;
    }

    LaserPoint* lastPoint = nullptr;
    auto counter = m_figure->get_node_count();

    if (counter == 0)
    {
        for (const auto& point : figure)
        {
            if (auto laserPoint = qobject_cast<LaserPoint*> (m_figure->insertLaserPoint(m_qmlComponent)))
            {
                setItemInformation(laserPoint);
                setLaserPointInformation(laserPoint, counter, point);
                createDefaultTrajectory(lastPoint, laserPoint);

                lastPoint = laserPoint;
                counter++;
            }
        }
        return;
    }

    lastPoint = qobject_cast<LaserPoint*> (m_figure->get_nodes().at(counter - 1));
    if (!lastPoint)
    {
        return;
    }

    for (const auto& point : figure)
    {
        if (auto laserPoint = qobject_cast<LaserPoint*> (m_figure->insertLaserPoint(m_qmlComponent)))
        {
            setItemInformation(laserPoint);
            setLaserPointInformation(laserPoint, counter, point);
            createDefaultTrajectory(lastPoint, laserPoint);

            lastPoint = laserPoint;
            counter++;
        }
    }
}

void LaserPointController::applyOffset(const QPointF& offset)
{
    if (!m_figure || offset.isNull())
    {
        return;
    }

    for (const auto& point : m_figure->get_nodes())
    {
        if (auto laserPoint = qobject_cast<LaserPoint*> (point))
        {
            laserPoint->setCenter(laserPoint->center() + (offset * m_figureScale));
        }
    }
}

void LaserPointController::deletePoint(int id)
{
    if (!m_figure)
    {
        return;
    }

    auto point = std::find_if(m_figure->get_nodes().begin(), m_figure->get_nodes().end(), [id] (const auto& currentPoint)
    {
        return qobject_cast<LaserPoint*> (currentPoint)->ID() == id;
    });
    if (point == m_figure->get_nodes().end())
    {
        return;
    }
    m_figure->removeNode(*point);
}

void LaserPointController::setNewIDs(int id)
{
    if (!m_figure || id < 0 || std::size_t(id) >= m_figure->get_node_count() || m_figure->get_node_count() == 0)
    {
        return;
    }

    for (std::size_t i = id; i < m_figure->get_node_count(); i++)
    {
        if (auto laserPoint = qobject_cast<LaserPoint*> (m_figure->get_nodes().at(i)))
        {
            laserPoint->setID(i);
            if (laserPoint->get_out_edges().size() == 1)
            {
                if (auto laserTrajectory = qobject_cast<LaserTrajectory*> (laserPoint->get_out_edges().at(0)))
                {
                    laserTrajectory->setID(i);
                }
            }
        }
    }
}

void LaserPointController::closeCreatedGap(int id)
{
    if (!m_figure || id <= 0 || std::size_t(id) >= m_figure->get_node_count())
    {
        return;
    }

    if (auto pointBeforeRemovedOne = qobject_cast<LaserPoint*> (m_figure->get_nodes().at(id - 1)))
    {
        if (auto pointAfterRemovedOne = qobject_cast<LaserPoint*> (m_figure->get_nodes().at(id)))
        {
            if (pointBeforeRemovedOne->get_out_edges().size() == 0 && pointAfterRemovedOne->get_in_edges().size() == 0)
            {
                createDefaultTrajectory(pointBeforeRemovedOne, pointAfterRemovedOne);
            }
        }
    }
}

void LaserPointController::newStartPoint(int id, bool closed)
{
    if (!m_figure || id <= 0 || std::size_t(id) >= (m_figure->get_node_count() - 1))
    {
        return;
    }

    if (closed)
    {
        m_figure->clear();
        return;
    }

    for (int i = 0; i < id; i++)
    {
        deletePoint(i);
    }
    setNewIDs(0);
}

void LaserPointController::mirrorYAxis()
{
    if (!m_figure || m_figure->get_node_count() == 0)
    {
        return;
    }

    for (auto &point : m_figure->get_nodes())
    {
        if (auto laserpoint = qobject_cast<LaserPoint*> (point))
        {
            laserpoint->setCenter({laserpoint->center().x(), -1.0 * laserpoint->center().y()});
        }
    }
}

void LaserPointController::clearPoints()
{
    if (!m_figure || m_figure->get_node_count() == 0)
    {
        return;
    }

    m_figure->clear();
}

void LaserPointController::clearFigure()
{
    if (!m_figure)
    {
        return;
    }

    m_figure->clear();
    m_figure->setName({tr("Figure name")});
    m_figure->setID(0);
    m_figure->setDescription({});
    m_figure->setAnalogPower(true);
}

void LaserPointController::setPointsAreModifiable(bool modifiable)
{
    if (!m_figure)
    {
        return;
    }

    for (const auto& node : m_figure->get_nodes())
    {
        node->getItem()->setSelectable(modifiable);
        node->getItem()->setDraggable(modifiable);
    }
}

void LaserPointController::changeFigureScale()
{
    if (!m_figure)
    {
        return;
    }

    for (auto &point : m_figure->get_nodes())
    {
        if (auto laserPoint = qobject_cast<LaserPoint*>(point))
        {
            laserPoint->setCenter(laserPoint->center() / m_oldFigureScale * m_figureScale);
        }
    }
}

QPointF LaserPointController::pointFromPair(const std::pair<double, double>& pair)
{
    return {pair.first, pair.second};
}

void LaserPointController::setItemInformation(LaserPoint* laserPoint)
{
    laserPoint->getItem()->setMinimumSize({static_cast<qreal> (m_laserPointSize), static_cast<qreal> (m_laserPointSize)});
    laserPoint->getItem()->setResizable(false);
    laserPoint->getItem()->setWidth(static_cast<qreal> (m_laserPointSize));
    laserPoint->getItem()->setHeight(static_cast<qreal> (m_laserPointSize));
}

void LaserPointController::setBasicItemInformation(LaserPoint* laserPoint)
{
    laserPoint->getItem()->setSelectable(false);
    laserPoint->getItem()->setDraggable(false);
    laserPoint->getItem()->setResizable(false);
}

void LaserPointController::setLaserPointInformation(LaserPoint* laserPoint, int id, const std::pair<double, double>& centerPosition)
{
    laserPoint->setID(id);
    laserPoint->setCenter(invertY(pointFromPair(centerPosition) * m_figureScale));
    laserPoint->initConnectionToNodeItem();
    laserPoint->setType(m_fileType);
}

void LaserPointController::setLaserPointInformation(LaserPoint* laserPoint, int id, const QPointF& centerPosition)
{
    laserPoint->setID(id);
    laserPoint->setCenter(invertY(centerPosition * m_figureScale));
    laserPoint->initConnectionToNodeItem();
    laserPoint->setType(m_fileType);
}

void LaserPointController::transferSeamInformationToLaserPoint(LaserPoint* laserPoint, const RTC6::seamFigure::command::Order& point)
{
    if (qFuzzyCompare(point.power, -1.0))
    {
        laserPoint->setLaserPower(point.power);
    }
    else
    {
        if (m_powerAnalog)
        {
            laserPoint->setLaserPower(point.power * 100);
        }
        else
        {
            laserPoint->setLaserPower(point.power);
        }
    }
    if (qFuzzyCompare(point.ringPower, -1.0))
    {
        laserPoint->setRingPower(point.ringPower);
    }
    else
    {
        laserPoint->setRingPower(point.ringPower * 100);
    }
    laserPoint->setVelocity(point.velocity);
}

void LaserPointController::transferWobbleInformationToLaserPoint(LaserPoint* laserPoint, const RTC6::wobbleFigure::command::Order& point)
{
    laserPoint->setLaserPower(point.power * 100);
    laserPoint->setRingPower(point.ringPower * 100);
}

void LaserPointController::setFigureProperties(const RTC6::seamFigure::SeamFigure& seam)
{
    setFigurePropertiesImpl(seam);
}

void LaserPointController::setFigureProperties(const RTC6::wobbleFigure::Figure& wobble)
{
    setFigurePropertiesImpl(wobble);
}

void LaserPointController::setFigureProperties(const RTC6::function::OverlayFunction& overlay)
{
    setFigurePropertiesImpl(overlay);
}

template<typename T>
void LaserPointController::setFigurePropertiesImpl(T figure)
{
    if (!m_figure)
    {
        return;
    }

    m_figure->setName(QString::fromStdString(figure.name));
    m_figure->setID(std::stoi(figure.ID));
    m_figure->setDescription(QString::fromStdString(figure.description));
}

void LaserPointController::createDefaultTrajectory(LaserPoint* sourcePoint, LaserPoint* destinationPoint)
{
    if (!m_figure || !sourcePoint || !destinationPoint || sourcePoint == destinationPoint)
    {
        return;
    }

    if (auto connection = m_figure->insertConnection(sourcePoint, destinationPoint, m_qmlComponentEdge))
    {
        if (auto trajectory = qobject_cast<LaserTrajectory*> (connection))
        {
            trajectory->setID(sourcePoint->ID());
            if (m_fileType == FileType::Seam)
            {
                sourcePoint->modifyAttachedEdge();
            }
        }
    }
}

QPointF LaserPointController::calculateFigureCenter()
{
    if (!m_figure || m_figure->get_node_count() == 0)
    {
        return {};
    }

    const auto& xMinMax = std::minmax_element(m_figure->get_nodes().begin(), m_figure->get_nodes().end(),
        [](const auto& node1, const auto& node2)
        {
            return qobject_cast<LaserPoint*> (node1)->center().x() < qobject_cast<LaserPoint*> (node2)->center().x();
        });

    const auto& yMinMax = std::minmax_element(m_figure->get_nodes().begin(), m_figure->get_nodes().end(),
        [](const auto& node1, const auto& node2)
        {
            return qobject_cast<LaserPoint*> (node1)->center().y() < qobject_cast<LaserPoint*> (node2)->center().y();
        });

    QRectF object{
        QPointF(qobject_cast<LaserPoint*>(*xMinMax.first)->center().x(),
        qobject_cast<LaserPoint*>(*yMinMax.second)->center().y()),
        QPointF(qobject_cast<LaserPoint*>(*xMinMax.second)->center().x(), qobject_cast<LaserPoint*>(*yMinMax.first)->center().y())
        };

    return object.center();
}

void LaserPointController::deleteAllEdges()
{
    if (!m_figure || m_figure->get_edge_count() == 0)
    {
        return;
    }

    std::vector<qan::Edge*> edges;
    edges.reserve(m_figure->get_edge_count());
    std::copy(m_figure->get_edges().begin(), m_figure->get_edges().end(), std::back_inserter(edges));

    for (auto const& selectedEdge : edges)
    {
        if (auto edge = selectedEdge)
        {
            m_figure->removeEdge(edge);
        }
    }
}

void LaserPointController::changeRampVisibility()
{
    if (!m_figure)
    {
        return;
    }

    changeRampPointVisibility();
    changeRampTrajectoryVisibility();
}

LaserPoint* LaserPointController::createRampPoint(const QVector3D& rampEndPoint)
{
    if (!m_figure)
    {
        return {};
    }

    if (auto node = m_figure->insertLaserPoint(m_qmlComponent))
    {
        if (auto laserPoint = qobject_cast<LaserPoint*>(node))
        {
            laserPoint->setIsRampPoint(true);
            laserPoint->setCenter(invertY(rampEndPoint.toPointF()) * m_figureScale);                //Y axis is mirrored
            laserPoint->getItem()->setSelectable(false);
            laserPoint->getItem()->setDraggable(false);
            laserPoint->getItem()->setResizable(false);
            laserPoint->getItem()->setVisible(m_visualizeRamps);

            return laserPoint;
        }
    }

    return {};
}

void LaserPointController::changeRampPointVisibility()
{
    if (!m_figure)
    {
        return;
    }

    const auto& rampPoints = m_figure->rampPoints();

    for (const auto& point : rampPoints)
    {
        point->getItem()->setVisible(m_visualizeRamps);
    }
}

void LaserPointController::createRampTrajectory(LaserPoint* sourcePoint, LaserPoint* destinationPoint)
{
    if (!m_figure || !sourcePoint || !destinationPoint || sourcePoint == destinationPoint)
    {
        return;
    }

    if (auto connection = m_figure->insertConnection(sourcePoint, destinationPoint, m_qmlComponentEdge))
    {
        if (auto trajectory = qobject_cast<LaserTrajectory*> (connection))
        {
            trajectory->setID(sourcePoint->ID());
            trajectory->setIsRampEdge(true);
            trajectory->getItem()->setSrcShape(qan::EdgeStyle::ArrowShape::Rect);
            trajectory->getItem()->setDstShape(qan::EdgeStyle::ArrowShape::Rect);
            trajectory->getItem()->setVisible(m_visualizeRamps);
        }
    }
}

void LaserPointController::changeRampTrajectoryVisibility()
{
    if (!m_figure)
    {
        return;
    }

    for (const auto& edge : m_figure->get_edges())
    {
        if (auto trajectory = qobject_cast<LaserTrajectory*> (edge))
        {
            if (trajectory->isRampEdge())
            {
                trajectory->getItem()->setVisible(m_visualizeRamps);
            }
        }
    }
}

void LaserPointController::deleteRampPoints()
{
    if (!m_figure)
    {
        return;
    }

    const auto& rampPoints = m_figure->rampPoints();

    for (const auto& point : rampPoints)
    {
        m_figure->removeNode(point);
    }
}

QPointF LaserPointController::invertY(const QPointF& point)
{
    return QPointF(point.x(), -1.0 * point.y());
}

}
}
}
}

