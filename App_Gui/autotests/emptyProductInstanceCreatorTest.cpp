#include <QTest>

#include "../src/emptyProductInstanceCreator.h"
#include "product.h"

using precitec::gui::EmptyProductInstanceCreator;
using precitec::storage::Product;

class EmptyProductInstanceCreatorTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testCtor();
    void testCreateFunctionWithoutProperties();
    void testCreateInNonExistingProductDirectory();
    void testCreateInExistingProductDirectoryWithoutAugmentUuidFile();
    void testCreateInExistingProductDirectoryWithAugmentUuidFile();

private:
};

void EmptyProductInstanceCreatorTest::testCtor()
{
    precitec::gui::EmptyProductInstanceCreator creator{this};
}

void EmptyProductInstanceCreatorTest::testCreateFunctionWithoutProperties()
{
    precitec::gui::EmptyProductInstanceCreator creator{this};

    QVERIFY(!creator.create());
}

void EmptyProductInstanceCreatorTest::testCreateInNonExistingProductDirectory()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    QString productDirName("product");
    QDir(dir.path()).mkdir(productDirName);
    // create a Product
    Product product(QUuid::createUuid());
    QUuid productInstanceId(QUuid::createUuid());

    precitec::gui::EmptyProductInstanceCreator creator{this};
    creator.setProductDirectory(dir.path() + "/" + productDirName);
    creator.setProduct(&product);
    creator.setSerialNumber(2);

    QVERIFY(creator.create());
    const auto productInstanceDir =
        QDir(dir.path() + "/" + productDirName + "/" + creator.productInstanceId().toString(QUuid::WithoutBraces) +
             "-SN-" + QString::number(creator.serialNumber()));
    QVERIFY(productInstanceDir.exists());
}

void EmptyProductInstanceCreatorTest::testCreateInExistingProductDirectoryWithoutAugmentUuidFile()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    Product product(QUuid::createUuid());
    QString productDirName(product.uuid().toString(QUuid::WithoutBraces));
    QDir(dir.path()).mkdir(productDirName);
    // create a Product
    QUuid productInstanceId(QUuid::createUuid());

    precitec::gui::EmptyProductInstanceCreator creator{this};
    creator.setProductDirectory(dir.path() + "/" + productDirName);
    creator.setProduct(&product);
    creator.setSerialNumber(2);

    QVERIFY(creator.create());
    const auto productInstanceDir =
        QDir(dir.path() + "/" + productDirName + "/" + creator.productInstanceId().toString(QUuid::WithoutBraces) +
             "-SN-" + QString::number(creator.serialNumber()));
    QVERIFY(productInstanceDir.exists());
}

void EmptyProductInstanceCreatorTest::testCreateInExistingProductDirectoryWithAugmentUuidFile()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    Product product(QUuid::createUuid());
    QString productDirName(product.uuid().toString(QUuid::WithoutBraces));
    QDir(dir.path()).mkdir(productDirName);
    QFile augmentProductUuidFile(dir.path() + "/" + productDirName + "/" +
                                 product.uuid().toString(QUuid::WithoutBraces) + ".id");
    QVERIFY(augmentProductUuidFile.open(QIODevice::WriteOnly));
    augmentProductUuidFile.close();

    QUuid productInstanceId(QUuid::createUuid());

    precitec::gui::EmptyProductInstanceCreator creator{this};
    creator.setProductDirectory(dir.path() + "/" + productDirName);
    creator.setProduct(&product);
    creator.setSerialNumber(2);

    QVERIFY(creator.create());
    const auto productInstanceDirectory =
        QDir(dir.path() + "/" + productDirName + "/" + creator.productInstanceId().toString(QUuid::WithoutBraces) +
             "-SN-" + QString::number(creator.serialNumber()));

    QFile augmentProductNullUuidFile(dir.path() + "/" + productDirName + "/" + QUuid{}.toString(QUuid::WithoutBraces) +
                                     ".id");
    QVERIFY(productInstanceDirectory.exists());
    QVERIFY(augmentProductUuidFile.exists());
    QVERIFY(!augmentProductNullUuidFile.exists());
}

QTEST_GUILESS_MAIN(EmptyProductInstanceCreatorTest)
#include "emptyProductInstanceCreatorTest.moc"
