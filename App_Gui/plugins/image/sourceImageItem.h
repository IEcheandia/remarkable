#pragma once

#include <QQuickItem>
#include <QAbstractItemModel>
#include <QMatrix4x4>
#include <QImage>
#include <QFileInfo>

#include <memory>
#include <mutex>

class TestSourceImageItem;

namespace precitec
{
namespace gui
{
namespace components
{
namespace image
{

class SourceImageItem : public QQuickItem
{
  Q_OBJECT

    Q_PROPERTY(QString source READ source WRITE setSource NOTIFY sourceChanged)

    Q_PROPERTY(qreal zoom READ zoom WRITE setZoom NOTIFY zoomChanged)

    /**
     * Minimum zoom level supported on this imageItem. This is a read-only constant property.
     **/
    Q_PROPERTY(qreal minZoom READ minZoom CONSTANT)
    /**
     * Maximum zoom level supported on this imageItem. This is a read-only constant property.
     **/
    Q_PROPERTY(qreal maxZoom READ maxZoom CONSTANT)

    Q_PROPERTY(QPointF panning READ panning WRITE setPanning NOTIFY panningChanged)

    Q_PROPERTY(QRectF paintedRect READ paintedRect NOTIFY transformationChanged)

    Q_PROPERTY(QSize imageSize READ imageSize NOTIFY imageChanged)

    Q_PROPERTY(QMatrix4x4 transformation READ transformation NOTIFY transformationChanged)

    Q_PROPERTY(bool imageValid READ imageValid NOTIFY imageChanged)

    Q_PROPERTY(bool loading READ isLoading NOTIFY loadingChanged)

    Q_PROPERTY(bool pauseUpdate READ pauseUpdate WRITE setPauseUpdate NOTIFY pauseUpdateChanged)

    /**
     * If the thumbnail size is valid a thumbnail is loaded instead of the actual image.
     * In case the thumbnail does not yet exist, the thumbnail will be generated. The thumbnail is
     * generated with KeepAspectRatio, thus will not exactly fit the specified size.
     *
     * The file source is split at the extension and extended by _{thumbnail.width}x{thumbnail.height}.
     *
     * If a thumbnail is used the @link{imageSize} is set to @link{thumbnailOriginalSize}
     **/
    Q_PROPERTY(QSize thumbnail READ thumbnail WRITE setThumbnail NOTIFY thumbnailChanged)

    /**
     * The original size of the image for which a thumbnail is used. Must be specified when using a thumbnail.
     **/
    Q_PROPERTY(QSize thumbnailOriginalSize READ thumbnailOriginalSize WRITE setThumbnailOriginalSize NOTIFY thumbnailOriginalSizeChanged)

public:
    explicit SourceImageItem(QQuickItem *parent = nullptr);
    ~SourceImageItem() override;

    QString source() const
    {
        return m_source;
    }
    void setSource(const QString &file);

    qreal zoom() const
    {
        return m_zoom;
    }
    void setZoom(qreal value);

    QPointF panning() const
    {
        return m_panning;
    }
    void setPanning(const QPointF &panning);

    QSize imageSize() const;

    QRectF paintedRect() const;

    QMatrix4x4 transformation() const
    {
        return m_transformation;
    }

    bool imageValid() const;

    bool isLoading() const
    {
        return m_loading;
    }

    static constexpr qreal minZoom()
    {
        return 0.1;
    }

    static constexpr qreal maxZoom()
    {
        return 5.0;
    }

    bool pauseUpdate() const
    {
        return m_pauseUpdate;
    }
    void setPauseUpdate(bool pause);

    QSize thumbnail() const
    {
        return m_thumbnail;
    }
    void setThumbnail(const QSize& size);

    QSize thumbnailOriginalSize() const
    {
        return m_thumbnailOriginalSize;
    }
    void setThumbnailOriginalSize(const QSize& size);

    Q_INVOKABLE void zoomToFit();

    Q_INVOKABLE QPointF mapToPaintedImage(const QPointF &pos) const;

    Q_INVOKABLE QPointF mapFromPaintedImage(const QPointF &pos) const;

    Q_INVOKABLE void centerAndFitTo(const QPointF &pos, const QSize &sz);

    Q_INVOKABLE void updateImage();

Q_SIGNALS:
    void sourceChanged();
    void imageChanged();
    void zoomChanged();
    void panningChanged();
    void cameraRoiChanged();
    void transformationChanged();
    void imageValidChanged();
    void loadingChanged();
    void pauseUpdateChanged();
    void thumbnailChanged();
    void thumbnailOriginalSizeChanged();

protected:
    QSGNode *updatePaintNode(QSGNode  *oldNode , UpdatePaintNodeData *updatePaintNodeData) override;

private:
    void updateTransformation();
    void setLoading(bool set);

    QImage imageOrThumbnail();
    QImage createThumbnail(const QFileInfo& thumbnail);
    QFileInfo thumbnailFilePath();

    bool m_pauseUpdate = false;
    QString m_source = QString{};
    qreal m_zoom = 1.0;
    QPointF m_panning = QPointF{0,0};
    QImage m_image = QImage{};
    QSize m_imageSize;
    QMatrix4x4 m_transformation = QMatrix4x4{};
    std::unique_ptr<std::mutex> m_imageMutex;
    bool m_loading = false;
    bool m_queueUpdate = false;

    bool m_transformationDirty{false};
    bool m_imageDirty{false};

    QSize m_thumbnail;
    QSize m_thumbnailOriginalSize;

    friend TestSourceImageItem;
};

}
}
}
}
