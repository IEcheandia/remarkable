#pragma once

#include <QQuickItem>
#include <QAbstractItemModel>
#include <QMatrix4x4>

#include <memory>

#include "image/image.h"
#include "common/geoContext.h"
#include "overlay/overlayCanvas.h"

class TestImageItem;
class QImage;

namespace precitec
{

namespace gui
{
namespace components
{
namespace image
{


class OverlayGroupModel;
class OverlayGroupFilterModel;
class NodeBasedOverlayCanvas;

using ImageData = std::tuple<precitec::interface::ImageContext, precitec::image::BImage, precitec::image::OverlayCanvas>;

/**
 * @brief QQuickItem to render a @link{precitec::image::BImage} together with the @link{precitec::image::OverlayCanvas}.
 *
 * The image and overlay needs to be provided through the imageData property.
 * The image gets centered inside the geometry of the item.
 **/
class ImageItem : public QQuickItem
{
    Q_OBJECT
    /**
     * Read-only model to the overlay groups.
     * The exposed model provides the following roles:
     * @li display (Title of the Overlay Group)
     * @li available (Whether the Overlay Group is available for the current @link{imageData}
     * @li enabled (Whether the Overlay Group should be rendered
     *
     * The exposed model filters on elements which have the available rule set to @c true.
     *
     * The model provides an invokable method @c swapEnabled(int) which allows to negate the
     * current value of the enabled role. An example could be:
     * @code
     * ImageItem {
     *     id: image
     *     ListView {
     *         anchors.fill: parent
     *         model: image.overlayGroupModel
     *         delegate: CheckBox {
     *             text: display
     *             checked: enabled
     *             onClicked: {
     *                 image.overlayGroupModel.swapEnabled(index);
     *             }
     *          }
     *     }
     * }
     * @endcode
     **/
    Q_PROPERTY(QAbstractItemModel *overlayGroupModel READ overlayGroupModel CONSTANT)
    Q_PROPERTY(bool lineOverlayEnabled READ isLineOverlayEnabled WRITE setLineOverlayEnabled NOTIFY overlayEnabledChanged)
    Q_PROPERTY(bool contourOverlayEnabled READ isContourOverlayEnabled WRITE setContourOverlayEnabled NOTIFY overlayEnabledChanged)
    Q_PROPERTY(bool positionOverlayEnabled READ isPositionOverlayEnabled WRITE setPositionOverlayEnabled NOTIFY overlayEnabledChanged)
    Q_PROPERTY(bool textOverlayEnabled READ isTextOverlayEnabled WRITE setTextOverlayEnabled NOTIFY overlayEnabledChanged)
    Q_PROPERTY(bool gridOverlayEnabled READ isGridOverlayEnabled WRITE setGridOverlayEnabled NOTIFY overlayEnabledChanged)
    Q_PROPERTY(bool imageOverlayEnabled READ isImageOverlayEnabled WRITE setImageOverlayEnabled NOTIFY overlayEnabledChanged)
    /**
     * The image data for this item consisting of the image and overlay canvas.
     * This property must be set to the image which should be rendered. Without the component is rather useless.
     **/
    Q_PROPERTY(precitec::gui::components::image::ImageData imageData READ imageData WRITE setImageData NOTIFY imageDataChanged)
    /**
     * The current zoom level used to render the image and overlay.
     * It is bound to @link{minZoom} and @link{maxZoom} properties.
     **/
    Q_PROPERTY(qreal zoom READ zoom WRITE setZoom NOTIFY zoomChanged)
    /**
     * Minimum zoom level supported on this imageItem. This is a read-only constant property.
     **/
    Q_PROPERTY(qreal minZoom READ minZoom CONSTANT)
    /**
     * Maximum zoom level supported on this imageItem. This is a read-only constant property.
     **/
    Q_PROPERTY(qreal maxZoom READ maxZoom CONSTANT)
    /**
     * The size of the @link{precitec::image::BImage} which is currently rendered.
     **/
    Q_PROPERTY(QSizeF imageSize READ imageSize NOTIFY imageSizeChanged)
    /**
     * Panning of the complete zoomed image.
     **/
    Q_PROPERTY(QPointF panning READ panning WRITE setPanning NOTIFY panningChanged)

    /**
     * The actual painted geometry.
     **/
    Q_PROPERTY(QRectF paintedRect READ paintedRect NOTIFY paintedRectChanged)
    
    /**
     * The hardware ROI of the received image
     * */
    Q_PROPERTY(QPoint hwROI READ hwROI NOTIFY hwROIChanged)

    /**
     * Whether the ImageItem is showing a valid image.
     **/
    Q_PROPERTY(bool valid READ isValid NOTIFY imageSizeChanged)
    
    
    Q_PROPERTY(bool hasScannerPosition READ hasScannerPosition NOTIFY hasScannerPositionChanged)
    Q_PROPERTY(QPointF scannerPosition READ scannerPosition NOTIFY scannerPositionChanged)

    Q_PROPERTY(bool mirrorX READ mirrorX WRITE setMirrorX NOTIFY mirrorXChanged)
    Q_PROPERTY(bool mirrorY READ mirrorY WRITE setMirrorY NOTIFY mirrorYChanged)
    
public:
    explicit ImageItem(QQuickItem *parent = nullptr);
    ~ImageItem() override;

    QAbstractItemModel *overlayGroupModel() const;

    ImageData imageData() const
    {
        return m_image;
    }

    void setImageData(const ImageData &data);

    qreal zoom() const
    {
        return m_zoom;
    }

    void setZoom(qreal value);

    QSizeF imageSize() const
    {
        return m_imageSize;
    }

    bool isValid() const
    {
        return !m_imageSize.isNull();
    }

    QPointF panning() const
    {
        return m_panning;
    }
    void setPanning(const QPointF &panning);

    QRectF paintedRect() const;

    QPoint hwROI() const
    {
        return m_hwROI;
    }
    
    bool hasScannerPosition() const
    {
        return m_ScannerInfo.m_hasPosition;
    }
    QPointF scannerPosition() const
    {
        return {m_ScannerInfo.m_x, m_ScannerInfo.m_y};
    }

    /**
     * Zooms the image so that it fits into the available width/height
     **/
    Q_INVOKABLE void zoomToFit();

    Q_INVOKABLE QPointF mapToPaintedImage(const QPointF &pos) const;
    Q_INVOKABLE QPointF mapFromPaintedImage(const QPointF &pos) const;

    Q_INVOKABLE void save();
    Q_INVOKABLE void saveWithOverlays();
    Q_INVOKABLE void save(const QString& filePath);
    /**
     * Saves a sub image (template) of the @p selection to the given @p filePath.
     **/
    Q_INVOKABLE void saveSubImage(const QRectF& selection, const QString& filePath);

    static constexpr qreal minZoom()
    {
        return 0.1;
    }

    static constexpr qreal maxZoom()
    {
        return 5.0;
    }

    bool isLineOverlayEnabled() const;
    bool isContourOverlayEnabled() const;
    bool isPositionOverlayEnabled() const;
    bool isTextOverlayEnabled() const;
    bool isGridOverlayEnabled() const;
    bool isImageOverlayEnabled() const;

    void setLineOverlayEnabled(bool enabled);
    void setContourOverlayEnabled(bool enabled);
    void setPositionOverlayEnabled(bool enabled);
    void setTextOverlayEnabled(bool enabled);
    void setGridOverlayEnabled(bool enabled);
    void setImageOverlayEnabled(bool enabled);

    bool mirrorX() const
    {
        return m_mirrorX;
    }
    void setMirrorX(bool enabled);

    bool mirrorY() const
    {
        return m_mirrorY;
    }
    void setMirrorY(bool enabled);

Q_SIGNALS:
    void imageDataChanged();
    void zoomChanged();
    void panningChanged();
    void paintedRectChanged();
    void imageSizeChanged();
    void hwROIChanged();
    void rendered();
    void overlayEnabledChanged();
    void hasScannerPositionChanged();
    void scannerPositionChanged();
    void mirrorXChanged();
    void mirrorYChanged();

protected:
    QSGNode *updatePaintNode(QSGNode  *oldNode , UpdatePaintNodeData *updatePaintNodeData) override;

private:
    void updateAvailableOverlays();
    void updateTransformation();
    QImage prepareImage();

    std::unique_ptr<NodeBasedOverlayCanvas> m_overlayCanvas;
    OverlayGroupModel *m_overlayGroupModel;
    OverlayGroupFilterModel *m_overlayGroupFilterModel;
    ImageData m_image;
    qreal m_zoom = 1.0;
    QMatrix4x4 m_transformation;
    QSizeF m_imageSize;
    QPointF m_panning;
    QPoint m_hwROI;
    interface::ScannerContextInfo m_ScannerInfo;
    
    bool m_emitImageRendered = false;
    bool m_transformationDirty = false;
    bool m_zoomDirty = false;
    bool m_imageSizeDirty = false;
    bool m_imageDirty = false;
    bool m_overlayDirty = false;
    friend TestImageItem;
    bool m_mirrorX{false};
    bool m_mirrorY{false};
};

}
}
}
}

Q_DECLARE_METATYPE(precitec::gui::components::image::ImageData)
