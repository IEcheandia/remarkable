#include <QTest>
#include <QSignalSpy>
#include <QQuickWindow>

#include "../imageNode.h"
#include "image/image.h"

using precitec::gui::components::image::ImageNode;

class TestImageNode : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testCtor();
    void testSetWindow();
    void testImage();
    void testImageNoWindow();
    void testDummyImage_data();
    void testDummyImage();
};

void TestImageNode::testCtor()
{
    ImageNode node;
    QCOMPARE(node.ownsTexture(), false);
    QCOMPARE(node.filtering(), QSGTexture::Linear);
    QVERIFY(node.m_window == nullptr);
}

void TestImageNode::testSetWindow()
{
    ImageNode node;
    QVERIFY(node.m_window == nullptr);
    QQuickWindow w;
    node.setWindow(&w);
    QCOMPARE(node.m_window, &w);
}

void TestImageNode::testImage()
{
    QQuickWindow w;
    QSignalSpy sgInitializedSpy(&w, &QQuickWindow::sceneGraphInitialized);
    QVERIFY(sgInitializedSpy.isValid());
    w.show();
    QVERIFY(sgInitializedSpy.wait());
    ImageNode node;
    node.setWindow(&w);
    const auto grayImage = precitec::image::genModuloPattern(precitec::geo2d::Size(20, 30), 5);
    node.setImage(grayImage);
    QCOMPARE(node.m_image.size(), QSize(20, 30));
    QCOMPARE(node.m_image.format(), QImage::Format_Grayscale8);
    QVERIFY(node.texture() != nullptr);
    QCOMPARE(node.texture()->textureSize(), QSize(20, 30));
    QCOMPARE(node.texture()->hasAlphaChannel(), false);
}

void TestImageNode::testImageNoWindow()
{
    ImageNode node;
    const auto grayImage = precitec::image::genModuloPattern(precitec::geo2d::Size(20, 30), 5);
    node.setImage(grayImage);
    QVERIFY(node.texture() == nullptr);
    QCOMPARE(node.m_image.isNull(), true);
}

void TestImageNode::testDummyImage_data()
{
    QTest::addColumn<QSize>("size");

    QTest::newRow("defaultValue") << QSize(1024, 1024);
    QTest::newRow("not default value") << QSize(20, 30);
}

void TestImageNode::testDummyImage()
{
    QQuickWindow w;
    QSignalSpy sgInitializedSpy(&w, &QQuickWindow::sceneGraphInitialized);
    QVERIFY(sgInitializedSpy.isValid());
    w.show();
    QVERIFY(sgInitializedSpy.wait());
    ImageNode node;
    node.setWindow(&w);
    node.setEnabled(false);

    QFETCH(QSize, size);
    const auto grayImage = precitec::image::genModuloPattern(precitec::geo2d::Size(size.width(), size.height()), 5);
    node.setImage(grayImage);
    QCOMPARE(node.m_dummyImage.size(), size);
    QCOMPARE(node.m_dummyImage.format(), QImage::Format_Grayscale8);
    QVERIFY(node.texture() != nullptr);
    QCOMPARE(node.texture()->textureSize(), size);
    QCOMPARE(node.texture()->hasAlphaChannel(), false);
}

QTEST_MAIN(TestImageNode)
#include "testImageNode.moc"
