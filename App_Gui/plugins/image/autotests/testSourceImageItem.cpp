#include <QTest>
#include <QSignalSpy>
#include <QQuickWindow>
#include <QSGTransformNode>
#include <QSGImageNode>

#include "../sourceImageItem.h"

using precitec::gui::components::image::SourceImageItem;

class TestSourceImageItem : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testCtor();
    void testImage();
    void testZoom();
    void testPanning();
    void testUpdateTransformation();
    void testPaintedRect();
    void testZoomToFit();
    void testMapToPaintedImage();
    void testMapFromPaintedImage();
    void testCenterAndFitTo();
    void testUpdatePaintNode();
    void testThumbnail();
};

void TestSourceImageItem::testCtor()
{
    SourceImageItem item;

    QVERIFY(item.source().isNull());
    QVERIFY(item.m_image.isNull());
    QVERIFY(!item.imageValid());
    QVERIFY(item.transformation().isIdentity());
    QCOMPARE(item.zoom(), 1.0);
    QCOMPARE(item.panning(), QPointF(0, 0));
    QCOMPARE(item.imageSize(), QSize{});
    QCOMPARE(item.m_image.size(), QSize(0, 0));
    QCOMPARE(item.isLoading(), false);
    QCOMPARE(item.thumbnail(), QSize{});
    QCOMPARE(item.thumbnailOriginalSize(), QSize{});
}

void TestSourceImageItem::testImage()
{
    SourceImageItem item;
    QSignalSpy loadingChangedSpy{&item, &SourceImageItem::loadingChanged};
    QVERIFY(loadingChangedSpy.isValid());

    QVERIFY(item.source().isNull());
    QVERIFY(item.m_image.isNull());
    QVERIFY(!item.imageValid());
    QCOMPARE(item.imageSize(), QSize{});
    QCOMPARE(item.m_image.size(), QSize(0, 0));

    QSignalSpy sourceChangedSpy(&item, &SourceImageItem::sourceChanged);
    QVERIFY(sourceChangedSpy.isValid());

    QSignalSpy imageChangedSpy(&item, &SourceImageItem::imageChanged);
    QVERIFY(imageChangedSpy.isValid());

    item.setSource(QStringLiteral(""));
    QCOMPARE(sourceChangedSpy.count(), 0);

    QDir dir{QFINDTESTDATA("testdata/test_image_7168x4096.png")};
    item.setSource(dir.absolutePath());
    QCOMPARE(item.source(), dir.absolutePath());
    QCOMPARE(sourceChangedSpy.count(), 1);

    QVERIFY(imageChangedSpy.wait());
    QCOMPARE(imageChangedSpy.count(), 1);
    QCOMPARE(loadingChangedSpy.count(), 2);
    QVERIFY(item.imageValid());
    QCOMPARE(item.imageSize(), QSize(7168, 4096));
    QCOMPARE(item.m_image, QImage(dir.absolutePath()));

    item.setSource(QStringLiteral("dummy"));
    QCOMPARE(item.source(), QStringLiteral("dummy"));
    QCOMPARE(sourceChangedSpy.count(), 2);

    QVERIFY(imageChangedSpy.wait());
    QCOMPARE(imageChangedSpy.count(), 2);
    QVERIFY(!item.imageValid());
    QCOMPARE(item.m_image, QImage());
}

void TestSourceImageItem::testZoom()
{
    SourceImageItem item;

    QCOMPARE(item.zoom(), 1.0);

    QSignalSpy zoomChangedSpy(&item, &SourceImageItem::zoomChanged);
    QVERIFY(zoomChangedSpy.isValid());

    item.setZoom(1.0);
    QCOMPARE(zoomChangedSpy.count(), 0);

    item.setZoom(0.0);
    QCOMPARE(item.zoom(), 0.1);
    QCOMPARE(zoomChangedSpy.count(), 1);

    item.setZoom(6.0);
    QCOMPARE(item.zoom(), 5.0);
    QCOMPARE(zoomChangedSpy.count(), 2);

    item.setZoom(3.0);
    QCOMPARE(item.zoom(), 3.0);
    QCOMPARE(zoomChangedSpy.count(), 3);
}

void TestSourceImageItem::testPanning()
{
    SourceImageItem item;

    QCOMPARE(item.panning(), QPointF(0.0, 0.0));

    QSignalSpy panningChangedSpy(&item, &SourceImageItem::panningChanged);
    QVERIFY(panningChangedSpy.isValid());

    item.setPanning(QPointF(0.0, 0.0));
    QCOMPARE(panningChangedSpy.count(), 0);

    item.setPanning(QPointF(0.0, 2.0));
    QCOMPARE(item.panning(), QPointF(0.0, 2.0));
    QCOMPARE(panningChangedSpy.count(), 1);

    item.setPanning(QPointF(3.0, 2.0));
    QCOMPARE(item.panning(), QPointF(3.0, 2.0));
    QCOMPARE(panningChangedSpy.count(), 2);

    item.setPanning(QPointF(3.0, 2.0));
    QCOMPARE(panningChangedSpy.count(), 2);
}

void TestSourceImageItem::testUpdateTransformation()
{
    SourceImageItem item;
    QMatrix4x4 matrix;

    QSignalSpy imageChangedSpy(&item, &SourceImageItem::imageChanged);
    QVERIFY(imageChangedSpy.isValid());

    QDir dir{QFINDTESTDATA("testdata/test_image_7168x4096.png")};
    item.setSource(dir.absolutePath());

    QVERIFY(imageChangedSpy.wait());
    QCOMPARE(imageChangedSpy.count(), 1);

    QCOMPARE(item.transformation(), matrix);

    QSignalSpy transformationChangedSpy(&item, &SourceImageItem::transformationChanged);
    QVERIFY(transformationChangedSpy.isValid());

    matrix.scale(3.0);

    item.setZoom(3.0);
    QCOMPARE(transformationChangedSpy.count(), 1);
    QCOMPARE(item.transformation(), matrix);

    matrix.setToIdentity();
    matrix.translate(2.0, 3.5);
    matrix.scale(3.0);

    item.setPanning(QPointF{2.0, 3.5});
    QCOMPARE(transformationChangedSpy.count(), 2);
    QCOMPARE(item.transformation(), matrix);

    matrix.setToIdentity();
    matrix.translate((200 - 7168 * 3) * 0.5, (400 - 4096 * 3) * 0.5);
    matrix.translate(2.0, 3.5);
    matrix.scale(3.0);

    item.setSize({200, 400});
    QCOMPARE(transformationChangedSpy.count(), 4);
    QCOMPARE(item.transformation(), matrix);
}

void TestSourceImageItem::testPaintedRect()
{
    SourceImageItem item;

    QSignalSpy imageChangedSpy(&item, &SourceImageItem::imageChanged);
    QVERIFY(imageChangedSpy.isValid());

    QDir dir{QFINDTESTDATA("testdata/test_image_7168x4096.png")};
    item.setSource(dir.absolutePath());

    QVERIFY(imageChangedSpy.wait());
    QCOMPARE(imageChangedSpy.count(), 1);

    QCOMPARE(item.paintedRect(), QRect(0, 0, 7168, 4096));

    QSignalSpy transformationChangedSpy(&item, &SourceImageItem::transformationChanged);
    QVERIFY(transformationChangedSpy.isValid());

    item.setZoom(3.0);
    item.setPanning(QPointF{2.0, 3.5});
    item.setSize({200, 400});
    QCOMPARE(transformationChangedSpy.count(), 4);

    QCOMPARE(item.paintedRect(), QRectF((200 - 7168 * 3) * 0.5 + 2, (400 - 4096 * 3) * 0.5 + 3.5, 7168 * 3, 4096 * 3));
}

void TestSourceImageItem::testZoomToFit()
{
    SourceImageItem item;

    QSignalSpy imageChangedSpy(&item, &SourceImageItem::imageChanged);
    QVERIFY(imageChangedSpy.isValid());

    QDir dir{QFINDTESTDATA("testdata/test_image_7168x4096.png")};
    item.setSource(dir.absolutePath());

    QVERIFY(imageChangedSpy.wait());
    QCOMPARE(imageChangedSpy.count(), 1);

    QCOMPARE(item.zoom(), 1.0);
    QCOMPARE(item.panning(), QPointF(0.0, 0.0));

    QSignalSpy transformationChangedSpy(&item, &SourceImageItem::transformationChanged);
    QVERIFY(transformationChangedSpy.isValid());

    item.zoomToFit();
    QCOMPARE(transformationChangedSpy.count(), 0);

    item.setSize({1024, 1024});
    QCOMPARE(transformationChangedSpy.count(), 2);

    item.setZoom(7);
    QCOMPARE(transformationChangedSpy.count(), 3);

    item.setPanning(QPointF(16.0, 3.0));
    QCOMPARE(transformationChangedSpy.count(), 4);

    item.zoomToFit();
    QCOMPARE(transformationChangedSpy.count(), 6);
    QCOMPARE(item.zoom(), 1024.0 / 7168.0);
    QCOMPARE(item.panning(), QPointF(0.0, 0.0));
}

void TestSourceImageItem::testMapToPaintedImage()
{
    SourceImageItem item;

    QCOMPARE(item.mapToPaintedImage(QPointF(3, 4)), QPointF(3, 4));

    item.setZoom(2);

    QCOMPARE(item.mapToPaintedImage(QPointF(3, 4)), QPointF(6, 8));

    item.setSize({200, 400});

    QCOMPARE(item.mapToPaintedImage(QPointF(3, 4)), QPointF(6, 8));

    item.setPanning({300, 200});

    QCOMPARE(item.mapToPaintedImage(QPointF(3, 4)), QPointF(6, 8));
}

void TestSourceImageItem::testMapFromPaintedImage()
{
    SourceImageItem item;

    QCOMPARE(item.mapFromPaintedImage(QPointF(3, 4)), QPointF(3, 4));

    item.setZoom(2);

    QCOMPARE(item.mapFromPaintedImage(QPointF(3, 4)), QPointF(1.5, 2));

    item.setSize({200, 400});

    QCOMPARE(item.mapFromPaintedImage(QPointF(3, 4)), QPointF(1.5, 2));

    item.setPanning({300, 200});

    QCOMPARE(item.mapFromPaintedImage(QPointF(3, 4)), QPointF(1.5, 2));
}

void TestSourceImageItem::testCenterAndFitTo()
{
    SourceImageItem item;

    QSignalSpy imageChangedSpy(&item, &SourceImageItem::imageChanged);
    QVERIFY(imageChangedSpy.isValid());

    QDir dir{QFINDTESTDATA("testdata/test_image_7168x4096.png")};
    item.setSource(dir.absolutePath());

    QVERIFY(imageChangedSpy.wait());
    QCOMPARE(imageChangedSpy.count(), 1);

    QCOMPARE(item.zoom(), 1.0);
    QCOMPARE(item.panning(), QPointF(0.0, 0.0));

    QSignalSpy transformationChangedSpy(&item, &SourceImageItem::transformationChanged);
    QVERIFY(transformationChangedSpy.isValid());

    item.zoomToFit();
    QCOMPARE(transformationChangedSpy.count(), 0);

    item.setSize({1024, 1024});
    QCOMPARE(transformationChangedSpy.count(), 2);

    item.setZoom(7);
    QCOMPARE(transformationChangedSpy.count(), 3);

    item.setPanning(QPointF(16.0, 3.0));
    QCOMPARE(transformationChangedSpy.count(), 4);

    item.centerAndFitTo(QPointF(100, 400), QSize(256, 128));
    QCOMPARE(transformationChangedSpy.count(), 6);
    QCOMPARE(item.zoom(), 1024.0 / 256.0);
    QCOMPARE(item.panning(), (-(1024.0 / 256.0) * QPointF(100.0 - 3584.0, 400.0 - 2048.0)));
}

void TestSourceImageItem::testUpdatePaintNode()
{
    SourceImageItem item;

    QQuickWindow window;
    QSignalSpy sgInitializedSpy(&window, &QQuickWindow::sceneGraphInitialized);
    QVERIFY(sgInitializedSpy.isValid());
    window.show();
    QVERIFY(sgInitializedSpy.wait());
    item.setParentItem(window.contentItem());
    QCOMPARE(item.window(), &window);

    QSignalSpy transformationChangedSpy(&item, &SourceImageItem::transformationChanged);
    QVERIFY(transformationChangedSpy.isValid());

    item.setSize({1024, 1024});
    QCOMPARE(transformationChangedSpy.count(), 2);

    QSignalSpy imageChangedSpy(&item, &SourceImageItem::imageChanged);
    QVERIFY(imageChangedSpy.isValid());

    QDir dir{QFINDTESTDATA("testdata/test_image_7168x4096.png")};
    item.setSource(dir.absolutePath());

    QVERIFY(imageChangedSpy.wait());
    QCOMPARE(imageChangedSpy.count(), 1);
    item.zoomToFit();

    QCOMPARE(item.zoom(), 1024.0 / 7168.0);
    QCOMPARE(item.panning(), QPointF(0.0, 0.0));

    auto node = dynamic_cast<QSGTransformNode*>(item.updatePaintNode(nullptr, nullptr));
    QVERIFY(node);
    QCOMPARE(node->childCount(), 2);

    QMatrix4x4 matrix;
    matrix.setToIdentity();
    matrix.translate((1024 - 7168 * (1024.0 / 7168.0)) * 0.5, (1024 - 4096 * (1024.0 / 7168.0)) * 0.5);
    matrix.translate(0.0, 0.0);
    matrix.scale(1024.0 / 7168.0);
    QCOMPARE(node->matrix(), matrix);

    auto imgNode = dynamic_cast<QSGImageNode*>(node->childAtIndex(0));
    QVERIFY(imgNode);
    QVERIFY(imgNode->texture());
    QCOMPARE(imgNode->rect(), QRectF(0, 0, 7168, 4096));

    auto clipNode = dynamic_cast<QSGClipNode*>(node->childAtIndex(1));
    QVERIFY(clipNode);
    QCOMPARE(clipNode->clipRect(), QRectF(0, 0, 7168, 4096));
}

void TestSourceImageItem::testThumbnail()
{
    SourceImageItem item;
    QSignalSpy thumbnailChangedSpy{&item, &SourceImageItem::thumbnailChanged};
    QVERIFY(thumbnailChangedSpy.isValid());
    QSignalSpy thumbnailOriginalSizeChangedSpy{&item, &SourceImageItem::thumbnailOriginalSizeChanged};
    QVERIFY(thumbnailOriginalSizeChangedSpy.isValid());

    item.setThumbnail({7168, 4096});
    QCOMPARE(thumbnailChangedSpy.count(), 1);
    QVERIFY(thumbnailOriginalSizeChangedSpy.isEmpty());
    QCOMPARE(item.thumbnail(), QSize(7168, 4096));
    item.setThumbnailOriginalSize({7168 * 2, 4096 * 2});
    QCOMPARE(item.thumbnailOriginalSize(), QSize(7168 * 2, 4096 * 2));
    QCOMPARE(thumbnailChangedSpy.count(), 1);
    QCOMPARE(thumbnailOriginalSizeChangedSpy.count(), 1);

    QQuickWindow window;
    QSignalSpy sgInitializedSpy(&window, &QQuickWindow::sceneGraphInitialized);
    QVERIFY(sgInitializedSpy.isValid());
    window.show();
    QVERIFY(sgInitializedSpy.wait());
    item.setParentItem(window.contentItem());
    QCOMPARE(item.window(), &window);

    QSignalSpy transformationChangedSpy(&item, &SourceImageItem::transformationChanged);
    QVERIFY(transformationChangedSpy.isValid());

    item.setSize({1024, 1024});
    QCOMPARE(transformationChangedSpy.count(), 2);

    QSignalSpy imageChangedSpy(&item, &SourceImageItem::imageChanged);
    QVERIFY(imageChangedSpy.isValid());

    QFileInfo file{QFINDTESTDATA("testdata/test_image_7168x4096.png")};
    item.setSource(file.absolutePath() + QStringLiteral("/test_image.png"));

    QVERIFY(imageChangedSpy.wait());
    QCOMPARE(imageChangedSpy.count(), 1);
    QCOMPARE(item.imageSize(), item.thumbnailOriginalSize());

    auto node = dynamic_cast<QSGTransformNode*>(item.updatePaintNode(nullptr, nullptr));
    QVERIFY(node);
    QCOMPARE(node->childCount(), 2);

    auto imgNode = dynamic_cast<QSGImageNode*>(node->childAtIndex(0));
    QVERIFY(imgNode);
    QVERIFY(imgNode->texture());
    QCOMPARE(imgNode->rect(), QRectF(0, 0, 7168 * 2, 4096 * 2));

    auto clipNode = dynamic_cast<QSGClipNode*>(node->childAtIndex(1));
    QVERIFY(clipNode);
    QCOMPARE(clipNode->clipRect(), QRectF(0, 0, 7168 * 2, 4096 * 2));
}

QTEST_MAIN(TestSourceImageItem)

#include "testSourceImageItem.moc"

