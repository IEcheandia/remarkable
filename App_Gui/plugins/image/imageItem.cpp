#include "imageItem.h"
#include "imageNode.h"
#include "nodeBasedOverlayCanvas.h"
#include "overlayGroupModel.h"
#include "overlayGroupFilterModel.h"

#include "image/image.h"

#include <QDateTime>
#include <QStandardPaths>
#include <QSGSimpleTextureNode>
#include <QQuickItemGrabResult>
#include <QQuickWindow>
#include <QDir>

#include <precitec/notificationSystem.h>

namespace precitec
{
namespace gui
{
namespace components
{
namespace image
{


ImageItem::ImageItem(QQuickItem *parent)
    : QQuickItem(parent)
    , m_overlayCanvas(std::make_unique<NodeBasedOverlayCanvas>())
    , m_overlayGroupModel(new OverlayGroupModel(this))
    , m_overlayGroupFilterModel(new OverlayGroupFilterModel(this))
{
    m_overlayGroupFilterModel->setSourceModel(m_overlayGroupModel);
    setFlag(QQuickItem::ItemHasContents);
    setClip(true);
    connect(this, &ImageItem::imageDataChanged, this, &ImageItem::updateAvailableOverlays, Qt::QueuedConnection);
    connect(this, &ImageItem::imageDataChanged, this, &ImageItem::update, Qt::QueuedConnection);
    connect(this, &QQuickItem::windowChanged, this, [this] (QQuickWindow *w) { m_overlayCanvas->setWindow(w); });

    connect(m_overlayGroupModel, &OverlayGroupModel::dataChanged, this,
        [this] (const QModelIndex &topLeft, const QModelIndex &bottomRight, const QVector<int> &roles)
        {
            if (roles.contains(Qt::UserRole + 1))
            {
                for (int i = topLeft.row() ; i <= bottomRight.row(); i++)
                {
                    const auto index = m_overlayGroupModel->index(i, 0, QModelIndex());
                    const bool enabled = m_overlayGroupModel->data(index, Qt::UserRole + 1).toBool();
                    const auto layers = m_overlayGroupModel->layers(index);
                    for (auto layer : layers)
                    {
                        m_overlayCanvas->setLayerEnabled(layer, enabled);
                    }
                    if (i == int(OverlayGroupModel::OverlayGroup::LiveImage))
                    {
                        m_imageDirty = true;
                    }
                    else
                    {
                        m_overlayDirty = true;
                    }
                }
                update();
                emit overlayEnabledChanged();
            }
        }
    );

    connect(this, &ImageItem::imageSizeChanged, this, &ImageItem::updateTransformation, Qt::QueuedConnection);
    connect(this, &ImageItem::zoomChanged, this, &ImageItem::updateTransformation);
    connect(this, &ImageItem::widthChanged, this, &ImageItem::updateTransformation);
    connect(this, &ImageItem::heightChanged, this, &ImageItem::updateTransformation);
    connect(this, &ImageItem::panningChanged, this, &ImageItem::updateTransformation);
    connect(this, &ImageItem::mirrorXChanged, this, &ImageItem::updateTransformation);
    connect(this, &ImageItem::mirrorYChanged, this, &ImageItem::updateTransformation);
    connect(this, &ImageItem::imageSizeChanged, this, &ImageItem::paintedRectChanged);
    connect(this, &ImageItem::widthChanged, this, &ImageItem::zoomToFit);
    connect(this, &ImageItem::heightChanged, this, &ImageItem::zoomToFit);
}

ImageItem::~ImageItem() = default;

void ImageItem::setImageData(const ImageData &data)
{
    m_image = data;
    const auto &image = std::get<precitec::image::BImage>(data);
    const auto newSize = QSizeF(image.width(), image.height());
    if (newSize != m_imageSize)
    {
        const bool wasNull = m_imageSize.isNull();
        m_imageSize = newSize;
        m_imageSizeDirty = true;
        emit imageSizeChanged();
        if (wasNull && newSize.isValid() && !newSize.isNull())
        {
            zoomToFit();
        }
    }
    const auto &context = std::get<precitec::interface::ImageContext>(data);
    const auto newHWROI = QPoint(context.HW_ROI_x0, context.HW_ROI_y0);
    if (newHWROI != m_hwROI)
    {
        m_hwROI = newHWROI;
        emit hwROIChanged();
    }
    
    bool hasScannerPosition = context.m_ScannerInfo.m_hasPosition;
    bool positionChanged = false;
    if ( hasScannerPosition )
    {
        positionChanged = (m_ScannerInfo.m_x != context.m_ScannerInfo.m_x) || (m_ScannerInfo.m_y != context.m_ScannerInfo.m_y) ;
        if (positionChanged)
        {
            m_ScannerInfo.m_x = context.m_ScannerInfo.m_x;
            m_ScannerInfo.m_y = context.m_ScannerInfo.m_y;
        }
    }
    if (m_ScannerInfo.m_hasPosition != hasScannerPosition)
    {
        m_ScannerInfo.m_hasPosition = hasScannerPosition;
        emit hasScannerPositionChanged();
    }
    if (positionChanged)
    {
        emit scannerPositionChanged();
    }
    

    if (!m_imageSize.isNull())
    {
        m_emitImageRendered = true;
    }
    m_imageDirty = true;
    m_overlayDirty = true;
    emit imageDataChanged();
}

void ImageItem::updateAvailableOverlays()
{
    const auto &canvas = std::get<precitec::image::OverlayCanvas>(m_image);
    // one less for live image which is not based on the overlays
    for (int i = 0 ; i < m_overlayGroupModel->rowCount(QModelIndex()) - 1; i++)
    {
        const auto index = m_overlayGroupModel->index(i, 0, QModelIndex());
        const auto layers = m_overlayGroupModel->layers(index);
        bool available = false;
        for (const auto layer : layers)
        {
            available |= canvas.getLayer(layer).hasShapes();
        }
        m_overlayGroupModel->setAvailable(index, available);
    }
    // and enable live image
    m_overlayGroupModel->setAvailable(m_overlayGroupModel->index(m_overlayGroupModel->rowCount(QModelIndex()) -1, 0, QModelIndex()), true);
}

QAbstractItemModel *ImageItem::overlayGroupModel() const
{
    return m_overlayGroupFilterModel;
}

QSGNode *ImageItem::updatePaintNode(QSGNode *oldNode, QQuickItem::UpdatePaintNodeData *updatePaintNodeData)
{
    Q_UNUSED(updatePaintNodeData)

    QSGTransformNode *transformNode = nullptr;
    ImageNode *imageNode = nullptr;
    QSGClipNode *clipNode = nullptr;

    if (oldNode)
    {
        transformNode = dynamic_cast<QSGTransformNode*>(oldNode);
        imageNode = dynamic_cast<ImageNode*>(transformNode->firstChild());
        clipNode = dynamic_cast<QSGClipNode*>(transformNode->lastChild());
    } else
    {
        transformNode = new QSGTransformNode();
        imageNode = new ImageNode();
        imageNode->setWindow(window());
        transformNode->appendChildNode(imageNode);
        clipNode = new QSGClipNode;
        clipNode->setIsRectangular(true);
        transformNode->appendChildNode(clipNode);
        m_transformationDirty = true;
        m_imageSizeDirty = true;
        m_imageDirty = true;
        m_overlayDirty = true;
    }
    if (m_transformationDirty)
    {
        transformNode->setMatrix(m_transformation);
        m_transformationDirty = false;
        if (m_zoomDirty && !m_overlayDirty)
        {
            // size of points is not influenced by m_transformation, so it has to be handled seperate
            m_overlayCanvas->zoom(m_zoom);
            m_zoomDirty = false;
        }
    }
    if (m_imageSizeDirty)
    {
        clipNode->setClipRect(QRectF(QPointF(0, 0), m_imageSize));
        m_imageSizeDirty = false;
    }

    const auto &imageData = m_image;
    if (m_imageDirty)
    {
        const bool imageNodeEnabled = m_overlayGroupModel->index(OverlayGroupModel::OverlayGroup::LiveImage).data(Qt::UserRole + 1).toBool();
        imageNode->setEnabled( imageNodeEnabled );
        imageNode->setImage(std::get<precitec::image::BImage>(imageData));
        m_imageDirty = false;
    }

    if (m_overlayDirty)
    {
        while (clipNode->firstChild())
        {
            delete clipNode->lastChild();
        }

        m_overlayCanvas->sync(clipNode, std::get<precitec::image::OverlayCanvas>(imageData), m_imageSize.toSize());
        m_overlayCanvas->zoom(m_zoom);
        m_overlayDirty = false;
        m_zoomDirty = false;
    }

    if (m_emitImageRendered)
    {
        m_emitImageRendered = false;
        emit rendered();
    }

    return transformNode;
}

void ImageItem::updateTransformation()
{
    m_transformation = QMatrix4x4{};

    QVector2D panningFactor{1.0, 1.0};
    if (m_mirrorX)
    {
        m_transformation.scale(-1.0, 1.0, 1.0);
        m_transformation.translate(-width(), 0.0, 0.0);
        panningFactor.setX(-1.0);
    }
    if (m_mirrorY)
    {
        m_transformation.scale(1.0, -1.0, 1.0);
        m_transformation.translate(0.0, - height(), 0.0);
        panningFactor.setY(-1.0);
    }

    // move into center
    if (m_imageSize.isValid())
    {
        m_transformation.translate((width() - m_imageSize.width() * m_zoom) * 0.5, (height() - imageSize().height() * m_zoom) * 0.5);
    }

    m_transformation.translate(panningFactor.x() * m_panning.x(), panningFactor.y() * m_panning.y());
    // and scale
    m_transformation.scale(m_zoom);
    m_transformationDirty = true;
    update();
    emit paintedRectChanged();
}

void ImageItem::setZoom(qreal zoom)
{
    zoom = qBound(minZoom(), zoom, maxZoom());
    if (m_zoom == zoom)
    {
        return;
    }
    m_zoom = zoom;
    m_zoomDirty = true;
    emit zoomChanged();
}

void ImageItem::setPanning(const QPointF &panning)
{
    if (m_panning == panning)
    {
        return;
    }
    m_panning = panning;
    emit panningChanged();
}

QRectF ImageItem::paintedRect() const
{
    if (!m_imageSize.isValid())
    {
        return {};
    }
    return m_transformation.mapRect(QRectF{{0, 0}, m_imageSize});
}

QPointF ImageItem::mapToPaintedImage(const QPointF &pos) const
{
    QMatrix4x4 matrix;
    matrix.scale(m_zoom);
    return matrix.map(pos);
}

QPointF ImageItem::mapFromPaintedImage(const QPointF &pos) const
{
    QMatrix4x4 matrix;
    matrix.scale(m_zoom);
    return matrix.inverted().map(pos);
}

void ImageItem::zoomToFit()
{
    if (!size().isValid() || !m_imageSize.isValid() || qFuzzyIsNull(m_imageSize.width()) || qFuzzyIsNull(m_imageSize.height()))
    {
        return;
    }
    setZoom(std::min(width()/m_imageSize.width(), height()/m_imageSize.height()));
}

void ImageItem::save()
{
    auto saveImage = prepareImage();
    if (!saveImage.isNull())
    {
        const auto name = QDateTime::currentDateTime().toString(QStringLiteral("yyyyMMdd-HHmmss-zzz"));
        saveImage.save(QStandardPaths::writableLocation(QStandardPaths::PicturesLocation) + QStringLiteral("/%1-screenshot.png").arg(name));
        notifications::NotificationSystem::instance()->information(tr("Screenshot %1 saved").arg(name), QStringLiteral("camera-photo"));
    }
}

void ImageItem::saveWithOverlays()
{
    const auto grabber = grabToImage();
    connect(grabber.data(), &QQuickItemGrabResult::ready, this, [grabber]
        {
            const auto name = QDateTime::currentDateTime().toString(QStringLiteral("yyyyMMdd-HHmmss-zzz"));
            // we need a const temporary variable to be able to access QQuickItemGrabResult::saveToFile() const
            // the non-const variant is marked as deprecated and causes a warning
            const auto* const helper = grabber.data();
            if (helper->saveToFile(QStandardPaths::writableLocation(QStandardPaths::PicturesLocation) + QStringLiteral("/%1-screenshot.png").arg(name)))
            {
                notifications::NotificationSystem::instance()->information(tr("Camera Image with overlays %1.png saved").arg(name), QStringLiteral("camera-photo"));
            }
            else
            {
                notifications::NotificationSystem::instance()->error(tr("Saving camera image failed"), QStringLiteral("camera-photo"));
            }
        });
}

namespace
{
void createDirIfNotExists(const QDir& dir)
{
    if (!dir.exists())
    {
        dir.mkpath(dir.absolutePath());
    }
}
}

void ImageItem::save(const QString& filePath)
{
    const auto dir = QDir{filePath};
    createDirIfNotExists(dir);

    const auto saveImage = prepareImage();
    if (!saveImage.isNull())
    {
        const auto name = QDateTime::currentDateTime().toString(QStringLiteral("yyyyMMdd-HHmmss-zzz"));
        saveImage.save(dir.absoluteFilePath(QStringLiteral("%1.png").arg(name)));
        notifications::NotificationSystem::instance()->information(tr("Camera Image %1.png saved").arg(name), QStringLiteral("camera-photo"));
    }
}

void ImageItem::saveSubImage(const QRectF& selection, const QString& filePath)
{
    const QFileInfo file{filePath};
    createDirIfNotExists(file.absoluteDir());

    const auto saveImage = prepareImage().copy(selection.toRect());
    if (!saveImage.isNull())
    {
        saveImage.save(file.absoluteFilePath());
        notifications::NotificationSystem::instance()->information(tr("Template %1 saved").arg(file.fileName()), QStringLiteral("camera-photo"));
    }
    else
    {
        notifications::NotificationSystem::instance()->warning(tr("Resulting image is empty and thus not saved.").arg(file.fileName()), QStringLiteral("camera-photo"));
    }
}

QImage ImageItem::prepareImage()
{
    const auto &image = std::get<precitec::image::BImage>(m_image);
    QImage saveImage;
    if (image.isValid())
    {
        if ( image.height() > 2 && image.isContiguos() && (image.rowBegin(1) - image.rowBegin(0)) % 4 == 0)
        {
            //QImage constructor: data must be 32-bit aligned
            saveImage = QImage{image.data(), image.width(), image.height(), QImage::Format_Grayscale8}.copy();
        }
        else
        {
            saveImage = QImage{image.width(), image.height(), QImage::Format_Grayscale8};
            for (auto y = 0; y < image.height(); ++y)
            {
                std::copy(image.rowBegin(y), image.rowEnd(y), saveImage.scanLine(y));
            }
        }
    }
    return saveImage;
}

bool ImageItem::isLineOverlayEnabled() const
{
    return m_overlayGroupModel->index(OverlayGroupModel::OverlayGroup::Line).data(Qt::UserRole + 1).toBool();
}

bool ImageItem::isContourOverlayEnabled() const
{
    return m_overlayGroupModel->index(OverlayGroupModel::OverlayGroup::Contour).data(Qt::UserRole + 1).toBool();
}

bool ImageItem::isPositionOverlayEnabled() const
{
    return m_overlayGroupModel->index(OverlayGroupModel::OverlayGroup::Position).data(Qt::UserRole + 1).toBool();
}

bool ImageItem::isTextOverlayEnabled() const
{
    return m_overlayGroupModel->index(OverlayGroupModel::OverlayGroup::Text).data(Qt::UserRole + 1).toBool();
}

bool ImageItem::isGridOverlayEnabled() const
{
    return m_overlayGroupModel->index(OverlayGroupModel::OverlayGroup::Grid).data(Qt::UserRole + 1).toBool();
}

bool ImageItem::isImageOverlayEnabled() const
{
    return m_overlayGroupModel->index(OverlayGroupModel::OverlayGroup::Image).data(Qt::UserRole + 1).toBool();
}

void ImageItem::setLineOverlayEnabled(bool enabled)
{
    m_overlayGroupModel->setData(m_overlayGroupModel->index(OverlayGroupModel::OverlayGroup::Line), enabled, Qt::UserRole + 1);
}

void ImageItem::setContourOverlayEnabled(bool enabled)
{
    m_overlayGroupModel->setData(m_overlayGroupModel->index(OverlayGroupModel::OverlayGroup::Contour), enabled, Qt::UserRole + 1);
}

void ImageItem::setPositionOverlayEnabled(bool enabled)
{
    m_overlayGroupModel->setData(m_overlayGroupModel->index(OverlayGroupModel::OverlayGroup::Position), enabled, Qt::UserRole + 1);
}

void ImageItem::setTextOverlayEnabled(bool enabled)
{
    m_overlayGroupModel->setData(m_overlayGroupModel->index(OverlayGroupModel::OverlayGroup::Text), enabled, Qt::UserRole + 1);
}

void ImageItem::setGridOverlayEnabled(bool enabled)
{
    m_overlayGroupModel->setData(m_overlayGroupModel->index(OverlayGroupModel::OverlayGroup::Grid), enabled, Qt::UserRole + 1);
}

void ImageItem::setImageOverlayEnabled(bool enabled)
{
    m_overlayGroupModel->setData(m_overlayGroupModel->index(OverlayGroupModel::OverlayGroup::Image), enabled, Qt::UserRole + 1);
}

void ImageItem::setMirrorX(bool enabled)
{
    if (m_mirrorX == enabled)
    {
        return;
    }
    m_mirrorX = enabled;
    emit mirrorXChanged();
}

void ImageItem::setMirrorY(bool enabled)
{
    if (m_mirrorY == enabled)
    {
        return;
    }
    m_mirrorY = enabled;
    emit mirrorYChanged();
}

}
}
}
}
