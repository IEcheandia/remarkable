#include <QTest>
#include <QSGFlatColorMaterial>
#include <QSGVertexColorMaterial>
#include <QSGNode>

#include "../nodeBasedOverlayCanvas.h"
#include "overlay/overlayPrimitive.h"

using precitec::gui::components::image::NodeBasedOverlayCanvas;
Q_DECLARE_METATYPE(precitec::image::LayerType)
Q_DECLARE_METATYPE(precitec::image::Color)

class TestNodeBasedOverlayCanvas : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testEnable_data();
    void testEnable();
    void testWriteRect_data();
    void testWriteRect();
    void testWritePixel_data();
    void testWritePixel();
    void testWriteLine_data();
    void testWriteLine();
    void testWriteCircle_data();
    void testWriteCircle();
    void testWritePixelList_data();
    void testWritePixelList();
};

void TestNodeBasedOverlayCanvas::testEnable_data()
{
    QTest::addColumn<precitec::image::LayerType>("layer");

    QTest::newRow("Line") << precitec::image::eLayerLine;
    QTest::newRow("Contour") << precitec::image::eLayerContour;
    QTest::newRow("Position") << precitec::image::eLayerPosition;
    QTest::newRow("Text") << precitec::image::eLayerText;
    QTest::newRow("Grid") << precitec::image::eLayerGridTransp;
    QTest::newRow("Transparent Line") << precitec::image::eLayerLineTransp;
    QTest::newRow("Transparent Contour") << precitec::image::eLayerContourTransp;
    QTest::newRow("Transparent Position") << precitec::image::eLayerPositionTransp;
    QTest::newRow("Transparent Text") << precitec::image::eLayerTextTransp;
    QTest::newRow("InfoBox") << precitec::image::eLayerInfoBox;
    QTest::newRow("Image") << precitec::image::eLayerImage;
}

void TestNodeBasedOverlayCanvas::testEnable()
{
    NodeBasedOverlayCanvas canvas;
    QFETCH(precitec::image::LayerType, layer);
    QCOMPARE(canvas.isLayerEnabled(layer), true);
    canvas.setLayerEnabled(layer, false);
    QCOMPARE(canvas.isLayerEnabled(layer), false);
    canvas.setLayerEnabled(layer, true);
    QCOMPARE(canvas.isLayerEnabled(layer), true);
}

void TestNodeBasedOverlayCanvas::testWriteRect_data()
{
    QTest::addColumn<precitec::image::LayerType>("layer");
    QTest::addColumn<QRectF>("geometry");
    QTest::addColumn<precitec::image::Color>("color");
    QTest::addColumn<QColor>("expectedColor");

    QTest::newRow("Line") << precitec::image::eLayerLine << QRectF(10, 20, 30, 40) << precitec::image::Color::Red() << QColor(Qt::red);
    QTest::newRow("Contour") << precitec::image::eLayerContour << QRectF(0, 0, 100, 50) << precitec::image::Color::Blue() << QColor(Qt::blue);
    QTest::newRow("Position") << precitec::image::eLayerPosition << QRectF(-10, 20, 100, 20) << precitec::image::Color::Green() << QColor(Qt::green);
    QTest::newRow("Text") << precitec::image::eLayerText << QRectF(1, 1, 2, 2) << precitec::image::Color::White() << QColor(Qt::white);
    QTest::newRow("Grid") << precitec::image::eLayerGridTransp << QRectF(20, 30, 2, 10) << precitec::image::Color::Black() << QColor(Qt::black);
    QTest::newRow("Transparent Line") << precitec::image::eLayerLineTransp << QRectF(0, 0, 10, 20) << precitec::image::Color::Yellow() << QColor(Qt::yellow);
    QTest::newRow("Transparent Contour") << precitec::image::eLayerContourTransp << QRectF(0, 0, 10, 20) << precitec::image::Color::Cyan() << QColor(Qt::cyan);
    QTest::newRow("Transparent Position") << precitec::image::eLayerPositionTransp << QRectF(0, 0, 10, 20) << precitec::image::Color::Orange() << QColor(255, 165, 0);
    QTest::newRow("Transparent Text") << precitec::image::eLayerTextTransp << QRectF(0, 0, 10, 20) << precitec::image::Color::Magenta() << QColor(Qt::magenta);
    QTest::newRow("InfoBox") << precitec::image::eLayerInfoBox << QRectF(0, 0, 10, 20) << precitec::image::Color::Magenta() << QColor(Qt::magenta);
    QTest::newRow("Image") << precitec::image::eLayerImage << QRectF(0, 0, 10, 20) << precitec::image::Color::Magenta() << QColor(Qt::magenta);
}

void TestNodeBasedOverlayCanvas::testWriteRect()
{
    NodeBasedOverlayCanvas canvas;
    NodeBasedOverlayCanvas setCanvas;
    QFETCH(precitec::image::LayerType, layer);
    QFETCH(QRectF, geometry);
    QFETCH(precitec::image::Color, color);
    setCanvas.getLayer(layer).add<precitec::image::OverlayRectangle>(geometry.x(), geometry.y(), geometry.width(), geometry.height(), color);

    QSGNode node;
    QCOMPARE(node.childCount(), 0);

    // let's test with the layer disabled
    canvas.setLayerEnabled(layer, false);
    canvas.sync(&node, canvas, QSize{});
    QCOMPARE(node.childCount(), 0);

    // now enable the layer
    canvas.setLayerEnabled(layer, true);
    canvas.sync(&node, setCanvas, QSize{});
    QCOMPARE(node.childCount(), 1);
    auto rect = dynamic_cast<QSGGeometryNode*>(node.childAtIndex(0));
    QVERIFY(rect);
    auto material = dynamic_cast<QSGVertexColorMaterial*>(rect->material());
    QVERIFY(material);
    QFETCH(QColor, expectedColor);

    QCOMPARE(rect->geometry()->drawingMode(), GLenum(GL_LINES));
    QCOMPARE(rect->geometry()->vertexCount(), 8);
    // the Rect substracts 1 for width and height, TODO: why???
    auto &point = rect->geometry()->vertexDataAsColoredPoint2D()[0];
    QCOMPARE(point.x, geometry.x());
    QCOMPARE(point.y, geometry.y());
    QCOMPARE(point.r, expectedColor.red());
    QCOMPARE(point.g, expectedColor.green());
    QCOMPARE(point.b, expectedColor.blue());
    QCOMPARE(point.a, expectedColor.alpha());

    point = rect->geometry()->vertexDataAsColoredPoint2D()[1];
    QCOMPARE(point.x, geometry.x() + geometry.width() -1);
    QCOMPARE(point.y, geometry.y());
    QCOMPARE(point.r, expectedColor.red());
    QCOMPARE(point.g, expectedColor.green());
    QCOMPARE(point.b, expectedColor.blue());
    QCOMPARE(point.a, expectedColor.alpha());

    point = rect->geometry()->vertexDataAsColoredPoint2D()[2];
    QCOMPARE(point.x, geometry.x() + geometry.width() -1);
    QCOMPARE(point.y, geometry.y());
    QCOMPARE(point.r, expectedColor.red());
    QCOMPARE(point.g, expectedColor.green());
    QCOMPARE(point.b, expectedColor.blue());
    QCOMPARE(point.a, expectedColor.alpha());

    point = rect->geometry()->vertexDataAsColoredPoint2D()[3];
    QCOMPARE(point.x, geometry.x() + geometry.width() -1);
    QCOMPARE(point.y, geometry.y() + geometry.height() -1);
    QCOMPARE(point.r, expectedColor.red());
    QCOMPARE(point.g, expectedColor.green());
    QCOMPARE(point.b, expectedColor.blue());
    QCOMPARE(point.a, expectedColor.alpha());

    point = rect->geometry()->vertexDataAsColoredPoint2D()[4];
    QCOMPARE(point.x, geometry.x() + geometry.width() -1);
    QCOMPARE(point.y, geometry.y() + geometry.height() -1);
    QCOMPARE(point.r, expectedColor.red());
    QCOMPARE(point.g, expectedColor.green());
    QCOMPARE(point.b, expectedColor.blue());
    QCOMPARE(point.a, expectedColor.alpha());

    point = rect->geometry()->vertexDataAsColoredPoint2D()[5];
    QCOMPARE(point.x, geometry.x());
    QCOMPARE(point.y, geometry.y() + geometry.height() -1);
    QCOMPARE(point.r, expectedColor.red());
    QCOMPARE(point.g, expectedColor.green());
    QCOMPARE(point.b, expectedColor.blue());
    QCOMPARE(point.a, expectedColor.alpha());

    point = rect->geometry()->vertexDataAsColoredPoint2D()[6];
    QCOMPARE(point.x, geometry.x());
    QCOMPARE(point.y, geometry.y() + geometry.height() -1);
    QCOMPARE(point.r, expectedColor.red());
    QCOMPARE(point.g, expectedColor.green());
    QCOMPARE(point.b, expectedColor.blue());
    QCOMPARE(point.a, expectedColor.alpha());

    point = rect->geometry()->vertexDataAsColoredPoint2D()[7];
    QCOMPARE(point.x, geometry.x());
    QCOMPARE(point.y, geometry.y());
    QCOMPARE(point.r, expectedColor.red());
    QCOMPARE(point.g, expectedColor.green());
    QCOMPARE(point.b, expectedColor.blue());
    QCOMPARE(point.a, expectedColor.alpha());
}

void TestNodeBasedOverlayCanvas::testWritePixel_data()
{
    QTest::addColumn<precitec::image::LayerType>("layer");
    QTest::addColumn<QPointF>("point");
    QTest::addColumn<precitec::image::Color>("color");
    QTest::addColumn<QColor>("expectedColor");

    QTest::newRow("Line") << precitec::image::eLayerLine << QPointF(10, 20) << precitec::image::Color::Red() << QColor(Qt::red);
    QTest::newRow("Contour") << precitec::image::eLayerContour << QPointF(0, 0) << precitec::image::Color::Blue() << QColor(Qt::blue);
    QTest::newRow("Position") << precitec::image::eLayerPosition << QPointF(-10, 20) << precitec::image::Color::Green() << QColor(Qt::green);
    QTest::newRow("Text") << precitec::image::eLayerText << QPointF(1, 1) << precitec::image::Color::White() << QColor(Qt::white);
    QTest::newRow("Grid") << precitec::image::eLayerGridTransp << QPointF(20, 30) << precitec::image::Color::Black() << QColor(Qt::black);
    QTest::newRow("Transparent Line") << precitec::image::eLayerLineTransp << QPointF(0, 0) << precitec::image::Color::Yellow() << QColor(Qt::yellow);
    QTest::newRow("Transparent Contour") << precitec::image::eLayerContourTransp << QPointF(0, 0) << precitec::image::Color::Cyan() << QColor(Qt::cyan);
    QTest::newRow("Transparent Position") << precitec::image::eLayerPositionTransp << QPointF(0, 0) << precitec::image::Color::Orange() << QColor(255, 165, 0);
    QTest::newRow("Transparent Text") << precitec::image::eLayerTextTransp << QPointF(0, 0) << precitec::image::Color::Magenta() << QColor(Qt::magenta);
    QTest::newRow("InfoBox") << precitec::image::eLayerInfoBox << QPointF(0, 0) << precitec::image::Color::Magenta() << QColor(Qt::magenta);
    QTest::newRow("Image") << precitec::image::eLayerImage << QPointF(0, 0) << precitec::image::Color::Magenta() << QColor(Qt::magenta);
}

void TestNodeBasedOverlayCanvas::testWritePixel()
{
    NodeBasedOverlayCanvas canvas;
    NodeBasedOverlayCanvas setCanvas;
    QFETCH(precitec::image::LayerType, layer);
    QFETCH(QPointF, point);
    QFETCH(precitec::image::Color, color);
    setCanvas.getLayer(layer).add<precitec::image::OverlayPoint>(point.x(), point.y(), color);

    QSGNode node;
    QCOMPARE(node.childCount(), 0);

    // let's test with the layer disabled
    canvas.setLayerEnabled(layer, false);
    canvas.sync(&node, setCanvas, QSize{});
    QCOMPARE(node.childCount(), 0);

    // now enable the layer
    canvas.setLayerEnabled(layer, true);
    canvas.sync(&node, setCanvas, QSize{});
    QCOMPARE(node.childCount(), 1);
    auto rect = dynamic_cast<QSGGeometryNode*>(node.childAtIndex(0));
    QVERIFY(rect);
    auto material = dynamic_cast<QSGVertexColorMaterial*>(rect->material());
    QVERIFY(material);
    QFETCH(QColor, expectedColor);

    QCOMPARE(rect->geometry()->drawingMode(), GLenum(GL_POINTS));
    QCOMPARE(rect->geometry()->vertexCount(), 1);
    QCOMPARE(rect->geometry()->vertexDataAsColoredPoint2D()[0].x, point.x());
    QCOMPARE(rect->geometry()->vertexDataAsColoredPoint2D()[0].y, point.y());
    QCOMPARE(rect->geometry()->vertexDataAsColoredPoint2D()[0].r, expectedColor.red());
    QCOMPARE(rect->geometry()->vertexDataAsColoredPoint2D()[0].g, expectedColor.green());
    QCOMPARE(rect->geometry()->vertexDataAsColoredPoint2D()[0].b, expectedColor.blue());
    QCOMPARE(rect->geometry()->vertexDataAsColoredPoint2D()[0].a, expectedColor.alpha());
}

void TestNodeBasedOverlayCanvas::testWriteLine_data()
{
    QTest::addColumn<precitec::image::LayerType>("layer");
    QTest::addColumn<QPointF>("point");
    QTest::addColumn<QPointF>("endPoint");
    QTest::addColumn<precitec::image::Color>("color");
    QTest::addColumn<QColor>("expectedColor");

    QTest::newRow("Line") << precitec::image::eLayerLine << QPointF(10, 20) << QPointF(30, 40) << precitec::image::Color::Red() << QColor(Qt::red);
    QTest::newRow("Contour") << precitec::image::eLayerContour << QPointF(0, 0) << QPointF(0, 10) << precitec::image::Color::Blue() << QColor(Qt::blue);
    QTest::newRow("Position") << precitec::image::eLayerPosition << QPointF(-10, 20) << QPointF(0, 20) << precitec::image::Color::Green() << QColor(Qt::green);
    QTest::newRow("Text") << precitec::image::eLayerText << QPointF(1, 1) << QPointF(2, 2) << precitec::image::Color::White() << QColor(Qt::white);
    QTest::newRow("Grid") << precitec::image::eLayerGridTransp << QPointF(20, 30) << QPointF(5, 6) << precitec::image::Color::Black() << QColor(Qt::black);
    QTest::newRow("Transparent Line") << precitec::image::eLayerLineTransp << QPointF(0, 0) << QPointF(30, 40) << precitec::image::Color::Yellow() << QColor(Qt::yellow);
    QTest::newRow("Transparent Contour") << precitec::image::eLayerContourTransp << QPointF(0, 0) << QPointF(40, 0) << precitec::image::Color::Cyan() << QColor(Qt::cyan);
    QTest::newRow("Transparent Position") << precitec::image::eLayerPositionTransp << QPointF(0, 0) << QPointF(0, 20) << precitec::image::Color::Orange() << QColor(255, 165, 0);
    QTest::newRow("Transparent Text") << precitec::image::eLayerTextTransp << QPointF(0, 0) << QPointF(10, 20) << precitec::image::Color::Magenta() << QColor(Qt::magenta);
    QTest::newRow("InfoBox") << precitec::image::eLayerInfoBox << QPointF(0, 0) << QPointF(10, 20) << precitec::image::Color::Magenta() << QColor(Qt::magenta);
    QTest::newRow("Image") << precitec::image::eLayerImage << QPointF(0, 0) << QPointF(10, 20) << precitec::image::Color::Magenta() << QColor(Qt::magenta);
}

void TestNodeBasedOverlayCanvas::testWriteLine()
{
    NodeBasedOverlayCanvas canvas;
    NodeBasedOverlayCanvas setCanvas;
    QFETCH(precitec::image::LayerType, layer);
    QFETCH(QPointF, point);
    QFETCH(QPointF, endPoint);
    QFETCH(precitec::image::Color, color);
    setCanvas.getLayer(layer).add<precitec::image::OverlayLine>(point.x(), point.y(), endPoint.x(), endPoint.y(), color);

    QSGNode node;
    QCOMPARE(node.childCount(), 0);

    // let's test with the layer disabled
    canvas.setLayerEnabled(layer, false);
    canvas.sync(&node, setCanvas, QSize{});
    QCOMPARE(node.childCount(), 0);

    // now enable the layer
    canvas.setLayerEnabled(layer, true);
    canvas.sync(&node, setCanvas, QSize{});
    QCOMPARE(node.childCount(), 1);
    auto rect = dynamic_cast<QSGGeometryNode*>(node.childAtIndex(0));
    QVERIFY(rect);
    auto material = dynamic_cast<QSGVertexColorMaterial*>(rect->material());
    QVERIFY(material);
    QFETCH(QColor, expectedColor);

    QCOMPARE(rect->geometry()->drawingMode(), GLenum(GL_LINES));
    QCOMPARE(rect->geometry()->vertexCount(), 2);
    QCOMPARE(rect->geometry()->vertexDataAsColoredPoint2D()[1].x, point.x());
    QCOMPARE(rect->geometry()->vertexDataAsColoredPoint2D()[1].y, point.y());
    QCOMPARE(rect->geometry()->vertexDataAsColoredPoint2D()[1].r, expectedColor.red());
    QCOMPARE(rect->geometry()->vertexDataAsColoredPoint2D()[1].g, expectedColor.green());
    QCOMPARE(rect->geometry()->vertexDataAsColoredPoint2D()[1].b, expectedColor.blue());
    QCOMPARE(rect->geometry()->vertexDataAsColoredPoint2D()[1].a, expectedColor.alpha());
    QCOMPARE(rect->geometry()->vertexDataAsColoredPoint2D()[0].x, endPoint.x());
    QCOMPARE(rect->geometry()->vertexDataAsColoredPoint2D()[0].y, endPoint.y());
    QCOMPARE(rect->geometry()->vertexDataAsColoredPoint2D()[0].r, expectedColor.red());
    QCOMPARE(rect->geometry()->vertexDataAsColoredPoint2D()[0].g, expectedColor.green());
    QCOMPARE(rect->geometry()->vertexDataAsColoredPoint2D()[0].b, expectedColor.blue());
    QCOMPARE(rect->geometry()->vertexDataAsColoredPoint2D()[0].a, expectedColor.alpha());
}

void TestNodeBasedOverlayCanvas::testWriteCircle_data()
{
    QTest::addColumn<precitec::image::LayerType>("layer");
    QTest::addColumn<QPointF>("point");
    QTest::addColumn<int>("radius");
    QTest::addColumn<precitec::image::Color>("color");
    QTest::addColumn<QColor>("expectedColor");

    QTest::newRow("Line") << precitec::image::eLayerLine << QPointF(10, 20) << 1 << precitec::image::Color::Red() << QColor(Qt::red);
    QTest::newRow("Contour") << precitec::image::eLayerContour << QPointF(0, 0) << 2 << precitec::image::Color::Blue() << QColor(Qt::blue);
    QTest::newRow("Position") << precitec::image::eLayerPosition << QPointF(-10, 20) << 3 << precitec::image::Color::Green() << QColor(Qt::green);
    QTest::newRow("Text") << precitec::image::eLayerText << QPointF(1, 1) << 0 << precitec::image::Color::White() << QColor(Qt::white);
    QTest::newRow("Grid") << precitec::image::eLayerGridTransp << QPointF(20, 30) << -5 << precitec::image::Color::Black() << QColor(Qt::black);
    QTest::newRow("Transparent Line") << precitec::image::eLayerLineTransp << QPointF(0, 30) << 8 << precitec::image::Color::Yellow() << QColor(Qt::yellow);
    QTest::newRow("Transparent Contour") << precitec::image::eLayerContourTransp << QPointF(0, 0) << 5 << precitec::image::Color::Cyan() << QColor(Qt::cyan);
    QTest::newRow("Transparent Position") << precitec::image::eLayerPositionTransp << QPointF(0, 0) << 5 << precitec::image::Color::Orange() << QColor(255, 165, 0);
    QTest::newRow("Transparent Text") << precitec::image::eLayerTextTransp << QPointF(0, 0) << 5 << precitec::image::Color::Magenta() << QColor(Qt::magenta);
    QTest::newRow("InfoBox") << precitec::image::eLayerInfoBox << QPointF(0, 0) << 5 << precitec::image::Color::Magenta() << QColor(Qt::magenta);
    QTest::newRow("Image") << precitec::image::eLayerImage << QPointF(0, 0) << 5 << precitec::image::Color::Magenta() << QColor(Qt::magenta);
}

void TestNodeBasedOverlayCanvas::testWriteCircle()
{
    NodeBasedOverlayCanvas canvas;
    QFETCH(precitec::image::LayerType, layer);
    QFETCH(QPointF, point);
    QFETCH(int, radius);
    QFETCH(precitec::image::Color, color);
    canvas.getLayer(layer).add<precitec::image::OverlayCircle>(point.x(), point.y(), radius, color);

    QSGNode node;
    QCOMPARE(node.childCount(), 0);

    // let's test with the layer disabled
    canvas.setLayerEnabled(layer, false);
    canvas.createNodes(&node);
    QCOMPARE(node.childCount(), 0);

    // now enable the layer
    canvas.setLayerEnabled(layer, true);
    canvas.createNodes(&node);
    if (radius <= 0)
    {
        return;
    }
    QCOMPARE(node.childCount(), 1);
    auto rect = dynamic_cast<QSGGeometryNode*>(node.childAtIndex(0));
    QVERIFY(rect);
    auto material = dynamic_cast<QSGFlatColorMaterial*>(rect->material());
    QVERIFY(material);
    QTEST(material->color(), "expectedColor");

    QCOMPARE(rect->geometry()->drawingMode(), GLenum(GL_LINE_LOOP));
    QCOMPARE(rect->geometry()->vertexCount(), 360 * radius);
    QCOMPARE(rect->geometry()->vertexDataAsPoint2D()[0].x, point.x() + radius * std::cos(0));
    QCOMPARE(rect->geometry()->vertexDataAsPoint2D()[0].y, point.y() - radius * std::sin(0));
    // 180 degrees
    QCOMPARE(rect->geometry()->vertexDataAsPoint2D()[180 * radius].x, float(point.x() + radius * std::cos(M_PI)));
    QCOMPARE(rect->geometry()->vertexDataAsPoint2D()[180 * radius].y, float(point.y() - radius * std::sin(float(180 * M_PI / 180))));
    // 90 degrees
    QCOMPARE(rect->geometry()->vertexDataAsPoint2D()[90 * radius].x, float(point.x() + radius * std::cos(float(M_PI/2))));
    QCOMPARE(rect->geometry()->vertexDataAsPoint2D()[90 * radius].y, float(point.y() - radius * std::sin(float(M_PI/2))));
    // 270 degrees
    QCOMPARE(rect->geometry()->vertexDataAsPoint2D()[270 * radius].x, float(point.x() + radius * std::cos(float(3*M_PI/2))));
    QCOMPARE(rect->geometry()->vertexDataAsPoint2D()[270 * radius].y, float(point.y() - radius * std::sin(float(3*M_PI/2))));
}




void TestNodeBasedOverlayCanvas::testWritePixelList_data()
{
    QTest::addColumn<precitec::image::LayerType>("layer");
    QTest::addColumn<QPointF>("position");
    QTest::addColumn<std::vector<int>>("yVector");
    QTest::addColumn<precitec::image::Color>("color");
    QTest::addColumn<QColor>("expectedColor");

    QTest::newRow("Line") << precitec::image::eLayerLine << QPointF(10, 20) << std::vector<int>{{10,1,2}} <<  precitec::image::Color::Red() << QColor(Qt::red);
    QTest::newRow("Contour") << precitec::image::eLayerContour << QPointF(0, 0) <<  std::vector<int>(100, 5)  <<  precitec::image::Color::Blue() << QColor(Qt::blue);
    QTest::newRow("Position") << precitec::image::eLayerPosition << QPointF(-10, 20) <<std::vector<int>{{10,1,2}}<< precitec::image::Color::Green() << QColor(Qt::green);
    QTest::newRow("Text") << precitec::image::eLayerText << QPointF(1, 1) << std::vector<int>{} << precitec::image::Color::White() << QColor(Qt::white);
    QTest::newRow("Grid") << precitec::image::eLayerGridTransp << QPointF(20, 30)  << std::vector<int>{{10,1,2}} << precitec::image::Color::Black() << QColor(Qt::black);
    QTest::newRow("Transparent Line") << precitec::image::eLayerLineTransp << QPointF(0, 0) << std::vector<int>{{10,1,2}} << precitec::image::Color::Yellow() << QColor(Qt::yellow);
    QTest::newRow("Transparent Contour") << precitec::image::eLayerContourTransp << QPointF(0, 0) << std::vector<int>{} << precitec::image::Color::Cyan() << QColor(Qt::cyan);
    QTest::newRow("Transparent Position") << precitec::image::eLayerPositionTransp << QPointF(0, 0) << std::vector<int>{{10,1,2}} << precitec::image::Color::Orange() << QColor(255, 165, 0);
    QTest::newRow("Transparent Text") << precitec::image::eLayerTextTransp << QPointF(0, 0) << std::vector<int>{{10,1,2}} << precitec::image::Color::Magenta() << QColor(Qt::magenta);
    QTest::newRow("InfoBox") << precitec::image::eLayerInfoBox << QPointF(0, 0) << std::vector<int>{{10,1,2}} << precitec::image::Color::Magenta() << QColor(Qt::magenta);
    QTest::newRow("Image") << precitec::image::eLayerImage << QPointF(0, 0) << std::vector<int>{{10,1,2}} << precitec::image::Color::Magenta() << QColor(Qt::magenta);
}

void TestNodeBasedOverlayCanvas::testWritePixelList()
{
    NodeBasedOverlayCanvas canvas;
    NodeBasedOverlayCanvas setCanvas;
    QFETCH(precitec::image::LayerType, layer);
    QFETCH(QPointF, position);
    QFETCH(std::vector<int>, yVector);
    QFETCH(precitec::image::Color, color);
    setCanvas.getLayer(layer).add<precitec::image::OverlayPointList>(precitec::geo2d::Point(position.x(), position.y()), yVector, color);

    //test also vector<double>
    std::vector<double> yVectorDouble;
    for (auto & y : yVector)
    {
        yVectorDouble.push_back(double(y)+0.1);
    }
    setCanvas.getLayer(layer).add<precitec::image::OverlayPointList>(precitec::geo2d::Point(position.x(), position.y()), yVectorDouble, color);

    QSGNode node;
    QCOMPARE(node.childCount(), 0);

    // let's test with the layer disabled
    canvas.setLayerEnabled(layer, false);
    canvas.sync(&node, setCanvas, QSize{});
    QCOMPARE(node.childCount(), 0);

    // now enable the layer
    canvas.setLayerEnabled(layer, true);
    canvas.sync(&node, setCanvas, QSize{});
    QCOMPARE(node.childCount(), 1);
    
    auto rect = dynamic_cast<QSGGeometryNode*>(node.firstChild());
    QVERIFY(rect);
    auto material = dynamic_cast<QSGVertexColorMaterial*>(rect->material());
    QVERIFY(material);
    QFETCH(QColor, expectedColor);

    QCOMPARE(rect->geometry()->drawingMode(), GLenum(GL_POINTS));
    QCOMPARE(rect->geometry()->vertexCount(), yVector.size() * 2);
    const int n = yVector.size();
    for (int j = 0; j < 2; ++j)
    {
        for (int i = 0 ; i < n; ++i)
        {
            auto vertex = rect->geometry()->vertexDataAsColoredPoint2D()[j*n + i];
            QCOMPARE(vertex.x, position.x() + n -1 - i );
            QCOMPARE(vertex.y, position.y() + yVector[n -1 - i]);
            QCOMPARE(vertex.r, expectedColor.red());
            QCOMPARE(vertex.g, expectedColor.green());
            QCOMPARE(vertex.b, expectedColor.blue());
            QCOMPARE(vertex.a, expectedColor.alpha());
        }
    }
    

   
}


QTEST_MAIN(TestNodeBasedOverlayCanvas)

#include "testNodeBasedOverlayCanvas.moc"
