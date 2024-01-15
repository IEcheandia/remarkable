#include "sourceImageItem.h"

#include <QSGSimpleTextureNode>
#include <QSGImageNode>
#include <QQuickWindow>
#include <QtConcurrentRun>
#include <QFutureWatcher>
#include <QDir>

namespace precitec
{
namespace gui
{
namespace components
{
namespace image
{

SourceImageItem::SourceImageItem(QQuickItem *parent)
    : QQuickItem(parent)
    , m_imageMutex(std::make_unique<std::mutex>())
{
    setFlag(QQuickItem::ItemHasContents);
    setClip(true);

    connect(this, &SourceImageItem::imageChanged, this, &SourceImageItem::update, Qt::QueuedConnection);
    connect(this, &SourceImageItem::sourceChanged, this, &SourceImageItem::updateImage, Qt::QueuedConnection);

    connect(this, &SourceImageItem::zoomChanged, this, &SourceImageItem::updateTransformation);
    connect(this, &SourceImageItem::panningChanged, this, &SourceImageItem::updateTransformation);
    connect(this, &SourceImageItem::widthChanged, this, &SourceImageItem::updateTransformation);
    connect(this, &SourceImageItem::heightChanged, this, &SourceImageItem::updateTransformation);
    connect(this, &SourceImageItem::pauseUpdateChanged, this, &SourceImageItem::updateTransformation);
}

SourceImageItem::~SourceImageItem() = default;

QSGNode *SourceImageItem::updatePaintNode(QSGNode *oldNode, QQuickItem::UpdatePaintNodeData *)
{
    if (m_pauseUpdate)
    {
        return oldNode;
    }

    QSGTransformNode* transformNode = nullptr;
    QSGImageNode* imageNode = nullptr;
    QSGClipNode* clipNode = nullptr;

    if (oldNode)
    {
        transformNode = dynamic_cast<QSGTransformNode*>(oldNode);
        imageNode = dynamic_cast<QSGImageNode*>(transformNode->firstChild());
        clipNode = dynamic_cast<QSGClipNode*>(transformNode->lastChild());
    }
    else
    {
        transformNode = new QSGTransformNode();

        imageNode = window()->createImageNode();
        imageNode->setFiltering(QSGTexture::Linear);
        imageNode->setOwnsTexture(true);
        transformNode->appendChildNode(imageNode);

        clipNode = new QSGClipNode;
        clipNode->setIsRectangular(true);
        transformNode->appendChildNode(clipNode);

        m_transformationDirty = true;
        m_imageDirty = true;
    }

    if (m_transformationDirty)
    {
        transformNode->setMatrix(m_transformation);
        m_transformationDirty = false;
    };

    if (m_imageDirty)
    {
        std::lock_guard<std::mutex> guard{*m_imageMutex};
        auto *texture = window()->createTextureFromImage(m_image, QQuickWindow::CreateTextureOptions(QQuickWindow::TextureOwnsGLTexture | QQuickWindow::TextureIsOpaque));
        imageNode->setTexture(texture);
        imageNode->setRect(0, 0, m_imageSize.width(), m_imageSize.height());

        clipNode->setClipRect(QRectF(QPointF(0, 0), m_imageSize));
        m_imageDirty = false;
    }

    return transformNode;
}

void SourceImageItem::setSource(const QString& file)
{
    if (m_source.compare(file) == 0)
    {
        return;
    }
    m_source = file;
    emit sourceChanged();
}

void SourceImageItem::setZoom(qreal zoom)
{
    zoom = qBound(0.1, zoom, 5.0);
    if (m_zoom == zoom)
    {
        return;
    }
    m_zoom = zoom;
    emit zoomChanged();
}

void SourceImageItem::setPanning(const QPointF &panning)
{
    if (m_panning == panning)
    {
        return;
    }
    m_panning = panning;
    emit panningChanged();
}

QRectF SourceImageItem::paintedRect() const
{
    if (m_imageSize.isEmpty())
    {
        return {};
    }

    return m_transformation.mapRect(QRectF{{0, 0}, m_imageSize});
}

void SourceImageItem::updateImage()
{
    if (m_source.isNull())
    {
        {
            std::lock_guard<std::mutex> guard{*m_imageMutex};
            m_image = {};
            m_imageSize = {};
            m_imageDirty = true;
        }
        emit imageChanged();
        return;
    }
    if (isLoading())
    {
        m_queueUpdate = true;
        return;
    }
    setLoading(true);

    auto watcher{new QFutureWatcher<void>{this}};
    connect(watcher, &QFutureWatcher<void>::finished, this,
        [this, watcher]
        {
            watcher->deleteLater();
            setLoading(false);
            emit imageChanged();
            if (m_queueUpdate)
            {
                m_queueUpdate = false;
                updateImage();
            }
        }
    );
    watcher->setFuture(QtConcurrent::run(
        [this]
        {
            QImage temporary{imageOrThumbnail()};
            std::lock_guard<std::mutex> guard{*m_imageMutex};
            m_image = std::move(temporary);
            if (!m_thumbnail.isValid())
            {
                m_imageSize = m_image.size();
            }
            else
            {
                m_imageSize = m_thumbnailOriginalSize;
            }
            m_imageDirty = true;
        }
    ));
}

QImage SourceImageItem::imageOrThumbnail()
{
    if (!m_thumbnail.isValid())
    {
        return QImage{m_source};
    }
    const auto thumbnailFile{thumbnailFilePath()};
    if (!thumbnailFile.exists())
    {
        return createThumbnail(thumbnailFile);
    }
    return QImage{thumbnailFile.absoluteFilePath()};
}

QFileInfo SourceImageItem::thumbnailFilePath()
{
    QFileInfo original{m_source};
    const auto newName{original.baseName() + QStringLiteral("_%1x%2").arg(m_thumbnail.width()).arg(m_thumbnail.height())};
    return QFileInfo{original.absoluteDir().absoluteFilePath(newName + QStringLiteral(".") + original.completeSuffix())};
}

QImage SourceImageItem::createThumbnail(const QFileInfo& thumbnail)
{
    const QImage original{m_source};
    const auto thumb{original.scaled(m_thumbnail, Qt::KeepAspectRatio, Qt::SmoothTransformation)};

    thumb.save(thumbnail.absoluteFilePath());

    return thumb;
}

void SourceImageItem::updateTransformation()
{
    if (m_pauseUpdate)
    {
        return;
    }

    m_transformation.setToIdentity();

    if (!size().isEmpty() && !m_imageSize.isEmpty())
    {
        m_transformation.translate((width() - m_imageSize.width() * m_zoom) * 0.5, (height() - m_imageSize.height() * m_zoom) * 0.5);
    }

    m_transformation.translate(m_panning.x(), m_panning.y());

    m_transformation.scale(m_zoom);
    m_transformationDirty = true;

    update();
    emit transformationChanged();
}

void SourceImageItem::zoomToFit()
{
    std::lock_guard<std::mutex> guard{*m_imageMutex};
    if (size().isEmpty() || m_imageSize.isEmpty())
    {
        return;
    }
    setZoom(std::min(width() / m_imageSize.width(), height() / m_imageSize.height()));
    setPanning(QPointF{0, 0});
}

QPointF SourceImageItem::mapToPaintedImage(const QPointF &pos) const
{
    QMatrix4x4 matrix;
    matrix.scale(m_zoom);
    return matrix.map(pos);
}

QPointF SourceImageItem::mapFromPaintedImage(const QPointF &pos) const
{
    QMatrix4x4 matrix;
    matrix.scale(m_zoom);
    return matrix.inverted().map(pos);
}

void SourceImageItem::centerAndFitTo(const QPointF& pos, const QSize& sz)
{
    std::lock_guard<std::mutex> guard{*m_imageMutex};
    if (size().isEmpty() || m_imageSize.isEmpty())
    {
        return;
    }
    setZoom(std::min(width() / sz.width(), height() / sz.height()));
    setPanning(-mapToPaintedImage(pos - 0.5 * QPoint{m_imageSize.width(), m_imageSize.height()}));
}

bool SourceImageItem::imageValid() const
{
    std::lock_guard<std::mutex> guard{*m_imageMutex};
    return !m_image.isNull();
}

QSize SourceImageItem::imageSize() const
{
    std::lock_guard<std::mutex> guard{*m_imageMutex};
    return m_imageSize;
}

void SourceImageItem::setLoading(bool set)
{
    if (m_loading == set)
    {
        return;
    }
    m_loading = set;
    emit loadingChanged();
}

void SourceImageItem::setPauseUpdate(bool pause)
{
    if (m_pauseUpdate == pause)
    {
        return;
    }
    m_pauseUpdate = pause;
    emit pauseUpdateChanged();
}

void SourceImageItem::setThumbnail(const QSize& size)
{
    std::unique_lock lock{*m_imageMutex};
    if (m_thumbnail == size)
    {
        return;
    }
    m_thumbnail = size;
    lock.unlock();
    emit thumbnailChanged();
}

void SourceImageItem::setThumbnailOriginalSize(const QSize& size)
{
    std::unique_lock lock{*m_imageMutex};
    if (m_thumbnailOriginalSize == size)
    {
        return;
    }
    m_thumbnailOriginalSize = size;
    lock.unlock();
    emit thumbnailOriginalSizeChanged();
}


}
}
}
}
