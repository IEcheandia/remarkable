#include <QTest>
#include <QSignalSpy>
#include <QQuickWindow>
#include <QSGTransformNode>

#include "../imageItem.h"
#include "../overlayGroupModel.h"
#include "../nodeBasedOverlayCanvas.h"

#include "image/image.h"
#include "overlay/overlayPrimitive.h"

Q_DECLARE_METATYPE(precitec::gui::components::image::OverlayGroupModel::OverlayGroup)
Q_DECLARE_METATYPE(precitec::image::LayerType)

using precitec::gui::components::image::ImageItem;
using precitec::gui::components::image::OverlayGroupModel;

class TestImageItem : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testZoom_data();
    void testZoom();
    void testImageCentering_data();
    void testImageCentering();
    void testEnableLayers_data();
    void testEnableLayers();
    void testAvailableLayers_data();
    void testAvailableLayers();
    void testUpdatePaintNode();
};

void TestImageItem::testZoom_data()
{
    QTest::addColumn<qreal>("zoom");
    QTest::addColumn<qreal>("expectedZoom");
    QTest::addColumn<bool>("changed");

    QTest::newRow("minZoom") << ImageItem::minZoom() << ImageItem::minZoom() << true;
    QTest::newRow("maxZoom") << ImageItem::maxZoom() << ImageItem::maxZoom() << true;
    QTest::newRow("default") << 1.0 << 1.0 << false;
    QTest::newRow("below min") << ImageItem::minZoom() - 1 << ImageItem::minZoom() << true;
    QTest::newRow("above max") << ImageItem::maxZoom() + 1 << ImageItem::maxZoom() << true;
}

void TestImageItem::testZoom()
{
    ImageItem item;
    QCOMPARE(item.zoom(), 1.0);
    QMatrix4x4 matrix;
    QCOMPARE(item.m_transformation, matrix);
    QSignalSpy zoomChangedSpy(&item, &ImageItem::zoomChanged);
    QVERIFY(zoomChangedSpy.isValid());

    QFETCH(qreal, zoom);
    item.setZoom(zoom);
    QTEST(!zoomChangedSpy.isEmpty(), "changed");
    QTEST(item.zoom(), "expectedZoom");
    matrix.scale(item.zoom());
    QCOMPARE(item.m_transformation, matrix);
}

void TestImageItem::testImageCentering_data()
{
    QTest::addColumn<QSizeF>("itemSize");
    QTest::addColumn<QSize>("imageSize");

    QTest::newRow("double") << QSizeF(100, 100) << QSize(50, 50);
    QTest::newRow("same size") << QSizeF(100, 200) << QSize(100, 200);
    QTest::newRow("larger") << QSizeF(10, 20) << QSize(20, 40);
}

void TestImageItem::testImageCentering()
{
    ImageItem item;
    QMatrix4x4 matrix;
    QCOMPARE(item.m_transformation, matrix);
    QFETCH(QSizeF, itemSize);
    item.setWidth(itemSize.width());
    item.setHeight(itemSize.height());
    // unchanged as the imageData is not yet set.
    QCOMPARE(item.m_transformation, matrix);
    QCOMPARE(item.width(), itemSize.width());
    QCOMPARE(item.height(), itemSize.height());

    QFETCH(QSize, imageSize);
    const auto image = precitec::image::genModuloPattern(precitec::geo2d::Size(imageSize.width(), imageSize.height()), 5);
    QSignalSpy imageDataChangedSpy(&item, &ImageItem::imageDataChanged);
    QVERIFY(imageDataChangedSpy.isValid());
    auto model = item.m_overlayGroupModel;
    QCOMPARE(model->data(model->index(OverlayGroupModel::OverlayGroup::LiveImage), Qt::UserRole).toBool(), false);
    const auto imageData = std::make_tuple(precitec::interface::ImageContext(), image, precitec::image::OverlayCanvas());
    item.setImageData(imageData);
    QCOMPARE(imageDataChangedSpy.count(), 1);
    QCOMPARE(item.imageSize(), QSizeF(imageSize));
    QVERIFY(item.imageSize().isValid());
    matrix.translate((itemSize.width() - imageSize.width()) * 0.5, (itemSize.height() - imageSize.height()) * 0.5);
    // delayed update
    QTRY_VERIFY(qFuzzyCompare(item.m_transformation, matrix));
    QTRY_COMPARE(model->data(model->index(OverlayGroupModel::OverlayGroup::LiveImage), Qt::UserRole).toBool(), true);

    // let's change the image width
    item.setWidth(itemSize.width() * 2);
    QCOMPARE(item.width(), itemSize.width() * 2);
    item.setZoom(1.0);
    matrix = QMatrix4x4{};
    matrix.translate((itemSize.width() * 2 - imageSize.width()) * 0.5, (itemSize.height() - imageSize.height()) * 0.5);
    QVERIFY(qFuzzyCompare(item.m_transformation, matrix));
    // and the height
    item.setHeight(itemSize.height() * 2);
    QCOMPARE(item.height(), itemSize.height() * 2);
    item.setZoom(1.0);
    matrix = QMatrix4x4{};
    matrix.translate((itemSize.width() * 2 - imageSize.width()) * 0.5, (itemSize.height() * 2 - imageSize.height()) * 0.5);
    QVERIFY(qFuzzyCompare(item.m_transformation, matrix));

    // and zoom
    item.setZoom(2);
    matrix = QMatrix4x4{};
    matrix.translate((itemSize.width() - imageSize.width()), (itemSize.height() - imageSize.height()));
    matrix.scale(2);
    QVERIFY(qFuzzyCompare(item.m_transformation, matrix));
}

void TestImageItem::testEnableLayers_data()
{
    QTest::addColumn<OverlayGroupModel::OverlayGroup>("overlayGroup");

    QTest::newRow("Line")      << OverlayGroupModel::OverlayGroup::Line;
    QTest::newRow("Contour")   << OverlayGroupModel::OverlayGroup::Contour;
    QTest::newRow("Position")  << OverlayGroupModel::OverlayGroup::Position;
    QTest::newRow("Text")      << OverlayGroupModel::OverlayGroup::Text;
    QTest::newRow("Grid")      << OverlayGroupModel::OverlayGroup::Grid;
    QTest::newRow("Image")     << OverlayGroupModel::OverlayGroup::Image;
    QTest::newRow("LiveImage") << OverlayGroupModel::OverlayGroup::LiveImage;
}

void TestImageItem::testEnableLayers()
{
    ImageItem item;
    auto model = item.m_overlayGroupModel;
    QFETCH(OverlayGroupModel::OverlayGroup, overlayGroup);
    const auto index = model->index(overlayGroup);
    QVERIFY(index.isValid());
    QCOMPARE(model->data(index, Qt::UserRole + 1).toBool(), true);
    const auto layers = model->layers(index);
    for (auto layer : layers)
    {
        QCOMPARE(item.m_overlayCanvas->isLayerEnabled(layer), true);
    }
    model->setData(index, false, Qt::UserRole + 1);
    for (auto layer : layers)
    {
        QCOMPARE(item.m_overlayCanvas->isLayerEnabled(layer), false);
    }
}

void TestImageItem::testAvailableLayers_data()
{
    QTest::addColumn<OverlayGroupModel::OverlayGroup>("overlayGroup");
    QTest::addColumn<precitec::image::LayerType>("layer");

    QTest::newRow("Line")      << OverlayGroupModel::OverlayGroup::Line << precitec::image::eLayerLine;
    QTest::newRow("Line transparent")      << OverlayGroupModel::OverlayGroup::Line << precitec::image::eLayerLineTransp;
    QTest::newRow("Contour")   << OverlayGroupModel::OverlayGroup::Contour << precitec::image::eLayerContour;
    QTest::newRow("Contour Transparent")   << OverlayGroupModel::OverlayGroup::Contour << precitec::image::eLayerContourTransp;
    QTest::newRow("Position")  << OverlayGroupModel::OverlayGroup::Position << precitec::image::eLayerPosition;
    QTest::newRow("Position Transparent")  << OverlayGroupModel::OverlayGroup::Position << precitec::image::eLayerPositionTransp;
    QTest::newRow("Text")      << OverlayGroupModel::OverlayGroup::Text << precitec::image::eLayerText;
    QTest::newRow("Text transparent")      << OverlayGroupModel::OverlayGroup::Text << precitec::image::eLayerTextTransp;
    QTest::newRow("Grid")      << OverlayGroupModel::OverlayGroup::Grid << precitec::image::eLayerGridTransp;
    QTest::newRow("Image")     << OverlayGroupModel::OverlayGroup::Image << precitec::image::eLayerImage;
}

void TestImageItem::testAvailableLayers()
{
    ImageItem item;
    auto model = item.m_overlayGroupModel;
    QFETCH(OverlayGroupModel::OverlayGroup, overlayGroup);
    const auto index = model->index(overlayGroup);
    QCOMPARE(model->data(index, Qt::UserRole).toBool(), false);

    QSignalSpy dataChangedSpy(model, &OverlayGroupModel::dataChanged);
    QVERIFY(dataChangedSpy.isValid());

    const auto image = precitec::image::genModuloPattern(precitec::geo2d::Size(10, 20), 5);
    auto canvas = precitec::image::OverlayCanvas();
    QFETCH(precitec::image::LayerType, layer);
    canvas.getLayer(layer).add<precitec::image::OverlayPoint>(1, 2, precitec::image::Color::Red());
    item.setImageData(std::make_tuple(precitec::interface::ImageContext(), image, canvas));
    QVERIFY(dataChangedSpy.wait());
    QCOMPARE(model->data(index, Qt::UserRole).toBool(), true);
    QCOMPARE(model->data(model->index(OverlayGroupModel::OverlayGroup::LiveImage), Qt::UserRole).toBool(), true);
    // all other should be false
    for (int i = 0; i < model->rowCount(QModelIndex()) - 1; i++)
    {
        const auto testIndex = model->index(i, 0, QModelIndex());
        if (testIndex == index)
        {
            continue;
        }
        QCOMPARE(model->data(testIndex, Qt::UserRole).toBool(), false);
    }
}

void TestImageItem::testUpdatePaintNode()
{
    ImageItem item;
    auto model = item.m_overlayGroupModel;
    const auto image = precitec::image::genModuloPattern(precitec::geo2d::Size(10, 20), 5);
    auto canvas = precitec::image::OverlayCanvas();
    canvas.getLayer(precitec::image::eLayerImage).add<precitec::image::OverlayPoint>(1, 2, precitec::image::Color::Red());

    QQuickWindow w;
    QSignalSpy sgInitializedSpy(&w, &QQuickWindow::sceneGraphInitialized);
    QVERIFY(sgInitializedSpy.isValid());
    w.show();
    QVERIFY(sgInitializedSpy.wait());
    item.setParentItem(w.contentItem());
    QCOMPARE(item.window(), &w);

    QSignalSpy dataChangedSpy(model, &OverlayGroupModel::dataChanged);
    QVERIFY(dataChangedSpy.isValid());
    item.setImageData(std::make_tuple(precitec::interface::ImageContext(), image, canvas));
    QVERIFY(dataChangedSpy.wait());

    auto node = dynamic_cast<QSGTransformNode*>(item.updatePaintNode(nullptr, nullptr));
    QVERIFY(node);
    QCOMPARE(node->childCount(), 2);

    // let's disable live image
    model->setData(model->index(OverlayGroupModel::OverlayGroup::LiveImage), false, Qt::UserRole + 1);
    QCOMPARE(model->data(model->index(OverlayGroupModel::OverlayGroup::LiveImage), Qt::UserRole + 1).toBool(), false);

    node = dynamic_cast<QSGTransformNode*>(item.updatePaintNode(nullptr, nullptr));
    QVERIFY(node);
    QCOMPARE(node->childCount(), 2);
    QMatrix4x4 matrix;
    matrix.translate(-5, -10);
    QCOMPARE(node->matrix(), matrix);
    auto clipNode = dynamic_cast<QSGClipNode*>(node->childAtIndex(1));
    QVERIFY(clipNode);
    QCOMPARE(clipNode->clipRect(), QRectF(0, 0, 10, 20));
}

QTEST_MAIN(TestImageItem)

#include "testImageItem.moc"
