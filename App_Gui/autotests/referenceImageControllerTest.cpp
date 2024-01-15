#include <QTest>
#include <QSignalSpy>

#include "../src/referenceImageController.h"
#include "product.h"
#include "seamSeries.h"
#include "seam.h"

using precitec::gui::ReferenceImageController;
using precitec::storage::Product;

class ReferenceImageControllerTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void testCtor();
    void testReferenceImageDir();
    void testCurrentSeam();
    void testUpdateImagePath();
};

void ReferenceImageControllerTest::testCtor()
{
    ReferenceImageController controller{this};
    QCOMPARE(controller.imagePath(), QStringLiteral(""));
    QCOMPARE(controller.imageFilePath(), QStringLiteral(""));
    QCOMPARE(controller.referenceImageDir(), QStringLiteral(""));
    QVERIFY(!controller.currentSeam());
}

void ReferenceImageControllerTest::testReferenceImageDir()
{
    ReferenceImageController controller{this};
    QCOMPARE(controller.referenceImageDir(), QStringLiteral(""));

    QSignalSpy dirChangedSpy(&controller, &ReferenceImageController::referenceImageDirChanged);
    QVERIFY(dirChangedSpy.isValid());

    QSignalSpy imagePathChangedSpy(&controller, &ReferenceImageController::imagePathChanged);
    QVERIFY(imagePathChangedSpy.isValid());

    QSignalSpy imageFilePathChangedSpy(&controller, &ReferenceImageController::imageFilePathChanged);
    QVERIFY(imageFilePathChangedSpy.isValid());

    controller.setReferenceImageDir(QStringLiteral(""));
    QCOMPARE(dirChangedSpy.count(), 0);
    QCOMPARE(imagePathChangedSpy.count(), 0);
    QCOMPARE(imageFilePathChangedSpy.count(), 0);

    QTemporaryDir dir;
    controller.setReferenceImageDir(dir.path());
    QCOMPARE(controller.referenceImageDir(), dir.path());
    QCOMPARE(dirChangedSpy.count(), 1);
    QCOMPARE(imagePathChangedSpy.count(), 0);
    QCOMPARE(imageFilePathChangedSpy.count(), 0);

    controller.setReferenceImageDir(dir.path());
    QCOMPARE(dirChangedSpy.count(), 1);
    QCOMPARE(imagePathChangedSpy.count(), 0);
    QCOMPARE(imageFilePathChangedSpy.count(), 0);
}

void ReferenceImageControllerTest::testCurrentSeam()
{
    ReferenceImageController controller{this};

    QSignalSpy seamChangedSpy(&controller, &ReferenceImageController::currentSeamChanged);
    QVERIFY(seamChangedSpy.isValid());

    QSignalSpy imagePathChangedSpy(&controller, &ReferenceImageController::imagePathChanged);
    QVERIFY(imagePathChangedSpy.isValid());

    QSignalSpy imageFilePathChangedSpy(&controller, &ReferenceImageController::imageFilePathChanged);
    QVERIFY(imageFilePathChangedSpy.isValid());

    Product product{QUuid{"54c9f453-9f6e-40d4-9e95-713fbce4c258"}};
    auto series = product.createSeamSeries();
    QVERIFY(series);
    series->setNumber(3);
    auto seam = product.createSeam();
    QVERIFY(seam);
    seam->setNumber(2);

    QCOMPARE(controller.currentSeam(), nullptr);

    controller.setCurrentSeam(seam);
    QCOMPARE(controller.currentSeam(), seam);
    QCOMPARE(seamChangedSpy.count(), 1);
    QCOMPARE(imagePathChangedSpy.count(), 0);
    QCOMPARE(imageFilePathChangedSpy.count(), 0);

    controller.setCurrentSeam(seam);
    QCOMPARE(seamChangedSpy.count(), 1);
    QCOMPARE(imagePathChangedSpy.count(), 0);
    QCOMPARE(imageFilePathChangedSpy.count(), 0);

    seam->deleteLater();
    QVERIFY(seamChangedSpy.wait());
    QCOMPARE(controller.currentSeam(), nullptr);
    QCOMPARE(seamChangedSpy.count(), 2);
    QCOMPARE(imagePathChangedSpy.count(), 0);
    QCOMPARE(imageFilePathChangedSpy.count(), 0);
}

void ReferenceImageControllerTest::testUpdateImagePath()
{
    ReferenceImageController controller{this};

    QSignalSpy imagePathChangedSpy(&controller, &ReferenceImageController::imagePathChanged);
    QVERIFY(imagePathChangedSpy.isValid());

    QSignalSpy imageFilePathChangedSpy(&controller, &ReferenceImageController::imageFilePathChanged);
    QVERIFY(imageFilePathChangedSpy.isValid());

    QTemporaryDir dir;
    controller.setReferenceImageDir(dir.path());
    QCOMPARE(imagePathChangedSpy.count(), 0);
    QCOMPARE(imageFilePathChangedSpy.count(), 0);

    Product product{QUuid{"54c9f453-9f6e-40d4-9e95-713fbce4c258"}};
    auto series = product.createSeamSeries();
    QVERIFY(series);
    series->setNumber(3);
    auto seam = product.createSeam();
    QVERIFY(seam);
    seam->setNumber(2);

    controller.setCurrentSeam(seam);
    QCOMPARE(imagePathChangedSpy.count(), 1);
    QCOMPARE(imageFilePathChangedSpy.count(), 1);

    auto imagePath = dir.path().append(QStringLiteral("/54c9f453-9f6e-40d4-9e95-713fbce4c258/seam_series0003/seam0002/"));
    QCOMPARE(controller.imagePath(), imagePath);

    QVERIFY(QDir{imagePath}.exists());

    QCOMPARE(controller.imageFilePath(), QStringLiteral(""));
}

QTEST_GUILESS_MAIN(ReferenceImageControllerTest)
#include "referenceImageControllerTest.moc"

