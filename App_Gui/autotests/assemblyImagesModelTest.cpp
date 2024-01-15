#include <QTest>
#include <QSignalSpy>
#include <QImage>

#include "../src/assemblyImagesModel.h"

using precitec::gui::AssemblyImagesModel;

class AssemblyImagesModelTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testCtor();
    void testNoFiles();
    void testImageFiles();
    void testImagesAdded();
};

void AssemblyImagesModelTest::testCtor()
{
    AssemblyImagesModel model;
    QCOMPARE(model.rowCount(), 0);
    QCOMPARE(model.isLoading(), false);
    QCOMPARE(model.property("loading").toBool(), false);
}

void AssemblyImagesModelTest::testNoFiles()
{
    AssemblyImagesModel model;
    QSignalSpy modelResetSpy{&model, &AssemblyImagesModel::modelReset};
    QVERIFY(modelResetSpy.isValid());
    QSignalSpy loadingChangedSpy{&model, &AssemblyImagesModel::loadingChanged};
    QVERIFY(loadingChangedSpy.isValid());

    QTemporaryDir tmp;
    QDir dir{tmp.path()};
    model.loadImages(dir);
    QVERIFY(modelResetSpy.wait());
    QCOMPARE(loadingChangedSpy.count(), 2);
    QCOMPARE(model.rowCount(), 0);

    // let's create a non-image file
    QFile file{dir.filePath(QStringLiteral("text.txt"))};
    QVERIFY(file.open(QIODevice::WriteOnly));
    file.write(QByteArrayLiteral("foo"));
    file.close();

    // let's load again
    model.loadImages(dir);
    QVERIFY(modelResetSpy.wait());
    QCOMPARE(model.rowCount(), 0);
}

void AssemblyImagesModelTest::testImageFiles()
{
    AssemblyImagesModel model;
    QSignalSpy modelResetSpy{&model, &AssemblyImagesModel::modelReset};
    QVERIFY(modelResetSpy.isValid());

    QTemporaryDir tmp;
    QDir dir{tmp.path()};

    // create images
    QImage image{10, 20, QImage::Format_RGB32};
    image.fill(Qt::red);
    image.save(dir.filePath(QStringLiteral("image.png")));
    image.save(dir.filePath(QStringLiteral("bitmap.bmp")));
    image.save(dir.filePath(QStringLiteral("jpeg.jpg")));

    model.loadImages(dir.absolutePath());
    QVERIFY(modelResetSpy.wait());
    QCOMPARE(model.rowCount(), 3);

    QCOMPARE(model.index(0, 0).data().toString(), QStringLiteral("bitmap"));
    QCOMPARE(model.index(1, 0).data().toString(), QStringLiteral("image"));
    QCOMPARE(model.index(2, 0).data().toString(), QStringLiteral("jpeg"));

    QCOMPARE(model.index(0, 0).data(Qt::UserRole).toString(), dir.filePath(QStringLiteral("bitmap.bmp")));
    QCOMPARE(model.index(1, 0).data(Qt::UserRole).toString(), dir.filePath(QStringLiteral("image.png")));
    QCOMPARE(model.index(2, 0).data(Qt::UserRole).toString(), dir.filePath(QStringLiteral("jpeg.jpg")));

    QCOMPARE(model.index(0, 0).data(Qt::UserRole + 1).toString(), QStringLiteral("bitmap.bmp"));
    QCOMPARE(model.index(1, 0).data(Qt::UserRole + 1).toString(), QStringLiteral("image.png"));
    QCOMPARE(model.index(2, 0).data(Qt::UserRole + 1).toString(), QStringLiteral("jpeg.jpg"));
}

void AssemblyImagesModelTest::testImagesAdded()
{
    AssemblyImagesModel model;
    QSignalSpy modelResetSpy{&model, &AssemblyImagesModel::modelReset};
    QVERIFY(modelResetSpy.isValid());

    QTemporaryDir tmp;
    QDir dir{tmp.path()};
    model.loadImages(dir);
    QVERIFY(modelResetSpy.wait());
    QCOMPARE(model.rowCount(), 0);

    QImage image{10, 20, QImage::Format_RGB32};
    image.fill(Qt::red);
    image.save(dir.filePath(QStringLiteral("image.png")));
    QVERIFY(modelResetSpy.wait());
    QCOMPARE(model.rowCount(), 1);
}

QTEST_GUILESS_MAIN(AssemblyImagesModelTest)
#include "assemblyImagesModelTest.moc"
