#pragma once

#include <QSGSimpleTextureNode>

#include "system/types.h"

class QSGTexture;
class QQuickWindow;
class TestImageNode;

namespace precitec
{

namespace image
{
template <typename T> class TLineImage;
typedef TLineImage<byte> BImage;
}

namespace gui
{
namespace components
{
namespace image
{

/**
 * The ImageNode is able to render a grayscale image as captured by the frame grabber.
 *
 **/
class ImageNode : public QSGSimpleTextureNode
{
public:
    explicit ImageNode();
    ~ImageNode() override;

    /**
     * Sets the @p window which is used to create the texture from.
     * Must be called before setting the image.
     **/
    void setWindow(QQuickWindow *window)
    {
        m_window = window;
    }

    /**
     * Sets the gray scale @p image which should be rendered by this node.
     * The node performs a deep copy into a QImage.
     **/
    void setImage(const precitec::image::BImage &image);
    
    /**
     * Enables rendering the actual image data, if false, a black image is rendered.
     */
    void setEnabled( bool enabled ) { m_enabled = enabled; }

private:
    QImage m_image;
    QImage m_dummyImage;
    QQuickWindow *m_window = nullptr;
    friend TestImageNode;
    bool m_enabled;
    QSGTexture *m_imageTexture = nullptr;
    QSGTexture *m_dummyTexture = nullptr;
};


}
}
}
}
