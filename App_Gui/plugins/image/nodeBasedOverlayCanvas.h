#pragma once

#include "overlay/overlayCanvas.h"
#include "overlay/layerType.h"

#include <QSize>

class QQuickWindow;
class QSGNode;

namespace precitec
{
namespace gui
{
namespace components
{
namespace image
{

class LineNode;
class PixelNode;
class TextNode;

/**
 * @brief Specialisation of OverlayCanvas which creates @link{QSGNode} for each overlay item.
 *
 * The class is an intended helper for the @link{ImageItem} and mainly used from
 * the updatePaintNode step. This class provides a @link{sync} method which takes a QSGNode and
 * another OverlayCanvas. The method creates a QSGNode for each element in the overlay canvas
 * and attaches it the provided node.
 *
 * The node is only added if the layer for the overlay item is enabled (see @link{setLayerEnabled}).
 **/
class NodeBasedOverlayCanvas : public precitec::image::OverlayCanvas
{
public:
    explicit NodeBasedOverlayCanvas();
    ~NodeBasedOverlayCanvas() override;

    /**
     * Syncs this NodeBasedOverlayCanvas to the elements from the @p canvas and creates
     * appropriate child nodes for @p parentNode.
     **/
    void sync(QSGNode *parentNode, const precitec::image::OverlayCanvas &canvas, const QSize &imageSize);

    /**
     * Sets lineWidth of all PointNodes to @p zoom but minimum 1 and maximum 3.
     **/
    void zoom(float zoom);

    /**
     * @p enable the @p layer for rendering.
     **/
    void setLayerEnabled(precitec::image::LayerType layer, bool enable);

    /**
     * @returns whether @p layer is enabled
     **/
    bool isLayerEnabled(precitec::image::LayerType layer) const;

    /**
     * Creates nodes for the elements in this canvas and adds them to @p parentNode.
     **/
    void createNodes(QSGNode *parentNode);

    /**
     * Sets the @p window the QQuickItem using this NodeBasedOverlayCanvas is attached to.
     * This is required for creating texture based elements (such as image and text overlay items).
     **/
    void setWindow(QQuickWindow *window)
    {
        m_window = window;
    }

    void writeRect(int layer, const geo2d::Rect &rectangle, const precitec::image::Color &c) override;
    void writePixel(int layer, int x, int y, const precitec::image::Color &c) override;
    void writeLine(int layer, int x0, int y0, int x1, int y1, const precitec::image::Color &c) override;
    void writeCircle(int layer, int x, int y, int r, const precitec::image::Color &c) override;
    void writeText(int layer, const std::string &text, const precitec::image::Font &font, const precitec::geo2d::Rect &bounds, const precitec::image::Color &c, int index) override;
    void writeImage(int layer, const geo2d::Point &position, const precitec::image::BImage& image, const precitec::image::OverlayText& title) override;
    void writePixelList(int layer, const geo2d::Point &p_oPosition, const std::vector<int> & y, const precitec::image::Color &c) override;
    void writeConnectedPixelList(int layer, const geo2d::Point &p_oPosition, const std::vector<int> & y, const precitec::image::Color &c) override;
    // TODO: add more write methods

private:
    bool isLayerEnabled(int layer) const
    {
        return isLayerEnabled(static_cast<precitec::image::LayerType>(layer));
    }

    LineNode *lineNodeAtLayer(int layer);

    PixelNode *pixelNodeAtLayer(int layer);

    TextNode *textNodeAtLayer(int layer);

    QSGNode *createTextNode(const std::string &text, const precitec::image::Font &font, const precitec::geo2d::Rect &bounds, const precitec::image::Color &color);
    QQuickWindow *m_window = nullptr;
    QSGNode *m_parentNode = nullptr;
    std::map<precitec::image::LayerType, bool> m_enabledLayers;
    std::array<LineNode*, precitec::image::eNbLayers> m_lineNodes;
    std::array<PixelNode*, precitec::image::eNbLayers> m_pixelNodes;
    std::array<TextNode*, precitec::image::eNbLayers> m_textNodes;
    QSize m_imageSize;
};

}
}
}
}
