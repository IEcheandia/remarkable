#include "imageNode.h"

#include "image/image.h"

#include <QQuickWindow>

namespace precitec
{
namespace gui
{
namespace components
{
namespace image
{


ImageNode::ImageNode()
    : QSGSimpleTextureNode()
    , m_enabled( true )
{
    setFiltering(QSGTexture::Linear);
    m_dummyImage = QImage{ 1024, 1024, QImage::Format_Grayscale8 };
    m_dummyImage.fill( 0 );
}

ImageNode::~ImageNode()
{
    delete m_imageTexture;
    delete m_dummyTexture;
}

void ImageNode::setImage(const precitec::image::BImage &image)
{
    if (!m_window)
    {
        return;
    }

    if (!m_enabled && (m_dummyImage.width() != image.width() || m_dummyImage.height() != image.height() || !m_dummyTexture) )
    {
        m_dummyImage = QImage{image.width(), image.height(), QImage::Format_Grayscale8};
        m_dummyImage.fill( 0 );

        delete m_dummyTexture;
        m_dummyTexture = m_window->createTextureFromImage(m_dummyImage, QQuickWindow::CreateTextureOptions(QQuickWindow::TextureOwnsGLTexture | QQuickWindow::TextureIsOpaque));
    }
    
    // perform deep copy
    if (m_enabled && image.isValid())
    {
        if (m_image.width() != image.width() || m_image.height() != image.height())
        {
            m_image = QImage{image.width(), image.height(), QImage::Format_Grayscale8};
        }
        if ( image.height() > 2 && image.isContiguos() && (image.rowBegin(1) - image.rowBegin(0)) % 4 == 0)
        {
            //QImage constructor: data must be 32-bit aligned
            std::copy(image.begin(), image.end(), m_image.bits());
        }
        else
        {
            for (auto y = 0; y < image.height(); ++y)
            {
                std::copy(image.rowBegin(y), image.rowEnd(y), m_image.scanLine(y));
            }
        }

        // we need to always recreate the texture, unfortunately QSGTexture does not provide functionality to re-bind the data.
        // TODO: replace through customized texture and don't use a QImage at all.
        delete m_imageTexture;
        m_imageTexture = m_window->createTextureFromImage(m_image, QQuickWindow::CreateTextureOptions(QQuickWindow::TextureOwnsGLTexture | QQuickWindow::TextureIsOpaque));
    } else if (m_enabled && !m_imageTexture)
    {
        m_imageTexture = m_window->createTextureFromImage(m_image, QQuickWindow::CreateTextureOptions(QQuickWindow::TextureOwnsGLTexture | QQuickWindow::TextureIsOpaque));
    }

    if (m_enabled)
    {
        if (m_imageTexture)
        {
            setTexture(m_imageTexture);
            setRect(QRectF(0, 0, m_image.width(), m_image.height()));
        }
    }
    else if (m_dummyTexture)
    {
        setTexture(m_dummyTexture);
        setRect(QRectF(0, 0, m_dummyImage.width(), m_dummyImage.height()));
    }

    markDirty(QSGNode::DirtyForceUpdate);
}

}
}
}
}
