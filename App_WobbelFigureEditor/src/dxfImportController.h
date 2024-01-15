#pragma once

#include <QAbstractListModel>
#include "editorDataTypes.h"
#include "WobbleFigure.h"
#include <dxfreader.h>
#include <frnn.h>

namespace precitec
{

namespace scantracker
{
namespace components
{
namespace wobbleFigureEditor
{
class WobbleFigureEditor;
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

/**
Serves import and preview of a DXF file as seam figure.

Implements QAbstractListModel for the list of imported segments.
**/
class DxfImportController : public QAbstractListModel
{
public:
    enum DxfUnit
    { // TODO: complete... or reuse nsDxfReader::Unit
        FromFile,
        Millimeter,
        Centimeter,
        Inches,
    };

    Q_ENUM(DxfUnit)

    struct Segment
    {
        // TODO: try if identification of cyclic non-circular works well enough (special because start==end), maybe find a way to improve it
        size_t index;        // index of the path that this segment referes to
        bool curved = false; // flag if this segment contains curved paths (it makes sense to configure accuracy for it)

        // used for reidentification
        bool isCircle = false;
        std::optional<nsDxfReader::CircularDesc> circular; // only present if this is based on a circle or closed ellipse
        nsDxfReader::Point2 start, end;                    // only used if this is not circular

        bool reverse = false;

        double startAngle = 0;          // only used for circles
        std::optional<double> accuracy; // only used for curved elements, optional override for Config::accuracy
        std::optional<double> maxDist;  // optional override for Config::maxDist
    };

    struct Config
    {
        double accuracy = .5;          // accuracy for curved elements (maximum deviation for approximation with line segments)
        std::optional<double> maxDist; // maximum distance of successive points
        DxfUnit unit = FromFile;
        QString name;
        int id = 0;
        QString description;
        std::vector<Segment> segments;
    };

private:
    class PathConfigProvider : public nsDxfReader::IPathConfigProvider
    {
        struct CircleInfo
        {
            nsDxfReader::CircularDesc desc;
            double startAngle;
            bool used = false;
        };

        DxfImportController& ctl;

        // helper structures for looking up circles by inexact position
        Frnn2D<double, CircleInfo*> circleConfigFrnn;
        std::vector<CircleInfo> circleConfigs;
    public:
        PathConfigProvider(DxfImportController& ctl);
        int pass = -1;
        std::vector<Segment const*> sourcePathIdx2Segment;

        double GetCircleStartAngle(size_t pathIdx, nsDxfReader::CircularDesc const& circleDesc) override;
        std::optional<double> GetMaxDist(size_t pathIdx) override;
        double GetMaxError(size_t pathIdx) override;
    };

    Config m_config;
    double& m_accuracy = m_config.accuracy;
    std::optional<double>& m_maxDist = m_config.maxDist;
    DxfUnit& m_unit = m_config.unit;
    QString& m_name = m_config.name;
    int& m_id = m_config.id;
    QString& m_description = m_config.description;
    std::vector<Segment>& m_segments = m_config.segments;

    Q_OBJECT

    /**
     * Laser point controller object
     * Is used to draw the figure.
     **/
    Q_PROPERTY(precitec::scanmaster::components::wobbleFigureEditor::LaserPointController* laserPointController READ laserPointController WRITE setLaserPointController NOTIFY laserPointControllerChanged REQUIRED)
    Q_PROPERTY(precitec::scantracker::components::wobbleFigureEditor::WobbleFigureEditor* figureEditor READ figureEditor WRITE setFigureEditor NOTIFY figureEditorChanged REQUIRED)
    Q_PROPERTY(QString dxfPath WRITE setDxfPath MEMBER m_dxfPath NOTIFY dxfPathChanged REQUIRED)

    Q_PROPERTY(double accuracy WRITE setAccuracy MEMBER m_accuracy NOTIFY accuracyChanged)
    Q_PROPERTY(double maxDist WRITE setMaxDist READ getMaxDist NOTIFY maxDistChanged)
    Q_PROPERTY(DxfUnit unit WRITE setUnit MEMBER m_unit NOTIFY unitChanged)
    Q_PROPERTY(QString name MEMBER m_name NOTIFY nameChanged)
    Q_PROPERTY(int figureId MEMBER m_id NOTIFY figureIdChanged)
    Q_PROPERTY(QString description MEMBER m_description NOTIFY descriptionChanged)

    Q_PROPERTY(int maxIndex READ getMaxIndex NOTIFY maxIndexChanged)
    Q_PROPERTY(QString errorMsg MEMBER m_errorMsg NOTIFY errorMsgChanged)

    Q_PROPERTY(QPointF center MEMBER m_center NOTIFY centerChanged)
    Q_PROPERTY(double size MEMBER m_size NOTIFY sizeChanged) // maximum of width/height of the bounding rectangle of all points

    Q_PROPERTY(QString fileUnit READ fileUnit NOTIFY updatedImport)

    Q_PROPERTY(int pointCount READ pointCount NOTIFY pointCountChanged)

    QString fileUnit() const;

    void tryCatchError(std::function<void()> f);

    void runImport(); ///< reads the DXF file and builds the figure

    void resetPostImport(); ///< resets everything except for the data that came from the DXF file
    void resetResults();    ///< resets everything except for the configuration

    void emitConfigChanged();

    RTC6::seamFigure::SeamFigure m_seam;
    QString m_dxfPath;
    QString m_errorMsg;

    precitec::scanmaster::components::wobbleFigureEditor::LaserPointController* m_laserPointController = nullptr;
    QMetaObject::Connection m_laserPointControllerDestroyedConnection;

    precitec::scantracker::components::wobbleFigureEditor::WobbleFigureEditor* m_figureEditor = nullptr;
    QMetaObject::Connection m_figureEditorDestroyedConnection;

    nsDxfReader::DxfData m_data;
    std::vector<nsDxfReader::Path> m_paths; // one path per segment (order of usage is defined by Segment::index)

    QPointF m_center = QPointF(0, 0);
    double m_size = 1;

public:
    DxfImportController(QObject* parent = nullptr);
    ~DxfImportController() override;

    int getMaxIndex() const { return m_segments.size() - 1; }
    int pointCount() const;

    precitec::scanmaster::components::wobbleFigureEditor::LaserPointController* laserPointController() const
    {
        return m_laserPointController;
    }
    void setLaserPointController(precitec::scanmaster::components::wobbleFigureEditor::LaserPointController* newLaserPointController);

    precitec::scantracker::components::wobbleFigureEditor::WobbleFigureEditor* figureEditor() const
    {
        return m_figureEditor;
    }
    void setFigureEditor(precitec::scantracker::components::wobbleFigureEditor::WobbleFigureEditor* newFigureEditor);

    void setDxfPath(QString dxfPath);
    void setAccuracy(double accuracy);
    void setMaxDist(double maxDist);
    double getMaxDist() { return m_maxDist.value_or(-1); }
    void setUnit(DxfUnit unit);

    Q_INVOKABLE void swapSegments(int oldIndex, int newIndex);
    Q_INVOKABLE void moveSegment(int oldIndex, int newIndex);
    Q_INVOKABLE void select(int segIndex);
    Q_INVOKABLE void restartWithDefaults();
    Q_INVOKABLE void finalizeImport();
    Q_INVOKABLE void renderSegments(); ///< uses the currently read DXF data (without re-reading it) to build the figure

    QHash<int, QByteArray> roleNames() const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;
    bool setData(const QModelIndex& index, const QVariant& value, int role) override;
    size_t m_modelLayoutSize = 0;

Q_SIGNALS:
    void laserPointControllerChanged();
    void figureEditorChanged();
    void accuracyChanged();
    void maxDistChanged();
    void unitChanged();
    void maxIndexChanged();
    void dxfPathChanged();
    void errorMsgChanged();
    void segmentSelected(int index);
    void updatedImport();
    void updatedFigure();
    void nameChanged();
    void figureIdChanged();
    void descriptionChanged();
    void centerChanged();
    void sizeChanged();
    void missingUnit();
    void pointCountChanged();

private Q_SLOTS:
    void onNodeClicked(qan::Node* node, QPointF pos);
};
}
}
}
}
