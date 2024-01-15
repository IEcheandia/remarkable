#pragma once

#include <QuickQanava.h>

namespace precitec
{

namespace scanmaster
{

namespace components
{

namespace wobbleFigureEditor
{

/**
 * Class which is used to draw the grid, the origin and the scanfield in the background.
 **/
class GridController : public qan::OrthoGrid
{
    Q_OBJECT
    /**
     * Whether to show the coordinate system
     **/
    Q_PROPERTY(bool showCoordinateSystem READ showCoordinateSystem WRITE setShowCoordinateSystem NOTIFY showCoordinateSystemChanged)
    /**
     * Whether to show the scan field
     **/
    Q_PROPERTY(bool showScanField READ showScanField WRITE setShowScanField NOTIFY showScanFieldChanged)
    /**
     * The origin of the scanfield
     **/
    Q_PROPERTY(QRect origin READ origin NOTIFY originChanged)
    /**
     * The bounding rectangle of the scanfield
     **/
    Q_PROPERTY(QRectF scanField READ scanField NOTIFY scanFieldChanged)
    /**
     * Factor to calculate pixels (figure editor) from mm and mm from pixels.
     **/
    Q_PROPERTY(int scale READ scale WRITE setScale NOTIFY scaleChanged)
    /**
     * Minor lines (small lines) of the background grid
     **/
    Q_PROPERTY(QQmlListProperty<qan::impl::GridLine> minorLines READ minorLines CONSTANT)
    /**
     * Major lines (big lines) of the background grid
     **/
    Q_PROPERTY(QQmlListProperty<qan::impl::GridLine> majorLines READ majorLines CONSTANT)

    Q_PROPERTY(double figureScale MEMBER m_figureScale NOTIFY figureScaleChanged)

    double m_figureScale = 1.0;

    Q_PROPERTY(double width READ width NOTIFY widthChanged)
    Q_PROPERTY(double height READ height NOTIFY widthChanged)

    double m_width = 0;
    double m_height = 0;

public:
    explicit GridController( QQuickItem* parent = nullptr) : qan::OrthoGrid(parent) { }
    ~GridController() override;

    double width() const { return m_width; }
    double height() const { return m_height; }

    Q_INVOKABLE int minorTickCount() const;
    Q_INVOKABLE double minorTickPos(int i) const;
    Q_INVOKABLE double minorTickValue(int i) const;
    Q_INVOKABLE bool minorTickIsVertical(int i) const;

    bool showCoordinateSystem() const
    {
        return m_showCoordinateSystem;
    }
    void setShowCoordinateSystem(bool showCS);

    bool showScanField() const
    {
        return m_showScanField;
    }
    void setShowScanField(bool set);

    QRect origin() const
    {
        return m_origin;
    }

    QRectF scanField() const
    {
        return m_scanField;
    }

    int scale() const
    {
        return m_scale;
    }
    void setScale(int newScale);

    QQmlListProperty<qan::impl::GridLine> minorLines();
    QQmlListProperty<qan::impl::GridLine> majorLines();

    virtual bool updateGrid(const QRectF& viewRect, const QQuickItem& container, const QQuickItem& navigable) noexcept override;
    bool updateOrigin(const QQuickItem &container) noexcept;

Q_SIGNALS:
    void redrawLines(int minorLineToDrawCount, int majorLineToDrawCount);

    void showCoordinateSystemChanged();
    void showScanFieldChanged();
    void scanFieldChanged();
    void originChanged();
    void figureScaleChanged();
    void widthChanged();
    void heightChanged();

private:
    void updateScanField(const QQuickItem &container);
    QSizeF scanFieldSize() const;
    QSizeF scanFieldSizeScanMaster() const;
    QSizeF scanFieldSizeScantracker() const;
    bool m_showCoordinateSystem = true;
    bool m_showScanField = false;
    QRect m_origin;
    QRectF m_scanField;
    int m_scale = 1000;

    int minorLinesCount() const
    {
        return m_minorLines.size();
    }
    qan::impl::GridLine* minorLinesAt(int index) const
    {
        if (index >= m_minorLines.size())
        {
            return {};
        }
        return m_minorLines.at(index);
    }
    QVector<qan::impl::GridLine*> m_minorLines;

    struct TickInfo
    {
        double val;
        double pos;
        bool vertical;
    };

    std::vector<TickInfo> m_minorTickInfos;

    int majorLinesCount() const
    {
        return m_majorLines.size();
    }
    qan::impl::GridLine* majorLinesAt(int index) const
    {
        if (index >= m_majorLines.size())
        {
            return {};
        }
        return m_majorLines.at(index);
    }
    QVector<qan::impl::GridLine*> m_majorLines;

    static int callMinorLinesCount(QQmlListProperty<qan::impl::GridLine>* list);
    static qan::impl::GridLine* callMinorLinesAt(QQmlListProperty<qan::impl::GridLine>* list, int index);

    static int callMajorLinesCount(QQmlListProperty<qan::impl::GridLine>* list);
    static qan::impl::GridLine* callMajorLinesAt(QQmlListProperty<qan::impl::GridLine>* list, int index);
};

}
}
}
}
QML_DECLARE_TYPE(precitec::scanmaster::components::wobbleFigureEditor::GridController)
