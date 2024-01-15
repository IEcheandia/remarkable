#include "gridController.h"
#include "figureEditorSettings.h"

#include <math.h>

const QSizeF scanFieldScantracker = {20.0, 20.0};           //mm x mm

namespace precitec
{

namespace scanmaster
{

namespace components
{

namespace wobbleFigureEditor
{

GridController::~GridController()
{
    for ( auto line : m_minorLines )
    {
        if ( line != nullptr && QQmlEngine::objectOwnership(line) == QQmlEngine::CppOwnership )
        {
            line->deleteLater();
        }
    }
    m_minorLines.clear();
    for ( auto line : m_majorLines )
    {
        if ( line != nullptr && QQmlEngine::objectOwnership(line) == QQmlEngine::CppOwnership )
        {
            line->deleteLater();
        }
    }
    m_majorLines.clear();
}

void GridController::setShowCoordinateSystem(bool showCS)
{
    if (m_showCoordinateSystem == showCS)
    {
        return;
    }

    m_showCoordinateSystem = showCS;
    emit showCoordinateSystemChanged();
}

void GridController::setShowScanField(bool set)
{
    if (m_showScanField == set)
    {
        return;
    }
    m_showScanField = set;
    emit showScanFieldChanged();
}

void GridController::setScale(int newScale)
{
    if (m_scale == newScale)
    {
        return;
    }
    m_scale = newScale;
    emit scaleChanged();
}

QQmlListProperty<qan::impl::GridLine> GridController::minorLines()
{
    return QQmlListProperty<qan::impl::GridLine>(this, this, &GridController::callMinorLinesCount, &GridController::callMinorLinesAt);
}

QQmlListProperty<qan::impl::GridLine> GridController::majorLines()
{
    return QQmlListProperty<qan::impl::GridLine>(this, this, &GridController::callMajorLinesCount, &GridController::callMajorLinesAt);
}

bool GridController::updateGrid(const QRectF& viewRect, const QQuickItem& container, const QQuickItem& navigable) noexcept
{
    qreal containerZoom = container.scale();
    if (qFuzzyCompare(1.0 + containerZoom, 1.0)) // Protect against 0 zoom
    {
        containerZoom = 1.0;
    }

    if (viewRect.width() != m_width)
    {
        m_width = viewRect.width() * containerZoom;
        emit widthChanged();
    }

    if (viewRect.height() != m_height)
    {
        m_height = viewRect.height() * containerZoom;
        emit heightChanged();
    }

    if (!OrthoGrid::updateGrid(viewRect, container, navigable))
    {
        return false;
    }

    const qreal gridScale{getGridScale()};

    qreal const adaptiveScale = std::pow(2.0, std::ceil(std::log2(gridScale / containerZoom)));

    int const topLeftX = std::floor(viewRect.topLeft().x() / adaptiveScale);
    int const topLeftY = std::floor(viewRect.topLeft().y() / adaptiveScale);

    int const tickInterval = 2; // a tick label will be created every nth line, this is n

    QPointF rectifiedTopLeft{topLeftX * adaptiveScale,
                             topLeftY * adaptiveScale};
    QPointF rectifiedBottomRight{(std::ceil(viewRect.bottomRight().x() / adaptiveScale) * adaptiveScale),
                                 (std::ceil(viewRect.bottomRight().y() / adaptiveScale) * adaptiveScale)};
    QRectF rectified{rectifiedTopLeft, rectifiedBottomRight};

    const int numLinesX = static_cast<int>(round(rectified.width() / adaptiveScale));
    const int numLinesY = static_cast<int>(round(rectified.height() / adaptiveScale));

    // Update points cache size
    const int linesCount = numLinesX + numLinesY;
    if ( m_minorLines.size() < linesCount )
    {
        m_minorLines.resize(linesCount);
    }

    m_minorTickInfos.clear();

    const auto navigableRectified = container.mapRectToItem(&navigable, rectified);
    int minorL = 0;
    int majorL = 0;
    for (int nly = 0; nly < numLinesY; ++nly)
    { // Generate HORIZONTAL lines
        auto py = rectifiedTopLeft.y() + (nly * adaptiveScale);
        auto navigablePoint = container.mapToItem(&navigable, QPointF{0.0, py});

        if ((nly + topLeftY) % tickInterval == 0)
        {
            m_minorTickInfos.push_back(TickInfo{-py / m_figureScale, navigablePoint.y(), false});
        }

        const QPointF p1{navigableRectified.left(), navigablePoint.y()};
        const QPointF p2{navigableRectified.right(), navigablePoint.y()};

        if (m_minorLines[minorL] == nullptr)
        {
            qan::impl::GridLine* line = new qan::impl::GridLine{std::move(p1), std::move(p2)};
            m_minorLines[minorL] = line;
        }
        else
        {
            m_minorLines[minorL]->getP1() = p1;
            m_minorLines[minorL]->getP2() = p2;
        }
        ++minorL;
    }

    for (int nlx = 0; nlx < numLinesX; ++nlx)
    { // Generate VERTICAL lines
        auto lx = rectifiedTopLeft.x() + (nlx * adaptiveScale);
        auto navigablePoint = container.mapToItem(&navigable, QPointF{lx, 0.0});
        if ((nlx + topLeftX) % tickInterval == 0)
        {
            m_minorTickInfos.push_back(TickInfo{lx / m_figureScale, navigablePoint.x(), true});
        }

        const QPointF p1{navigablePoint.x(), navigableRectified.top()};
        const QPointF p2{navigablePoint.x(), navigableRectified.bottom()};

        if (m_minorLines[minorL] != nullptr )
        {
            m_minorLines[minorL]->getP1() = p1;
            m_minorLines[minorL]->getP2() = p2;
        }
        else
        {
            qan::impl::GridLine* line = new qan::impl::GridLine{std::move(p1), std::move(p2)};
            m_minorLines[minorL] = line;
        }
        ++minorL;
    }

    // Note: Lines when l < _lines.size() are left ignored, they won't be drawn since we
    // emit the number of lines to draw in redrawLines signal.
    updateOrigin(container);
    updateScanField(container);
    emit redrawLines(minorL, majorL);
    return true;
}

int GridController::minorTickCount() const
{
    return m_minorTickInfos.size();
}

double GridController::minorTickPos(int i) const
{
    return m_minorTickInfos.at(i).pos;
}

double GridController::minorTickValue(int i) const
{
    return m_minorTickInfos.at(i).val;
}

bool GridController::minorTickIsVertical(int i) const
{
    return m_minorTickInfos.at(i).vertical;
}

bool GridController::updateOrigin(const QQuickItem& container) noexcept
{
    m_origin.setX(0 + container.x());
    m_origin.setY(0 + container.y());
    m_origin.setWidth(15 * container.scale());
    m_origin.setHeight(100 * container.scale());

    emit originChanged();

    return true;
}

void GridController::updateScanField(const QQuickItem& container)
{
    const auto size = scanFieldSize();
    m_scanField = QRectF{container.x() - size.width() * container.scale() * 0.5, container.y() - size.height() * container.scale() * 0.5, size.width() * container.scale(), size.height() * container.scale()};
    emit scanFieldChanged();
}

QSizeF GridController::scanFieldSize() const
{
    if (FigureEditorSettings::instance()->scanMasterMode())
    {
        return scanFieldSizeScanMaster();
    }

    return scanFieldSizeScantracker();
}

QSizeF GridController::scanFieldSizeScanMaster() const
{
    switch (FigureEditorSettings::instance()->lensType())
    {
    case precitec::interface::LensType::F_Theta_340:
        // 220mm x 104mm
        return QSizeF{220, 104} * m_scale;
    case precitec::interface::LensType::F_Theta_460:
        // 380mm x 290mm
        return QSizeF{380, 290} * m_scale;
    case precitec::interface::LensType::F_Theta_255:
        // 170mm x 100mm
        return QSizeF{170, 100} * m_scale;
    default:
        __builtin_unreachable();
    }
}

QSizeF GridController::scanFieldSizeScantracker() const
{
    return scanFieldScantracker * m_scale;
}

int GridController::callMinorLinesCount(QQmlListProperty<qan::impl::GridLine>* list)
{
    return reinterpret_cast<GridController*>(list->data)->minorLinesCount();
}
int GridController::callMajorLinesCount(QQmlListProperty<qan::impl::GridLine>* list)
{
    return reinterpret_cast<GridController*>(list->data)->majorLinesCount();
}
qan::impl::GridLine * GridController::callMajorLinesAt(QQmlListProperty<qan::impl::GridLine>* list, int index)
{
    return reinterpret_cast<GridController*>(list->data)->majorLinesAt(index);
}
qan::impl::GridLine * GridController::callMinorLinesAt(QQmlListProperty<qan::impl::GridLine>* list, int index)
{
    return reinterpret_cast<GridController*>(list->data)->minorLinesAt(index);
}

}
}
}
}
