#include <QTest>
#include <QSignalSpy>

#include "../src/seamErrorModel.h"
#include "../src/simpleErrorModel.h"
#include "../src/errorGroupModel.h"
#include "attributeModel.h"
#include "product.h"
#include "seam.h"
#include "seamError.h"

using precitec::gui::SeamErrorModel;
using precitec::gui::SimpleErrorModel;
using precitec::storage::AttributeModel;
using precitec::storage::Product;
using precitec::storage::Seam;
using precitec::storage::SeamError;

class SeamErrorModelTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testCtor();
    void testRoleNames();
    void testDisplayRole_data();
    void testDisplayRole();
    void testError_data();
    void testError();
    void testSetAttributeModel();
    void testSetCurrentSeam();
    void testCreateSeamError();
    void testRemoveError();
};

void SeamErrorModelTest::testCtor()
{
    SeamErrorModel control{this};
    QVERIFY(!control.currentSeam());
    QVERIFY(!control.attributeModel());
    QCOMPARE(control.rowCount(), 0);
}

void SeamErrorModelTest::testRoleNames()
{
    SeamErrorModel control{this};
    const auto roleNames = control.roleNames();
    QCOMPARE(roleNames.size(), 3);
    QCOMPARE(roleNames[Qt::DisplayRole], QByteArrayLiteral("name"));
    QCOMPARE(roleNames[Qt::UserRole], QByteArrayLiteral("error"));
}

void SeamErrorModelTest::testDisplayRole_data()
{
    QTest::addColumn<int>("row");
    QTest::addColumn<QString>("name");

    QTest::newRow("0")  <<  0 << QStringLiteral("Length Outside Static Boundary Failure");
    QTest::newRow("1")  <<  1 << QStringLiteral("Length Outside Reference Boundary Failure");
    QTest::newRow("2")  <<  2 << QStringLiteral("Accumulated Length Outside Static Boundary Failure");
    QTest::newRow("3")  <<  3 << QStringLiteral("Accumulated Length Outside Reference Boundary Failure");
    QTest::newRow("4")  <<  4 << QStringLiteral("Length Inside Static Boundary Failure");
    QTest::newRow("5")  <<  5 << QStringLiteral("Length Inside Reference Boundary Failure");
    QTest::newRow("6")  <<  6 << QStringLiteral("Area Outside Static Boundary Failure");
    QTest::newRow("7")  <<  7 << QStringLiteral("Area Outside Reference Boundary Failure");
    QTest::newRow("8")  <<  8 << QStringLiteral("Accumulated Area Outside Static Boundary Failure");
    QTest::newRow("9")  <<  9 << QStringLiteral("Accumulated Area Outside Reference Boundary Failure");
    QTest::newRow("10")  <<  10 << QStringLiteral("Peak Outside Static Boundary Failure");
    QTest::newRow("11")  <<  11 << QStringLiteral("Peak Outside Reference Boundary Failure");
}

void SeamErrorModelTest::testDisplayRole()
{
    SeamErrorModel control{this};

    Product p{QUuid::createUuid()};
    p.createFirstSeamSeries();
    auto s = p.createSeam();
    QVERIFY(s);
    control.setCurrentSeam(s);

    control.createError(SimpleErrorModel::ErrorType::LengthOutsideStaticBoundary);
    control.createError(SimpleErrorModel::ErrorType::LengthOutsideReferenceBoundary);
    control.createError(SimpleErrorModel::ErrorType::AccumulatedLengthOutsideStaticBoundary);
    control.createError(SimpleErrorModel::ErrorType::AccumulatedLengthOutsideReferenceBoundary);
    control.createError(SimpleErrorModel::ErrorType::LengthInsideStaticBoundary);
    control.createError(SimpleErrorModel::ErrorType::LengthInsideReferenceBoundary);
    control.createError(SimpleErrorModel::ErrorType::AreaStaticBoundary);
    control.createError(SimpleErrorModel::ErrorType::AreaReferenceBoundary);
    control.createError(SimpleErrorModel::ErrorType::AccumulatedAreaStaticBoundary);
    control.createError(SimpleErrorModel::ErrorType::AccumulatedAreaReferenceBoundary);
    control.createError(SimpleErrorModel::ErrorType::PeakStaticBoundary);
    control.createError(SimpleErrorModel::ErrorType::PeakReferenceBoundary);

    QFETCH(int, row);
    const auto index = control.index(row, 0);
    QVERIFY(index.isValid());
    QTEST(index.data().toString(), "name");
}

void SeamErrorModelTest::testError_data()
{
    QTest::addColumn<int>("row");
    QTest::addColumn<QString>("name");
    QTest::addColumn<QUuid>("variantId");

    QTest::newRow("0")  <<  0 << QStringLiteral("Length Outside Static Boundary Failure") << QUuid{"3B5FE50F-6FD5-4FBC-BD78-06B892E1F97D"};
    QTest::newRow("1")  <<  1 << QStringLiteral("Length Outside Reference Boundary Failure") << QUuid{"5EB04560-2641-4E64-A016-14207E59A370"};
    QTest::newRow("2")  <<  2 << QStringLiteral("Accumulated Length Outside Static Boundary Failure") << QUuid{"CEDEACB4-D4BB-4FDC-945A-BDC54E5848A6"};
    QTest::newRow("3")  <<  3 << QStringLiteral("Accumulated Length Outside Reference Boundary Failure") << QUuid{"F8F4E0A8-D259-40F9-B134-68AA24E0A06C"};
    QTest::newRow("4")  <<  4 << QStringLiteral("Length Inside Static Boundary Failure") << QUuid{"3AF9EF6A-A4E9-4234-8BA5-7B42D3E58B2C"};
    QTest::newRow("5")  <<  5 << QStringLiteral("Length Inside Reference Boundary Failure") << QUuid{"4A6AE9B0-3A1A-427F-8D58-2D0205452377"};
    QTest::newRow("6")  <<  6 << QStringLiteral("Area Outside Static Boundary Failure") << QUuid{"73708EA1-580A-4660-8D80-63622670BC7C"};
    QTest::newRow("7")  <<  7 << QStringLiteral("Area Outside Reference Boundary Failure") << QUuid{"D36ECEBA-286B-4D06-B596-0491B6544F40"};
    QTest::newRow("8")  <<  8 << QStringLiteral("Accumulated Area Outside Static Boundary Failure") << QUuid{"740FD8B3-852C-485A-BC24-6C67A36DABD2"};
    QTest::newRow("9")  <<  9 << QStringLiteral("Accumulated Area Outside Reference Boundary Failure") << QUuid{"527B7421-5DDD-436C-BE33-C1A359A736F6"};
    QTest::newRow("10")  <<  10 << QStringLiteral("Peak Outside Static Boundary Failure") << QUuid{"396CA433-AD11-4073-A2B2-5314CC41D152"};
    QTest::newRow("11")  <<  11 << QStringLiteral("Peak Outside Reference Boundary Failure") << QUuid{"7CF9F16D-36DE-4840-A2EA-C41979F91A9B"};
}

void SeamErrorModelTest::testError()
{
    SeamErrorModel control{this};

    Product p{QUuid::createUuid()};
    p.createFirstSeamSeries();
    auto s = p.createSeam();
    QVERIFY(s);
    control.setCurrentSeam(s);

    control.createError(SimpleErrorModel::ErrorType::LengthOutsideStaticBoundary);
    control.createError(SimpleErrorModel::ErrorType::LengthOutsideReferenceBoundary);
    control.createError(SimpleErrorModel::ErrorType::AccumulatedLengthOutsideStaticBoundary);
    control.createError(SimpleErrorModel::ErrorType::AccumulatedLengthOutsideReferenceBoundary);
    control.createError(SimpleErrorModel::ErrorType::LengthInsideStaticBoundary);
    control.createError(SimpleErrorModel::ErrorType::LengthInsideReferenceBoundary);
    control.createError(SimpleErrorModel::ErrorType::AreaStaticBoundary);
    control.createError(SimpleErrorModel::ErrorType::AreaReferenceBoundary);
    control.createError(SimpleErrorModel::ErrorType::AccumulatedAreaStaticBoundary);
    control.createError(SimpleErrorModel::ErrorType::AccumulatedAreaReferenceBoundary);
    control.createError(SimpleErrorModel::ErrorType::PeakStaticBoundary);
    control.createError(SimpleErrorModel::ErrorType::PeakReferenceBoundary);

    QFETCH(int, row);
    const auto index = control.index(row, 0);
    QVERIFY(index.isValid());
    QTEST(index.data(Qt::UserRole).value<SeamError*>()->name(), "name");
    QTEST(index.data(Qt::UserRole).value<SeamError*>()->variantId(), "variantId");
}

void SeamErrorModelTest::testSetAttributeModel()
{
    SeamErrorModel control{this};
    QSignalSpy attributeModelChangedSpy(&control, &SeamErrorModel::attributeModelChanged);
    QVERIFY(attributeModelChangedSpy.isValid());

    std::unique_ptr<AttributeModel> am = std::make_unique<AttributeModel>();
    QSignalSpy modelResetSpy(am.get(), &AttributeModel::modelReset);
    QVERIFY(modelResetSpy.isValid());

    am->load(QFINDTESTDATA("testdata/errors_data/attributes.json"));
    QVERIFY(modelResetSpy.wait());
    QCOMPARE(modelResetSpy.count(), 1);

    control.setAttributeModel(am.get());
    QCOMPARE(control.attributeModel(), am.get());
    QCOMPARE(attributeModelChangedSpy.count(), 1);

    control.setAttributeModel(am.get());
    QCOMPARE(attributeModelChangedSpy.count(), 1);

    Product p{QUuid::createUuid()};
    p.createFirstSeamSeries();
    auto s = p.createSeam();
    QVERIFY(s);
    control.setCurrentSeam(s);

    auto error = control.createError(SimpleErrorModel::ErrorType::AccumulatedLengthOutsideStaticBoundary);
    QVERIFY(error);
    QCOMPARE(error->max(), 3);

    am.reset();
    QCOMPARE(control.attributeModel(), nullptr);
    QCOMPARE(attributeModelChangedSpy.count(), 2);
}

void SeamErrorModelTest::testSetCurrentSeam()
{
    SeamErrorModel control{this};

    QSignalSpy seamChangedSpy(&control, &SeamErrorModel::currentSeamChanged);
    QVERIFY(seamChangedSpy.isValid());

    Product p{QUuid::createUuid()};
    p.createFirstSeamSeries();
    auto s = p.createSeam();
    QVERIFY(s);

    control.setCurrentSeam(s);
    QCOMPARE(control.currentSeam(), s);
    QCOMPARE(seamChangedSpy.count(), 1);

    control.setCurrentSeam(s);
    QCOMPARE(seamChangedSpy.count(), 1);

    s->deleteLater();
    QVERIFY(seamChangedSpy.wait());
    QCOMPARE(control.currentSeam(), nullptr);
    QCOMPARE(seamChangedSpy.count(), 2);
}

void SeamErrorModelTest::testCreateSeamError()
{
    SeamErrorModel control{this};
    QSignalSpy rowsInsertedSpy(&control, &SeamErrorModel::rowsInserted);
    QVERIFY(rowsInsertedSpy.isValid());

    QCOMPARE(rowsInsertedSpy.count(), 0);

    auto nullError = control.createError(SimpleErrorModel::ErrorType::LengthOutsideStaticBoundary);
    QVERIFY(!nullError);
    QCOMPARE(rowsInsertedSpy.count(), 0);

    Product p{QUuid::createUuid()};
    p.createFirstSeamSeries();
    auto s = p.createSeam();
    QVERIFY(s);
    control.setCurrentSeam(s);

    auto lengthOutsideStatic = control.createError(SimpleErrorModel::ErrorType::LengthOutsideStaticBoundary);
    QVERIFY(lengthOutsideStatic);
    QCOMPARE(rowsInsertedSpy.count(), 1);
    QCOMPARE(lengthOutsideStatic->name(), QStringLiteral("Length Outside Static Boundary Failure"));
    QCOMPARE(lengthOutsideStatic->variantId(), QUuid{"3B5FE50F-6FD5-4FBC-BD78-06B892E1F97D"});

    auto accLengthOutsideStatic = control.createError(SimpleErrorModel::ErrorType::AccumulatedLengthOutsideStaticBoundary);
    QVERIFY(accLengthOutsideStatic);
    QCOMPARE(rowsInsertedSpy.count(), 2);
    QCOMPARE(accLengthOutsideStatic->name(), QStringLiteral("Accumulated Length Outside Static Boundary Failure"));
    QCOMPARE(accLengthOutsideStatic->variantId(), QUuid{"CEDEACB4-D4BB-4FDC-945A-BDC54E5848A6"});

    auto lengthOutsideReference = control.createError(SimpleErrorModel::ErrorType::LengthOutsideReferenceBoundary);
    QVERIFY(lengthOutsideReference);
    QCOMPARE(rowsInsertedSpy.count(), 3);
    QCOMPARE(lengthOutsideReference->name(), QStringLiteral("Length Outside Reference Boundary Failure"));
    QCOMPARE(lengthOutsideReference->variantId(), QUuid{"5EB04560-2641-4E64-A016-14207E59A370"});

    auto accLengthOutsideReference = control.createError(SimpleErrorModel::ErrorType::AccumulatedLengthOutsideReferenceBoundary);
    QVERIFY(accLengthOutsideReference);
    QCOMPARE(rowsInsertedSpy.count(), 4);
    QCOMPARE(accLengthOutsideReference->name(), QStringLiteral("Accumulated Length Outside Reference Boundary Failure"));
    QCOMPARE(accLengthOutsideReference->variantId(), QUuid{"F8F4E0A8-D259-40F9-B134-68AA24E0A06C"});

    auto lengthInsideStatic = control.createError(SimpleErrorModel::ErrorType::LengthInsideStaticBoundary);
    QVERIFY(lengthInsideStatic);
    QCOMPARE(rowsInsertedSpy.count(), 5);
    QCOMPARE(lengthInsideStatic->name(), QStringLiteral("Length Inside Static Boundary Failure"));
    QCOMPARE(lengthInsideStatic->variantId(), QUuid{"3AF9EF6A-A4E9-4234-8BA5-7B42D3E58B2C"});

    auto lengthInsideReference = control.createError(SimpleErrorModel::ErrorType::LengthInsideReferenceBoundary);
    QVERIFY(lengthInsideReference);
    QCOMPARE(rowsInsertedSpy.count(), 6);
    QCOMPARE(lengthInsideReference->name(), QStringLiteral("Length Inside Reference Boundary Failure"));
    QCOMPARE(lengthInsideReference->variantId(), QUuid{"4A6AE9B0-3A1A-427F-8D58-2D0205452377"});

    auto areaOutsideStatic = control.createError(SimpleErrorModel::ErrorType::AreaStaticBoundary);
    QVERIFY(areaOutsideStatic);
    QCOMPARE(rowsInsertedSpy.count(), 7);
    QCOMPARE(areaOutsideStatic->name(), QStringLiteral("Area Outside Static Boundary Failure"));
    QCOMPARE(areaOutsideStatic->variantId(), QUuid{"73708EA1-580A-4660-8D80-63622670BC7C"});

    auto accAreaOutsideStatic = control.createError(SimpleErrorModel::ErrorType::AccumulatedAreaStaticBoundary);
    QVERIFY(accAreaOutsideStatic);
    QCOMPARE(rowsInsertedSpy.count(), 8);
    QCOMPARE(accAreaOutsideStatic->name(), QStringLiteral("Accumulated Area Outside Static Boundary Failure"));
    QCOMPARE(accAreaOutsideStatic->variantId(), QUuid{"740FD8B3-852C-485A-BC24-6C67A36DABD2"});

    auto areaOutsideReference = control.createError(SimpleErrorModel::ErrorType::AreaReferenceBoundary);
    QVERIFY(areaOutsideReference);
    QCOMPARE(rowsInsertedSpy.count(), 9);
    QCOMPARE(areaOutsideReference->name(), QStringLiteral("Area Outside Reference Boundary Failure"));
    QCOMPARE(areaOutsideReference->variantId(), QUuid{"D36ECEBA-286B-4D06-B596-0491B6544F40"});

    auto accAreaOutsideReference = control.createError(SimpleErrorModel::ErrorType::AccumulatedAreaReferenceBoundary);
    QVERIFY(accAreaOutsideReference);
    QCOMPARE(rowsInsertedSpy.count(), 10);
    QCOMPARE(accAreaOutsideReference->name(), QStringLiteral("Accumulated Area Outside Reference Boundary Failure"));
    QCOMPARE(accAreaOutsideReference->variantId(), QUuid{"527B7421-5DDD-436C-BE33-C1A359A736F6"});

    auto peakStatic = control.createError(SimpleErrorModel::ErrorType::PeakStaticBoundary);
    QVERIFY(peakStatic);
    QCOMPARE(rowsInsertedSpy.count(), 11);
    QCOMPARE(peakStatic->name(), QStringLiteral("Peak Outside Static Boundary Failure"));
    QCOMPARE(peakStatic->variantId(), QUuid{"396CA433-AD11-4073-A2B2-5314CC41D152"});

    auto peakReference = control.createError(SimpleErrorModel::ErrorType::PeakReferenceBoundary);
    QVERIFY(peakReference);
    QCOMPARE(rowsInsertedSpy.count(), 12);
    QCOMPARE(peakReference->name(), QStringLiteral("Peak Outside Reference Boundary Failure"));
    QCOMPARE(peakReference->variantId(), QUuid{"7CF9F16D-36DE-4840-A2EA-C41979F91A9B"});
}

void SeamErrorModelTest::testRemoveError()
{
    SeamErrorModel control{this};
    QSignalSpy rowsRemovedSpy(&control, &SeamErrorModel::rowsRemoved);
    QVERIFY(rowsRemovedSpy.isValid());

    QCOMPARE(rowsRemovedSpy.count(), 0);

    SeamError newError{this};
    control.removeError(&newError);
    QCOMPARE(rowsRemovedSpy.count(), 0);

    Product p{QUuid::createUuid()};
    p.createFirstSeamSeries();
    auto s = p.createSeam();
    QVERIFY(s);
    control.setCurrentSeam(s);

    auto lengthOutsideStatic = control.createError(SimpleErrorModel::ErrorType::LengthOutsideStaticBoundary);
    QVERIFY(lengthOutsideStatic);

    auto accLengthOutsideStatic = control.createError(SimpleErrorModel::ErrorType::AccumulatedLengthOutsideStaticBoundary);
    QVERIFY(accLengthOutsideStatic);

    auto lengthOutsideReference = control.createError(SimpleErrorModel::ErrorType::LengthOutsideReferenceBoundary);
    QVERIFY(lengthOutsideReference);

    auto accLengthOutsideReference = control.createError(SimpleErrorModel::ErrorType::AccumulatedLengthOutsideReferenceBoundary);
    QVERIFY(accLengthOutsideReference);

    QCOMPARE(control.rowCount(), 4);

    control.removeError(nullptr);
    QCOMPARE(rowsRemovedSpy.count(), 0);

    control.removeError(accLengthOutsideStatic);
    QCOMPARE(rowsRemovedSpy.count(), 1);
    QCOMPARE(control.rowCount(), 3);

    QCOMPARE(control.index(0, 0).data(Qt::UserRole).value<SeamError*>()->variantId(), QUuid{"3B5FE50F-6FD5-4FBC-BD78-06B892E1F97D"});
    QCOMPARE(control.index(1, 0).data(Qt::UserRole).value<SeamError*>()->variantId(), QUuid{"5EB04560-2641-4E64-A016-14207E59A370"});
    QCOMPARE(control.index(2, 0).data(Qt::UserRole).value<SeamError*>()->variantId(), QUuid{"F8F4E0A8-D259-40F9-B134-68AA24E0A06C"});
}

QTEST_GUILESS_MAIN(SeamErrorModelTest)
#include "seamErrorModelTest.moc"


