#include "dxfImportController.h"
#include "laserPointController.h"
#include "WobbleFigure.h"
#include "WobbleFigureEditor.h"
#include "LaserPoint.h"
#include <iostream>
#include <json.hpp>
#include <fstream>

/*

"circular element" is used as umbrella term to refer to circles and closed ellipses in the imported data.
This is useful because identification of such elements is done by looking at their center and minor/major
axis. Elements with distinct start- and end-positions are identified by a pair of that positions instead.

"cyclic element" is used for all closed paths. In addition to "circualar elements" this also includes closed
polylines and splines. For such paths the end point is omitted in nsDxfReader::Path::points because it is
identical to the start point. This is useful when moving the start position around by simply using std::rotate
but has to be kept in mind when the number of points or the last point of a path is needed. In hindsight this
was probably not the best decision because it adds a bunch of accidental special cases.

All other supported elements (arcs, lines, open polylines, open splines, ellipses) have a well defined start and end
(but not necessarily a direction).

Important features of the user interface for the import:

 - defining the order in which the imported paths are used to produce the final sequence of points
 - per-path flag to reverse the order of individual path segments
 - definition of the starting point for circles by setting the angle

This means that the user needs to assign some configuration to individual path segments. This configuration
has to support persistence so the user can reapply a previous configuration for the same or a slightly updated
DXF file for later imports. Elements in DXF files have no user-visible tag or ID to identify them. There is
even the possibility that the user defines "blocks" (groups of entities) that are instantiated multiple times
(with different transformation).

Of course an additional motivation is to keep the implementation of the import mechanism as simple as possible
without sacrificing significant flexibility and usability for the typical use cases.

The rough steps during import are:

- The DXF file is read into a datastructure that represents the raw DXF-Entities (circles, polylines etc.).
- If an import-configuration for the imported file is present it is read (a simple json-file next to the DXF-file).
- All elements are converted to point-lists (nsDxfReader::Path). Curved elements (splines, arcs, circles, ellipses)
  are sampled in a way that keeps their deviation from the ideal path below a user defined distance.
  For circles the starting-angle is queried by a callback. In this callback we search the import-configuration
  for settings of an element with the given center and transformation (major/minor-axis).
- All non-cyclic paths that touch at the ends are joined. Ends that are located at the start/end of a cyclic path are
  not joined (to make sure the user can decide the order and direction of the individual paths that meet at such positions).
  This can result in cyclic paths. The user can control the starting position by making sure the end
  of another path ends at it (so there are at least three ends meeting at that position). This will cause
  that position to interrupt the cycle.
- Existing configurations for non-circular elements are set (they are reidentified by the position of their start- and end-position)
  TODO: This is ambigious when multiple cyclic paths start and end at the same position!
- The "reverse segment"-Flag of each segment is used to reverse the oder of points in the segments
- The segments are joined to produce the sequence of laser points for the figure editor
*/

namespace nlohmann
{
template<typename T>
struct adl_serializer<std::optional<T>>
{
    static void to_json(json& j, const std::optional<T>& opt)
    {
        if (!opt)
        {
            j = nullptr;
        }
        else
        {
            j = *opt; // this will call adl_serializer<T>::to_json which will
                      // find the free function to_json in T's namespace!
        }
    }

    static void from_json(const json& j, std::optional<T>& opt)
    {
        if (j.is_null())
        {
            opt.reset();
        }
        else
        {
            opt = j.get<T>(); // same as above, but with
                              // adl_serializer<T>::from_json
        }
    }
};
}

void to_json(nlohmann::json& j, QString const& s)
{
    j = s.toStdString();
}

void from_json(nlohmann::json const& j, QString& s)
{
    s = QString::fromStdString(j);
}

using precitec::scantracker::components::wobbleFigureEditor::WobbleFigure;

namespace
{

QString exceptionMessage(std::exception_ptr ep);

template<class T>
QString _nestedExceptionMessage(T& e)
{
    try
    {
        std::rethrow_if_nested(e);
    }
    catch (...)
    {
        return "\nbecause: " + exceptionMessage(std::current_exception());
    }

    return {};
}

QString exceptionMessage(std::exception_ptr ep)
{
    try
    {
        std::rethrow_exception(ep);
    }
    catch (std::exception& e)
    {
        return e.what() + _nestedExceptionMessage(e);
    }
    catch (...)
    {
        Q_ASSERT(!"this should not happen");
        return "unknown error";
    }
}

QString exceptionMessage()
{
    return exceptionMessage(std::current_exception());
}

QFileInfo importCfgPath(QString dxfPath)
{
    QFileInfo fi(dxfPath);
    return fi.dir().path() + "/" + fi.baseName() + "-import.cfg";
}

} // namespace

namespace nsDxfReader
{
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(nsDxfReader::Point2, x, y)
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(nsDxfReader::Vec2, x, y)
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(nsDxfReader::CircularDesc, center, minor, major)
}

namespace precitec
{
namespace scanmaster
{
namespace components
{
namespace wobbleFigureEditor
{

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(DxfImportController::Segment,
                                   index,
                                   isCircle,
                                   circular,
                                   start,
                                   end,
                                   reverse,
                                   startAngle,
                                   accuracy,
                                   maxDist)

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(DxfImportController::Config,
                                   accuracy,
                                   maxDist,
                                   unit,
                                   name,
                                   id,
                                   description,
                                   segments)

NLOHMANN_JSON_SERIALIZE_ENUM(DxfImportController::DxfUnit, {
                                                               {DxfImportController::FromFile, "from file"},
                                                               {DxfImportController::Millimeter, "mm"},
                                                               {DxfImportController::Centimeter, "cm"},
                                                               {DxfImportController::Inches, "in"},
                                                           })

DxfImportController::DxfImportController(QObject* parent)
    : QAbstractListModel(parent)
{
}

DxfImportController::~DxfImportController() = default;

void DxfImportController::setLaserPointController(precitec::scanmaster::components::wobbleFigureEditor::LaserPointController* newLaserPointController)
{
    if (m_laserPointController == newLaserPointController)
    {
        return;
    }

    disconnect(m_laserPointControllerDestroyedConnection);
    m_laserPointController = newLaserPointController;

    if (m_laserPointController)
    {
        m_laserPointControllerDestroyedConnection = connect(m_laserPointController, &QObject::destroyed, this, std::bind(&DxfImportController::setLaserPointController, this, nullptr));
    }
    else
    {
        m_laserPointControllerDestroyedConnection = {};
    }
    emit laserPointControllerChanged();
}

void DxfImportController::setFigureEditor(precitec::scantracker::components::wobbleFigureEditor::WobbleFigureEditor* newFigureEditor)
{
    if (m_figureEditor == newFigureEditor)
    {
        return;
    }

    disconnect(m_figureEditorDestroyedConnection);
    m_figureEditor = newFigureEditor;

    if (m_figureEditor)
    {
        m_figureEditorDestroyedConnection = connect(m_figureEditor, &QObject::destroyed, this, std::bind(&DxfImportController::setFigureEditor, this, nullptr));
    }
    else
    {
        m_figureEditorDestroyedConnection = {};
    }
    emit figureEditorChanged();
}

void DxfImportController::setDxfPath(QString dxfPath)
{
    if (m_dxfPath == dxfPath)
    {
        return;
    }

    m_dxfPath = dxfPath;

    if (m_dxfPath.isEmpty())
    {
        m_config = Config();
        resetResults();
    }
    else
    {
        QFileInfo cfgPath = importCfgPath(m_dxfPath);
        bool cfgRestored = false;
        if (cfgPath.exists())
        {
            try
            {
                std::ifstream f(cfgPath.filePath().toStdString());
                f.exceptions(std::ifstream::failbit);

                nlohmann::ordered_json json;
                f >> json;

                nlohmann::from_json(json, m_config);
                cfgRestored = true;
            }
            catch (...)
            {
                m_config = Config();
            }
        }

        if (!cfgRestored)
            m_config.name = QFileInfo(dxfPath).baseName();

        runImport();
    }

    emit dxfPathChanged();
    emitConfigChanged();
}

void DxfImportController::setAccuracy(double accuracy)
{
    if (accuracy == m_accuracy || accuracy < .01 || accuracy > 1000 || !std::isfinite(accuracy))
    {
        return;
    }

    m_accuracy = accuracy;
    renderSegments();
    emit accuracyChanged();
}

void DxfImportController::setMaxDist(double _maxDist)
{
    std::optional<double> maxDist;
    if (_maxDist >= 0.01 && std::isfinite(_maxDist))
        maxDist = _maxDist;

    if (maxDist == m_maxDist)
    {
        return;
    }

    m_maxDist = maxDist;
    renderSegments();
    emit maxDistChanged();
}

void DxfImportController::setUnit(DxfUnit unit)
{
    if (unit == m_unit)
    {
        return;
    }

    m_unit = unit;
    renderSegments();
    emit unitChanged();
}

void DxfImportController::resetPostImport()
{
    m_errorMsg.clear();
    m_seam = {};
    m_paths.clear();
    if (auto lpc = m_laserPointController)
    {
        if (auto figure = lpc->figure())
            figure->clearGraph();
    }

    emit errorMsgChanged();
    emit maxIndexChanged();
}

void DxfImportController::resetResults()
{
    m_data.clear();
    emit updatedImport();
    resetPostImport();
}

void DxfImportController::finalizeImport()
{
    QFileInfo cfgPath = importCfgPath(m_dxfPath);
    try
    {
        std::string fn = cfgPath.filePath().toStdString();
        std::ofstream f(fn);
        f.exceptions(std::ofstream::failbit);

        nlohmann::json json;
        nlohmann::to_json(json, m_config);
        f << json.dump(2) << std::endl;
    }
    catch (...)
    {
        Q_ASSERT(!"this is not catastrophic, but also not really expected");
    }

    Q_ASSERT(m_figureEditor);
    if (m_figureEditor)
    {
        m_figureEditor->newFigure();
        m_figureEditor->setFileType(FileType::Seam);
        m_figureEditor->setSeamFigure(m_seam);

        WobbleFigure* figure = m_figureEditor->figure();
        figure->setName(m_config.name);
        figure->setID(m_config.id);
        figure->setDescription(m_config.description);
    }
}

void DxfImportController::emitConfigChanged()
{
    emit accuracyChanged();
    emit maxDistChanged();
    emit unitChanged();
    emit nameChanged();
    emit figureIdChanged();
    emit descriptionChanged();
}

int DxfImportController::pointCount() const
{
    size_t n = 0;
    for (Segment const& seg : m_segments)
    {
        n += m_paths[seg.index].points.size();
    }

    return n;
}

void DxfImportController::tryCatchError(std::function<void()> f)
{
    auto setError = [&]
    {
        resetPostImport();
        m_errorMsg = exceptionMessage();
        emit errorMsgChanged();
    };

    try
    {
        f();

        if (!m_errorMsg.isEmpty())
        {
            m_errorMsg.clear();
            emit errorMsgChanged();
        }
    }
    catch (nsDxfReader::MissingUnitError&)
    {
        emit missingUnit();
        setError();
    }
    catch (...)
    {
        setError();
    }
}

void DxfImportController::runImport()
{
    resetPostImport();

    tryCatchError([&]
                  { m_data = nsDxfReader::DxfData(m_dxfPath.toStdString()); });

    renderSegments();
    emit updatedImport();
}

QString DxfImportController::fileUnit() const
{
    if (m_data)
    {
        if (auto optUnit = m_data.GetUnit())
        {
            nsDxfReader::Unit unit = *optUnit;

            for (auto [s, u] : nsDxfReader::KnownUnits())
            {
                if (u == unit)
                    return s;
            }
        }
    }

    return "none";
}

DxfImportController::PathConfigProvider::PathConfigProvider(DxfImportController& ctl)
    : ctl(ctl)
    , circleConfigFrnn(ctl.m_accuracy)
{

    for (Segment const& seg : ctl.m_segments)
    {
        if (!seg.isCircle)
            continue;

        CircleInfo ci;
        ci.desc = *seg.circular;
        ci.startAngle = seg.startAngle;
        circleConfigs.push_back(ci);
    }

    for (CircleInfo& ci : circleConfigs)
        circleConfigFrnn.Insert(ci.desc.center.x, ci.desc.center.y, &ci);
}

double DxfImportController::PathConfigProvider::GetCircleStartAngle(size_t /*pathIdx*/, const nsDxfReader::CircularDesc& desc)
{

    CircleInfo* closest = nullptr;
    double bestDistance = ctl.m_accuracy + 1;
    circleConfigFrnn.QueryCandidates(desc.center.x, desc.center.y, [&](CircleInfo* ci)
                                     {
            if (ci->used)
                return;

            double d = Distance(desc.center, ci->desc.center);
            if (d >= bestDistance)
                return;

            if (!equals(desc, ci->desc, ctl.m_accuracy))
                return;

            bestDistance = d;
            closest = ci; });

    if (closest)
    {
        assert(!closest->used);
        closest->used = true;
        return closest->startAngle;
    }

    return 0;
};

std::optional<double> DxfImportController::PathConfigProvider::GetMaxDist(size_t pathIdx)
{
    if (pass == 0)
    {
        return {};
    }

    assert(pathIdx < sourcePathIdx2Segment.size());
    Segment const* seg = sourcePathIdx2Segment[pathIdx];

    if (seg->maxDist)
    {
        if (!*seg->maxDist)
        {
            return {};
        }

        return seg->maxDist;
    }

    return ctl.m_maxDist;
}

double DxfImportController::PathConfigProvider::GetMaxError(size_t pathIdx)
{
    if (pass == 0)
    {
        return ctl.m_accuracy;
    }

    assert(pathIdx < sourcePathIdx2Segment.size());
    Segment const* seg = sourcePathIdx2Segment[pathIdx];

    return seg->accuracy.value_or(ctl.m_accuracy);
}

void DxfImportController::renderSegments()
{
    resetPostImport();

    if (!m_data)
        return;

    tryCatchError([&]
                  {
        std::optional<nsDxfReader::Unit> unit;
        switch (m_unit)
        {
        case FromFile:
            break;
        case Millimeter:
            unit = nsDxfReader::Unit::Millimeters;
            break;
        case Centimeter:
            unit = nsDxfReader::Unit::Centimeters;
            break;
        case Inches:
            unit = nsDxfReader::Unit::Inches;
            break;
        default:
            Q_UNREACHABLE();
        }

        PathConfigProvider cfg(*this);

        /*
        The "segments" are created from paths that come from DxfData::CreatePaths() by joining paths that touch
        at their ends and by reordering to optimize the length of the route that is taken to visit them.

        The triccky part: We have to provide per-path configuration (accuracy for curved elements, maximum distance
        between points) during CreatePahts(). But the user applies this configuration to "segments". So we need to
        know what paths belong to which segment to tell what configuration applies to them!

        The solution: We run the path- and segment-creation two times. In the first pass we use the global configuration
        for all paths and keep track of the path-indices (from CreatePaths) that are used to create the final segments.
        Then we do a second pass where we use this information to provide the per-segment configuration during CreatePaths().

        This only works correctly as long as the segment-configuration does not affect what paths end up in each segment.
        */
        for (cfg.pass = 0; cfg.pass < 2; ++cfg.pass)
        {
            if (cfg.pass == 1)
            {
                for (Segment const& seg : m_segments)
                {
                    for (size_t i : m_paths[seg.index].sourcePathIndices)
                    {
                        assert(i < cfg.sourcePathIdx2Segment.size());
                        cfg.sourcePathIdx2Segment[i] = &seg;
                    }
                }
            }

            { // TODO: optimize order and starting angles (at least when importing for the first time)
                m_paths = m_data.CreatePaths(unit, cfg, 1000);

                if (cfg.pass == 0)
                {
                    assert(cfg.sourcePathIdx2Segment.empty());
                    cfg.sourcePathIdx2Segment.resize(m_paths.size());
                }
                m_paths = nsDxfReader::JoinPaths(m_paths, m_accuracy);

                for (auto& path : m_paths)
                    path.optimizeStart = false;

                m_paths = nsDxfReader::RouteOptimized(m_paths);

                for (int i = 0; nsDxfReader::ImproveOrderLocally(m_paths) && i < 10; ++i)
                    ;
            }

            { // apply segment configuration to imported paths

                // build lookup structure for paths by center (for circular elements) or start (for others)
                Frnn2D<double, size_t> frnnCenter(m_accuracy), frnnStart(m_accuracy);
                for (size_t pathIdx = 0; pathIdx < m_paths.size(); ++pathIdx)
                {
                    auto insert = [&](Frnn2D<double, size_t>& frnn, nsDxfReader::Point2 const& p)
                    {
                        frnn.Insert(p.x, p.y, pathIdx);
                    };

                    auto const& path = m_paths[pathIdx];
                    if (path.circular)
                        insert(frnnCenter, path.circular->desc.center);
                    else
                        insert(frnnStart, path.StartPoint());
                }

                std::vector<bool> pathUsed(m_paths.size()); // per-path flag if it is used for new segment-configuration
                std::vector<std::optional<size_t>> segIdx2Path(m_segments.size());

                for (size_t segIdx = 0; segIdx < m_segments.size(); ++segIdx)
                {
                    Segment const& seg = m_segments[segIdx];
                    std::optional<size_t> closest;
                    if (seg.circular)
                    {
                        nsDxfReader::CircularDesc const& desc = *seg.circular;
                        double bestDistance = m_accuracy + 1;
                        frnnCenter.QueryCandidates(desc.center.x, desc.center.y, [&](size_t pathIdx)
                                                   {
                            if (pathUsed[pathIdx])
                                return;

                            auto const & path = m_paths[pathIdx];
                            assert(path.circular);

                            double d = Distance(desc.center, path.circular->desc.center);
                            if (d >= bestDistance)
                                return;

                            if (!equals(desc, path.circular->desc, m_accuracy))
                                return;

                            bestDistance = d;
                            closest = pathIdx; });
                    }
                    else
                    {
                        double bestDistance = m_accuracy + 1;
                        frnnStart.QueryCandidates(seg.start.x, seg.start.y, [&](size_t pathIdx)
                                                  {
                            if (pathUsed[pathIdx])
                                return;

                            auto const & path = m_paths[pathIdx];
                            assert(!path.circular);

                            double d1 = Distance(seg.start, path.StartPoint());
                            double d2 = Distance(seg.end, path.EndPoint());

                            double d = std::max(d1, d2);
                            if (d >= bestDistance)
                                return;

                            bestDistance = d;
                            closest = pathIdx; });
                    }

                    if (closest)
                    {
                        size_t pathIdx = *closest;
                        assert(!pathUsed[pathIdx]);
                        pathUsed[pathIdx] = true;
                        segIdx2Path[segIdx] = pathIdx;
                    }
                }

                // reuse segment-configurations by index as last resort
                for (size_t segIdx = 0; segIdx < m_segments.size(); ++segIdx)
                {
                    if (segIdx2Path[segIdx])
                        continue;

                    Segment const& seg = m_segments[segIdx];
                    if (seg.index < m_paths.size() && !pathUsed[seg.index])
                    {
                        segIdx2Path[segIdx] = seg.index;
                        pathUsed[seg.index] = true;
                    }
                }

                std::vector<Segment> newSegs;
                for (size_t segIdx = 0; segIdx < m_segments.size(); ++segIdx)
                {
                    std::optional<size_t> pathIdx = segIdx2Path[segIdx];
                    if (!pathIdx)
                        continue;

                    Segment seg = m_segments[segIdx];
                    seg.index = *pathIdx;
                    newSegs.push_back(seg);
                }

                // fill up segment-configurations for unused paths
                for (size_t pathIdx = 0; pathIdx < m_paths.size(); ++pathIdx)
                {
                    if (pathUsed[pathIdx])
                        continue;

                    Segment seg;
                    seg.index = pathIdx;
                    newSegs.push_back(seg);
                }

                // update information of segments from paths
                for (Segment& seg : newSegs)
                {
                    auto const& path = m_paths[seg.index];

                    if (path.circular)
                        seg.circular = path.circular->desc;

                    if (path.isCircle)
                        seg.startAngle = path.circular->startAngle;

                    seg.start = path.StartPoint();
                    seg.end = path.EndPoint();
                    seg.isCircle = path.isCircle;
                    seg.curved = path.curved;
                }

                m_segments = newSegs;

                for (Segment& seg : m_segments)
                {
                    if (seg.reverse)
                    {
                        auto& points = m_paths[seg.index].points;
                        std::reverse(points.begin(), points.end());
                    }
                }
            }
        }

        if (m_laserPointController)
        {
            m_seam.name = m_name.toStdString();
            m_seam.ID = std::to_string(m_id);
            m_seam.description = m_description.toStdString();

            std::optional<nsDxfReader::Point2> minPoint, maxPoint;
            for (nsDxfReader::Path const& path : m_paths)
            {
                for (auto const& point : path.points)
                {
                    if (!minPoint)
                        minPoint = point;
                    else
                    {
                        minPoint->x = std::min(point.x, minPoint->x);
                        minPoint->y = std::min(point.y, minPoint->y);
                    }

                    if (!maxPoint)
                        maxPoint = point;
                    else
                    {
                        maxPoint->x = std::max(point.x, maxPoint->x);
                        maxPoint->y = std::max(point.y, maxPoint->y);
                    }
                }
            }

            // Shift the figure center to the origin
            if (minPoint && maxPoint)
            {
                auto offset = nsDxfReader::Point2() - Midpoint(*minPoint, *maxPoint);
                *minPoint += offset;
                *maxPoint += offset;
                for (nsDxfReader::Path& path : m_paths)
                {
                    for (nsDxfReader::Point2& point : path.points)
                    {
                        point += offset;
                    }
                }
            }

            std::vector<int> cycleEnds;
            for (auto const& seg : m_segments)
            {
                auto addPoint = [&](nsDxfReader::Point2 const& point, bool isLast)
                {
                    RTC6::seamFigure::command::Order order;
                    order.endPosition = std::make_pair(point.x, point.y);
                    order.power = isLast ? 0 : -1;
                    order.ringPower = isLast ? 0 : -1;
                    order.velocity = -1;
                    m_seam.figure.push_back(order);
                };

                nsDxfReader::Path const& path = m_paths[seg.index];
                if (path.cyclic)
                    cycleEnds.push_back(m_seam.figure.size());

                for (auto const& point : path.points)
                    addPoint(point, !path.cyclic && &point == &path.points.back());

                if (path.cyclic)
                {
                    cycleEnds.push_back(m_seam.figure.size());
                    addPoint(path.EndPoint(), true);
                }
            }

            if (minPoint && maxPoint)
            {
                auto size = *maxPoint - *minPoint;
                auto center = *minPoint + size * .5;
                m_size = std::max(size.x, size.y);
                m_center = QPointF(center.x, center.y);

                emit centerChanged();
                emit sizeChanged();
            }

            auto lpc = m_laserPointController;

            lpc->drawSeamFigure(m_seam);
            //lpc->setPointsAreModifiable(false);

            for (auto node : lpc->figure()->get_nodes())
            {
                node->setIsProtected(true);
            }

            for (int i : cycleEnds)
            {
                auto node = lpc->figure()->get_nodes().at(i);
                LaserPoint* lp = dynamic_cast<LaserPoint*>(node);
                lp->setColor(QColor(255, 0, 0));
            }

            QObject::connect(lpc->figure(), &precitec::scantracker::components::wobbleFigureEditor::WobbleFigure::nodeClicked,
                             this, &DxfImportController::onNodeClicked,
                             Qt::UniqueConnection);
        } });

    emit maxIndexChanged();

    if (m_modelLayoutSize != m_segments.size())
    {
        m_modelLayoutSize = m_segments.size();
        emit layoutChanged();
    }

    emit dataChanged(index(0), index(m_segments.empty() ? 0 : m_segments.size() - 1));
    emit updatedFigure();
    emit pointCountChanged();
}

void DxfImportController::onNodeClicked(qan::Node* node, QPointF /*pos*/)
{
    auto lp = dynamic_cast<LaserPoint*>(node);
    size_t id = lp->ID();

    size_t idBegin = 0;
    for (size_t segIdx = 0; segIdx < m_segments.size(); ++segIdx)
    {
        nsDxfReader::Path const& path = m_paths[m_segments[segIdx].index];
        size_t idEnd = idBegin + path.points.size() + path.cyclic;
        if (id >= idBegin && id < idEnd)
        {
            select(segIdx);
            return;
        }

        idBegin = idEnd;
    }

    emit segmentSelected(-1);
}

void DxfImportController::swapSegments(int oldIndex, int newIndex)
{
    if (oldIndex < 0 || static_cast<size_t>(oldIndex) >= m_segments.size() ||
        newIndex < 0 || static_cast<size_t>(newIndex) >= m_segments.size())
    {
        Q_ASSERT(false);
        return;
    }

    std::swap(m_segments[oldIndex], m_segments[newIndex]);
    for (auto i : {index(oldIndex), index(newIndex)})
        emit dataChanged(i, i);

    renderSegments();
}

void DxfImportController::moveSegment(int oldIndex, int newIndex) // TODO: This is one-based because it was initially ment for QML and the UI uses one-based indices... now this no longer true and should get zero-based
{
    if (oldIndex < 0 || static_cast<size_t>(oldIndex) >= m_segments.size() ||
        newIndex < 0 || static_cast<size_t>(newIndex) >= m_segments.size())
    {
        Q_ASSERT(false);
        return;
    }

    int dir = oldIndex < newIndex ? 1 : -1;
    for (int i = oldIndex; i != newIndex; i += dir)
        std::swap(m_segments[i], m_segments[i + dir]);

    emit dataChanged(index(oldIndex), index(newIndex));

    renderSegments();
}

void DxfImportController::select(int segIdx)
{
    WobbleFigure* figure = m_laserPointController->figure();
    figure->clearSelection();

    if (segIdx < 0 || static_cast<size_t>(segIdx) >= m_segments.size())
    {
        Q_ASSERT(false);
        return;
    }

    int idBegin = 0;
    for (int i = 0; i < segIdx; ++i)
    {
        nsDxfReader::Path const& path = m_paths[m_segments[i].index];
        idBegin += path.points.size() + path.cyclic;
    }

    int idEnd = idBegin + m_paths[m_segments[segIdx].index].points.size();

    for (auto node : figure->get_nodes())
    {
        auto lp = dynamic_cast<LaserPoint*>(node);
        if (lp->ID() >= idBegin && lp->ID() < idEnd)
            figure->setNodeSelected(lp, true);
    }

    emit segmentSelected(segIdx);
}

void DxfImportController::restartWithDefaults()
{
    QFileInfo cfgPath = importCfgPath(m_dxfPath);

    m_config = Config();
    m_config.name = QFileInfo(m_dxfPath).baseName();

    runImport();
    emitConfigChanged();
}

enum Role
{
    Index = Qt::UserRole, // NOTE: This refers to the index of the segment inside the segment-vector (so it is derived from the order of the segments), the index of the path that belongs to this segment is a different concept and is stored in Segment::index
    Circle,
    Reverse,
    StartAngle,
    Curved,
    Accuracy,
    MaxDist,
    PointCount,
};

QHash<int, QByteArray> DxfImportController::roleNames() const
{
    return {
        {Index, QByteArrayLiteral("index")},
        {Circle, QByteArrayLiteral("circle")},
        {Reverse, QByteArrayLiteral("reverse")},
        {StartAngle, QByteArrayLiteral("startAngle")},
        {Curved, QByteArrayLiteral("curved")},
        {Accuracy, QByteArrayLiteral("accuracy")},
        {MaxDist, QByteArrayLiteral("maxDist")},
        {PointCount, QByteArrayLiteral("pointCount")},
    };
}

QVariant DxfImportController::data(const QModelIndex& index, int role) const
{
    if (!index.isValid())
    {
        return {};
    }

    const Segment& s = m_segments.at(index.row());

    switch (role)
    {
    case Index:
        return index.row();
    case Circle:
        return s.isCircle;
    case Reverse:
        return s.reverse;
    case StartAngle:
        return s.startAngle;
    case Curved:
        return s.curved;
    case Accuracy:
        return s.accuracy.value_or(-1);
    case MaxDist:
        return s.maxDist.value_or(-1);
    case PointCount:
        return static_cast<int>(m_paths[s.index].points.size());
    }

    Q_UNREACHABLE();
    return {};
}

int DxfImportController::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid())
    {
        return 0;
    }

    return m_segments.size();
}

Qt::ItemFlags DxfImportController::flags(const QModelIndex& /*index*/) const
{
    return Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsEditable;
}

bool DxfImportController::setData(const QModelIndex& index, const QVariant& value, int role)
{
    if (!index.isValid())
    {
        return {};
    }

    Segment& s = m_segments.at(index.row());

    switch (role)
    {
    case Index:
    {
        int val = value.toInt();
        if (val < 0 || static_cast<size_t>(val) >= m_segments.size())
            return false;
        swapSegments(index.row(), val);
        return true;
    }
    case Circle:
        return false;
    case Reverse:
        s.reverse = value.toBool();
        renderSegments();
        emit dataChanged(index, index, {role});
        return true;
    case StartAngle:
        assert(s.isCircle);
        s.startAngle = value.toDouble();
        renderSegments();
        emit dataChanged(index, index, {role});
        return true;
    case Accuracy:
    {
        assert(s.curved);
        double val = value.toDouble();
        if (val <= 0 || !std::isfinite(val))
        {
            s.accuracy.reset();
        }
        else
        {
            s.accuracy = val;
        }
        renderSegments();
        emit dataChanged(index, index, {role});
        return true;
    }
    case MaxDist:
    {
        double val = value.toDouble();
        if (val < 0 || !std::isfinite(val))
        {
            s.maxDist.reset();
        }
        else
        {
            s.maxDist = val;
        }
        renderSegments();
        emit dataChanged(index, index, {role});
        return true;
    }
    }

    Q_UNREACHABLE();
    return false;
}

}
}
}
}
