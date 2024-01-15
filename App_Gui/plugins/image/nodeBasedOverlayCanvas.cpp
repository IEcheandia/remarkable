#include "nodeBasedOverlayCanvas.h"
#include "imageNode.h"

#include "overlay/overlayPrimitive.h"

#include <QImage>
#include <QPainter>
#include <QSGNode>
#include <QSGSimpleRectNode>
#include <QSGSimpleTextureNode>
#include <QSGVertexColorMaterial>
#include <QQuickWindow>

#include <forward_list>

using namespace precitec::image;
using namespace precitec::geo2d;

namespace precitec
{
namespace gui
{
namespace components
{
namespace image
{

class LineNode : public QSGGeometryNode
{
public:
    LineNode();

    void addRect(const Rect &rectangle, const Color &c);
    void addLine(int x0, int y0, int x1, int y1, const Color &c);

    void sync();

private:
    std::forward_list<QSGGeometry::ColoredPoint2D> m_points;
    int m_pointCount = 0;
};

LineNode::LineNode()
{
    setFlag(QSGNode::OwnedByParent);
    setFlag(QSGNode::OwnsGeometry);
    setMaterial(new QSGVertexColorMaterial);
    setFlag(QSGNode::OwnsMaterial);
}

void LineNode::addRect(const Rect &rectangle, const Color &c)
{
    QSGGeometry::ColoredPoint2D point;
    point.set(rectangle.x().start(), rectangle.y().start(), c.red, c.green, c.blue, c.alpha);
    m_points.push_front(point);
    m_pointCount++;
    point.set(rectangle.x().start(), rectangle.y().start() + rectangle.height(), c.red, c.green, c.blue, c.alpha);
    m_points.push_front(point);
    m_pointCount++;

    point.set(rectangle.x().start(), rectangle.y().start() + rectangle.height(), c.red, c.green, c.blue, c.alpha);
    m_points.push_front(point);
    m_pointCount++;
    point.set(rectangle.x().start() + rectangle.width(), rectangle.y().start() + rectangle.height(), c.red, c.green, c.blue, c.alpha);
    m_points.push_front(point);
    m_pointCount++;

    point.set(rectangle.x().start() + rectangle.width(), rectangle.y().start() + rectangle.height(), c.red, c.green, c.blue, c.alpha);
    m_points.push_front(point);
    m_pointCount++;
    point.set(rectangle.x().start() + rectangle.width(), rectangle.y().start(), c.red, c.green, c.blue, c.alpha);
    m_points.push_front(point);
    m_pointCount++;

    point.set(rectangle.x().start() + rectangle.width(), rectangle.y().start(), c.red, c.green, c.blue, c.alpha);
    m_points.push_front(point);
    m_pointCount++;
    point.set(rectangle.x().start(), rectangle.y().start(), c.red, c.green, c.blue, c.alpha);
    m_points.push_front(point);
    m_pointCount++;
}

void LineNode::addLine(int x0, int y0, int x1, int y1, const Color &c)
{
    QSGGeometry::ColoredPoint2D point;
    point.set(x0, y0, c.red, c.green, c.blue, c.alpha);
    m_points.push_front(point);
    m_pointCount++;
    point.set(x1, y1, c.red, c.green, c.blue, c.alpha);
    m_points.push_front(point);
    m_pointCount++;
}

void LineNode::sync()
{
    auto geometry = new QSGGeometry(QSGGeometry::defaultAttributes_ColoredPoint2D(), m_pointCount);
    geometry->setDrawingMode(GL_LINES);
    geometry->setLineWidth(3);

    auto points = geometry->vertexDataAsColoredPoint2D();
    int i = 0;
    for (auto it = m_points.begin(); it != m_points.end(); it++, i++)
    {
        points[i].set(it->x, it->y, it->r, it->g, it->b, it->a);
    }

    setGeometry(geometry);
}


class PixelNode : public QSGGeometryNode
{
public:
    PixelNode();

    void addPixel(int x0, int y0, const Color &c);
    void addPixelList(const geo2d::Point &p_oPosition, const std::vector<int> & y, const precitec::image::Color &c);

    void sync();

private:
    std::forward_list<QSGGeometry::ColoredPoint2D> m_points;
    int m_pointCount = 0;
};

PixelNode::PixelNode()
{
    setFlag(QSGNode::OwnedByParent);
    setFlag(QSGNode::OwnsGeometry);
    setMaterial(new QSGVertexColorMaterial);
    setFlag(QSGNode::OwnsMaterial);
}

void PixelNode::addPixel(int x0, int y0, const Color &c)
{
    QSGGeometry::ColoredPoint2D point;
    point.set(x0, y0, c.red, c.green, c.blue, c.alpha);
    m_points.push_front(point);
    m_pointCount++;
}

void PixelNode::addPixelList(const geo2d::Point &p_oPosition, const std::vector<int> & y, const precitec::image::Color &c)
{
    const auto numPoints = y.size();

    float x = float (p_oPosition.x);
    const float y_offset(p_oPosition.y);

    const int * p_y = y.data();

    QSGGeometry::ColoredPoint2D point;
    for( auto end = y.data() + numPoints ; p_y != end; x += 1.0f, ++p_y)
    {
        point.set(x, y_offset + float(*p_y), c.red, c.green, c.blue, c.alpha);
        m_points.push_front(point);
        m_pointCount++;
    }
}

void PixelNode::sync()
{
    auto geometry = new QSGGeometry(QSGGeometry::defaultAttributes_ColoredPoint2D(), m_pointCount);
    geometry->setDrawingMode(GL_POINTS);

    auto points = geometry->vertexDataAsColoredPoint2D();
    int i = 0;
    for (auto it = m_points.begin(); it != m_points.end(); it++, i++)
    {
        points[i].set(it->x, it->y, it->r, it->g, it->b, it->a);
    }

    setGeometry(geometry);
}

class TextNode : public QSGSimpleTextureNode
{
public:
    TextNode(const QSize &imageSize);

    void addText(const QString &text, const QColor &color, const QFont &font, const QRect &bounds);

    void sync(QQuickWindow *window);

private:
    QImage m_image;
    QPainter m_painter;
};

TextNode::TextNode(const QSize &imageSize)
    : m_image(imageSize, QImage::Format_ARGB32_Premultiplied)
    , m_painter(&m_image)
{
    m_image.fill(Qt::transparent);
    setFlag(QSGNode::OwnedByParent);
    setOwnsTexture(true);
    setFiltering(QSGTexture::Linear);
}

void TextNode::addText(const QString &text, const QColor &color, const QFont &font, const QRect &bounds)
{
    m_painter.setFont(font);
    m_painter.setPen(color);
    m_painter.drawText(bounds, Qt::AlignLeft, text);
}

void TextNode::sync(QQuickWindow *window)
{
    setTexture(window->createTextureFromImage(m_image, QQuickWindow::CreateTextureOptions(QQuickWindow::TextureOwnsGLTexture | QQuickWindow::TextureHasAlphaChannel)));
    setRect(QRect{QPoint{0, 0}, m_image.size()});
}

NodeBasedOverlayCanvas::NodeBasedOverlayCanvas()
    : OverlayCanvas()
    , m_enabledLayers({
        {eLayerLine, true},
        {eLayerContour, true},
        {eLayerPosition, true},
        {eLayerText, true},
        {eLayerGridTransp, true},
        {eLayerLineTransp, true},
        {eLayerContourTransp, true},
        {eLayerPositionTransp, true},
        {eLayerTextTransp, true},
        {eLayerInfoBox, true},
        {eLayerImage, true}
    })
{
}

NodeBasedOverlayCanvas::~NodeBasedOverlayCanvas() = default;

void NodeBasedOverlayCanvas::zoom(float zoom)
{
    for (auto pixelNode : m_pixelNodes)
    {
        if (pixelNode)
        {
            pixelNode->geometry()->setLineWidth(zoom < 1 ? 1 : (zoom > 3 ? 3 : zoom));
        }
    }
}

void NodeBasedOverlayCanvas::sync(QSGNode *parentNode, const precitec::image::OverlayCanvas &canvas, const QSize &imageSize)
{
    m_imageSize = imageSize;
    layers_.clear();
    layers_.reserve(eNbLayers);
    for (int i = eLayerMin; i < eNbLayers; i++)
    {
        m_lineNodes[i] = nullptr;
        m_pixelNodes[i] = nullptr;
        m_textNodes[i] = nullptr;
        layers_.push_back(canvas.getLayer(i));
    }
    createNodes(parentNode);
    for (int i = eLayerMin; i < eNbLayers; i++)
    {
        auto lineNode = m_lineNodes[i];
        if (lineNode)
        {
            lineNode->sync();
        }

        auto pixelNode = m_pixelNodes[i];
        if (pixelNode)
        {
            pixelNode->sync();
        }

        auto textNode = m_textNodes[i];
        if (textNode)
        {
            textNode->sync(m_window);
        }
    }
}

void NodeBasedOverlayCanvas::createNodes(QSGNode *parentNode)
{
    m_parentNode = parentNode;
    write();
    m_parentNode = nullptr;
}

void NodeBasedOverlayCanvas::setLayerEnabled(precitec::image::LayerType layer, bool enable)
{
    auto it = m_enabledLayers.find(layer);
    if (it == m_enabledLayers.end())
    {
        return;
    }
    it->second = enable;
}

bool NodeBasedOverlayCanvas::isLayerEnabled(precitec::image::LayerType layer) const
{
    auto it = m_enabledLayers.find(layer);
    if (it == m_enabledLayers.end())
    {
        return false;
    }
    return it->second;
}

namespace
{

QColor toQt(const Color &c)
{
    return QColor(c.red, c.green, c.blue, c.alpha);
}

static QFont toQt(const precitec::image::Font &font)
{
    QFont f(QString::fromStdString(font.name), font.size);
    f.setBold(font.bold);
    f.setItalic(font.italic);
    return f;
}

QSGGeometryNode *createGeometryNode(QSGGeometry *geometry, QSGMaterial *material)
{
    auto node = new QSGGeometryNode;
    node->setFlag(QSGNode::OwnedByParent);
    node->setGeometry(geometry);
    node->setFlag(QSGNode::OwnsGeometry);
    node->setMaterial(material);
    node->setFlag(QSGNode::OwnsMaterial);
    return node;
}

}

void NodeBasedOverlayCanvas::writeRect(int layer, const Rect &rectangle, const Color &c)
{
    if (!isLayerEnabled(layer))
    {
        return;
    }
    lineNodeAtLayer(layer)->addRect(rectangle, c);
}

void NodeBasedOverlayCanvas::writePixel(int layer, int x, int y, const Color &c)
{
    if (!isLayerEnabled(layer))
    {
        return;
    }
    pixelNodeAtLayer(layer)->addPixel(x, y, c);
}

void NodeBasedOverlayCanvas::writeLine(int layer, int x0, int y0, int x1, int y1, const Color &c)
{
    if (!isLayerEnabled(layer))
    {
        return;
    }
    lineNodeAtLayer(layer)->addLine(x0, y0, x1, y1, c);
}

void NodeBasedOverlayCanvas::writeCircle(int layer, int x, int y, int r, const precitec::image::Color &c)
{
    if (!isLayerEnabled(layer))
    {
        return;
    }
    
    if (r <= 0)
    {
        return;
    }
    
    const std::size_t numPoints = 360 * r;
    auto geometry = new QSGGeometry(QSGGeometry::defaultAttributes_Point2D(), numPoints);
    geometry->setDrawingMode(GL_LINE_LOOP);

    for (std::size_t i = 0; i < numPoints; i++)
    {
        float theta = i * 2 * M_PI / numPoints;
        geometry->vertexDataAsPoint2D()[i].set(x + r * std::cos(theta), y - r * std::sin(theta));
    }

    auto material = new QSGFlatColorMaterial;
    material->setColor(toQt(c));

    m_parentNode->appendChildNode(createGeometryNode(geometry, material));
}

void NodeBasedOverlayCanvas::writeText(int layer, const std::string &text, const precitec::image::Font &font, const precitec::geo2d::Rect &bounds, const precitec::image::Color &c, int index)
{
    Q_UNUSED(index)
    if (!isLayerEnabled(layer))
    {
        return;
    }

    textNodeAtLayer(layer)->addText(QString::fromStdString(text), toQt(c), toQt(font), QRect{bounds.x().start(), bounds.y().start(), bounds.width(), bounds.height()});
}

QSGNode *NodeBasedOverlayCanvas::createTextNode(const std::string &text, const precitec::image::Font &font, const precitec::geo2d::Rect &bounds, const precitec::image::Color &color)
{
    if (!m_window)
    {
        return nullptr;
    }
    TextNode *node = new TextNode{QSize{bounds.width(), bounds.height()}};
    node->addText(QString::fromStdString(text), toQt(color), toQt(font), QRect{bounds.x().start(), bounds.y().start(), bounds.width(), bounds.height()});
    node->sync(m_window);
    return node;
}

void NodeBasedOverlayCanvas::writeImage(int layer, const geo2d::Point &position, const precitec::image::BImage& image, const precitec::image::OverlayText& title)
{
    if (!isLayerEnabled(layer))
    {
        return;
    }
    if (!m_window)
    {
        return;
    }
    auto node = new ImageNode;
    node->setFlag(QSGNode::OwnedByParent);
    node->setWindow(m_window);
    node->setImage(image);
    node->setRect(node->rect().translated(position.x, position.y));
    if (!title.text_.empty())
    {
        node->appendChildNode(createTextNode(title.text_, title.font_, title.bounds_, title.color_));
    }

    m_parentNode->appendChildNode(node);
}

void NodeBasedOverlayCanvas::writePixelList(int layer, const geo2d::Point &p_oPosition, const std::vector<int> & y, const precitec::image::Color &c)
{
    if (!isLayerEnabled(layer))
    {
        return;
    }
    pixelNodeAtLayer(layer)->addPixelList(p_oPosition, y, c);
}

void NodeBasedOverlayCanvas::writeConnectedPixelList(int layer, const geo2d::Point &p_oPosition, const std::vector<int> & y, const precitec::image::Color &c)
{
    if (!isLayerEnabled(layer))
    {
        return;
    }
    if (y.size() <= 1)
    {
        writePixelList(layer, p_oPosition, y, c);
        return;
    }

    int x0 = p_oPosition.x;
    int y0 = p_oPosition.y + y[0];
    for (int i = 1, n = y.size(); i < n; i++)
    {
        int x1 = p_oPosition.x + i;
        int y1 = p_oPosition.y + y[i];
        lineNodeAtLayer(layer)->addLine(x0, y0, x1, y1,c);
        x0 = x1;
        y0 = y1;
    }
}
LineNode *NodeBasedOverlayCanvas::lineNodeAtLayer(int layer)
{
    auto lineNode = m_lineNodes[layer];
    if (!lineNode)
    {
        lineNode = new LineNode;
        m_parentNode->appendChildNode(lineNode);
        m_lineNodes[layer] = lineNode;
    }
    return lineNode;
}

PixelNode *NodeBasedOverlayCanvas::pixelNodeAtLayer(int layer)
{
    auto pixelNode = m_pixelNodes[layer];
    if (!pixelNode)
    {
        pixelNode = new PixelNode;
        m_parentNode->appendChildNode(pixelNode);
        m_pixelNodes[layer] = pixelNode;
    }
    return pixelNode;
}

TextNode *NodeBasedOverlayCanvas::textNodeAtLayer(int layer)
{
    auto textNode = m_textNodes[layer];
    if (!textNode)
    {
        textNode = new TextNode{m_imageSize};
        m_parentNode->appendChildNode(textNode);
        m_textNodes[layer] = textNode;
    }
    return textNode;
}

}
}
}
}
