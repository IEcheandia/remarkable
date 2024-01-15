#pragma once

#include <QObject>
#include <QQmlComponent>

#include <QVector3D>

#include "fileType.h"

class LaserPointControllerTest;

namespace RTC6
{
namespace seamFigure
{
class SeamFigure;
namespace command
{
class Order;
}
}
namespace wobbleFigure
{
class Figure;
namespace command
{
class Order;
}
}
namespace function
{
class OverlayFunction;
}
}

namespace precitec
{

namespace scantracker
{
namespace components
{
namespace wobbleFigureEditor
{
class WobbleFigure;
class LaserPoint;
}
}
}
namespace scanmaster
{

namespace components
{

namespace wobbleFigureEditor
{

using precitec::scantracker::components::wobbleFigureEditor::LaserPoint;
using precitec::scantracker::components::wobbleFigureEditor::FileType;

/**
 * The LaserPointController is between the backend (WobbleFigureEditor, cpp & SimulationController, cpp) and the frontend (WobbleFigure, qml, visualization).
 * The controller ensures that the data of the backend is displayed correctly by the frontend in the figure editor.
 * This encapsulates the data from the visualization.
 **/
class LaserPointController : public QObject
{
    Q_OBJECT
    /**
     * Figure object
     * Is used to create the qml points and edges and holds current displayed points and edges.
     * Thus if there are any global changes (like position) to all points which are displayed
     * then this object contains and gives access to all the points.
     **/
    Q_PROPERTY(precitec::scantracker::components::wobbleFigureEditor::WobbleFigure* figure READ figure WRITE setFigure NOTIFY figureChanged)

    /**
     * Figure scale
     * Is a factor to convert the millimeters to pixels.
     * If the factor is too small then all points are on the same position.
     **/
    Q_PROPERTY(int figureScale READ figureScale WRITE setFigureScale NOTIFY figureScaleChanged)

    /**
     * Figure type
     * Contains the type of the current figure which is displayed.
     * The type is important to set the right properties of the qml points.
     **/
    Q_PROPERTY(precitec::scantracker::components::wobbleFigureEditor::FileType fileType READ fileType WRITE setFileType NOTIFY fileTypeChanged)

    /**
     * Power analog
     * Contains if the power of a seam figure is in percent or in digital bits.
     * The digital bits correspond to laser programs which are stored in the laser.
     **/
    Q_PROPERTY(bool powerAnalog READ powerAnalog WRITE setPowerAnalog NOTIFY powerAnalogChanged)

    /**
     * //TODO Power analog
     * Contains if the power of a seam figure is in percent or in digital bits.
     * The digital bits correspond to laser programs which are stored in the laser.
     **/
    Q_PROPERTY(bool visualizeRamps READ visualizeRamps WRITE setVisualizateRamps NOTIFY visualizeRampsChanged)

public:
    explicit LaserPointController( QObject* parent = nullptr);
    ~LaserPointController() override;

    precitec::scantracker::components::wobbleFigureEditor::WobbleFigure* figure() const
    {
        return m_figure;
    }
    void setFigure(precitec::scantracker::components::wobbleFigureEditor::WobbleFigure* wobbleFigure);

    int figureScale() const
    {
        return m_figureScale;
    }
    void setFigureScale(int newScaleFactor);

    precitec::scantracker::components::wobbleFigureEditor::FileType fileType() const
    {
        return m_fileType;
    }
    void setFileType(precitec::scantracker::components::wobbleFigureEditor::FileType type);

    bool powerAnalog() const
    {
        return m_powerAnalog;
    }
    void setPowerAnalog(bool isPowerAnalog);

    bool visualizeRamps() const
    {
        return m_visualizeRamps;
    }
    void setVisualizateRamps(bool visualize);

    void drawSeamFigure(const RTC6::seamFigure::SeamFigure& seam);
    void drawWobbleFigure(const RTC6::wobbleFigure::Figure& wobble);
    void drawOverlayFigure(const RTC6::function::OverlayFunction& overlay);
    void drawBasicFigure(const RTC6::wobbleFigure::Figure& basic);

    void drawSimulatedFigure(const RTC6::seamFigure::SeamFigure& simulatedFigure, const std::pair<unsigned int, int>& visualizationInformation);

    void drawRamps(const std::vector<QVector3D>& ramps);

    void drawFigureFromCreator(const std::vector<QPointF>& figure);

    void applyOffset(const QPointF& offset);

    void deletePoint(int id);
    void setNewIDs(int id);
    void closeCreatedGap(int id);

    void newStartPoint(int id, bool closed);

    void mirrorYAxis();

    void clearPoints();
    void clearFigure();

    void setPointsAreModifiable(bool modifiable);

Q_SIGNALS:
    void figureChanged();
    void figureScaleChanged();
    void fileTypeChanged();
    void powerAnalogChanged();
    void visualizeRampsChanged();

private:
    void changeFigureScale();
    QPointF pointFromPair(const std::pair<double, double>& pair);
    void setItemInformation(LaserPoint* laserPoint);
    void setBasicItemInformation(LaserPoint* laserPoint);
    void setLaserPointInformation(LaserPoint* laserPoint, int id, const std::pair<double, double>& centerPosition);
    void setLaserPointInformation(LaserPoint* laserPoint, int id, const QPointF& centerPosition);
    void transferSeamInformationToLaserPoint(LaserPoint* laserPoint, const RTC6::seamFigure::command::Order& point);
    void transferWobbleInformationToLaserPoint(LaserPoint* laserPoint, const RTC6::wobbleFigure::command::Order& point);
    void setFigureProperties(const RTC6::seamFigure::SeamFigure& seam);
    void setFigureProperties(const RTC6::wobbleFigure::Figure& wobble);
    void setFigureProperties(const RTC6::function::OverlayFunction& overlay);
    template <typename T>
    void setFigurePropertiesImpl(T figure);
    void createDefaultTrajectory(LaserPoint* sourcePoint, LaserPoint* destinationPoint);
    QPointF calculateFigureCenter();
    void deleteAllEdges();

    void changeRampVisibility();
    LaserPoint* createRampPoint(const QVector3D& rampEndPoint);
    void changeRampPointVisibility();
    void createRampTrajectory(LaserPoint* sourcePoint, LaserPoint* destinationPoint);
    void changeRampTrajectoryVisibility();
    void deleteRampPoints();

    QPointF invertY(const QPointF& point);

    precitec::scantracker::components::wobbleFigureEditor::WobbleFigure* m_figure = nullptr;
    QMetaObject::Connection m_figureDestroyedConnection;

    int m_oldFigureScale{1};
    int m_figureScale{1000};

    FileType m_fileType{FileType::Seam};

    bool m_powerAnalog{true};
    bool m_visualizeRamps{true};

    int m_laserPointSize = 15;

    QQmlComponent* m_qmlComponent = nullptr;
    QQmlComponent* m_qmlComponentEdge = nullptr;
    friend LaserPointControllerTest;
};

}
}
}
}
