#include <QTest>
#include <QSignalSpy>

#include "../src/seamSeriesErrorModel.h"
#include "product.h"
#include "seamSeries.h"
#include "seamSeriesError.h"
#include "attributeModel.h"

using precitec::gui::SeamSeriesErrorModel;
using precitec::storage::Product;
using precitec::storage::SeamSeries;
using precitec::storage::SeamSeriesError;
using precitec::storage::AttributeModel;

class SeamSeriesErrorModelTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testCtor();
    void testRoleNames();
    void testDisplayRole_data();
    void testDisplayRole();
    void testError();
    void testType_data();
    void testType();
    void testIsType_data();
    void testIsType();
    void testCurrentSeamSeries();
    void testAttributeModel();
    void testVariantId();
    void testName();
    void testNameFromId();
    void testIsTypeError();
    void testCreateError();
    void testRemoveError();
};

void SeamSeriesErrorModelTest::testCtor()
{
    SeamSeriesErrorModel model{this};
    QVERIFY(!model.currentSeamSeries());
    QVERIFY(!model.attributeModel());
    QCOMPARE(model.rowCount(), 0);
}

void SeamSeriesErrorModelTest::testRoleNames()
{
    SeamSeriesErrorModel model{this};
    const auto roleNames = model.roleNames();
    QCOMPARE(roleNames.size(), 4);
    QCOMPARE(roleNames[Qt::DisplayRole], QByteArrayLiteral("name"));
    QCOMPARE(roleNames[Qt::UserRole ], QByteArrayLiteral("error"));
    QCOMPARE(roleNames[Qt::UserRole + 1], QByteArrayLiteral("type"));
    QCOMPARE(roleNames[Qt::UserRole + 2], QByteArrayLiteral("isTypeError"));
}

void SeamSeriesErrorModelTest::testDisplayRole_data()
{
    QTest::addColumn<SeamSeriesErrorModel::ErrorType>("errorType");
    QTest::addColumn<QString>("name");

    QTest::newRow("0") << SeamSeriesErrorModel::ErrorType::ConsecutiveTypeErrors << QStringLiteral("Consecutive Type Errors");
    QTest::newRow("1") << SeamSeriesErrorModel::ErrorType::AccumulatedTypeErrors << QStringLiteral("Accumulated Type Errors");
    QTest::newRow("2") << SeamSeriesErrorModel::ErrorType::ConsecutiveSeamErrors << QStringLiteral("Consecutive Seam Errors");
    QTest::newRow("3") << SeamSeriesErrorModel::ErrorType::AccumulatedSeamErrors << QStringLiteral("Accumulated Seam Errors");
}

void SeamSeriesErrorModelTest::testDisplayRole()
{
    SeamSeriesErrorModel model{this};
    const auto product = new Product(QUuid::createUuid(), this);
    const auto seamSeries = product->createSeamSeries();
    model.setCurrentSeamSeries(seamSeries);

    QFETCH(SeamSeriesErrorModel::ErrorType, errorType);
    auto error = model.createError(errorType);

    const auto index = model.index(0);
    QVERIFY(index.isValid());

    QTEST(index.data().toString(), "name");

    error->setName(QStringLiteral("My Error"));
    QCOMPARE(index.data().toString(), QStringLiteral("My Error"));
}

void SeamSeriesErrorModelTest::testError()
{
    SeamSeriesErrorModel model{this};
    const auto product = new Product(QUuid::createUuid(), this);
    const auto seamSeries = product->createSeamSeries();
    model.setCurrentSeamSeries(seamSeries);

    auto error1 = model.createError(SeamSeriesErrorModel::ErrorType::ConsecutiveTypeErrors);
    auto error2 = model.createError(SeamSeriesErrorModel::ErrorType::AccumulatedTypeErrors);
    auto error3 = model.createError(SeamSeriesErrorModel::ErrorType::ConsecutiveSeamErrors);
    auto error4 = model.createError(SeamSeriesErrorModel::ErrorType::AccumulatedSeamErrors);

    QCOMPARE(model.index(0).data(Qt::UserRole).value<SeamSeriesError*>(), error1);
    QCOMPARE(model.index(1).data(Qt::UserRole).value<SeamSeriesError*>(), error2);
    QCOMPARE(model.index(2).data(Qt::UserRole).value<SeamSeriesError*>(), error3);
    QCOMPARE(model.index(3).data(Qt::UserRole).value<SeamSeriesError*>(), error4);
}

void SeamSeriesErrorModelTest::testType_data()
{
    QTest::addColumn<SeamSeriesErrorModel::ErrorType>("errorType");
    QTest::addColumn<QString>("name");

    QTest::newRow("0") << SeamSeriesErrorModel::ErrorType::ConsecutiveTypeErrors << QStringLiteral("Consecutive Type Errors");
    QTest::newRow("1") << SeamSeriesErrorModel::ErrorType::AccumulatedTypeErrors << QStringLiteral("Accumulated Type Errors");
    QTest::newRow("2") << SeamSeriesErrorModel::ErrorType::ConsecutiveSeamErrors << QStringLiteral("Consecutive Seam Errors");
    QTest::newRow("3") << SeamSeriesErrorModel::ErrorType::AccumulatedSeamErrors << QStringLiteral("Accumulated Seam Errors");
}

void SeamSeriesErrorModelTest::testType()
{
    SeamSeriesErrorModel model{this};
    const auto product = new Product(QUuid::createUuid(), this);
    const auto seamSeries = product->createSeamSeries();
    model.setCurrentSeamSeries(seamSeries);

    QFETCH(SeamSeriesErrorModel::ErrorType, errorType);
    auto error = model.createError(errorType);

    const auto index = model.index(0);
    QVERIFY(index.isValid());

    QTEST(index.data(Qt::UserRole + 1).toString(), "name");

    error->setName(QStringLiteral("My Error"));

    QTEST(index.data(Qt::UserRole + 1).toString(), "name");
}

void SeamSeriesErrorModelTest::testIsType_data()
{
    QTest::addColumn<SeamSeriesErrorModel::ErrorType>("errorType");
    QTest::addColumn<bool>("isTypeError");

    QTest::newRow("0") << SeamSeriesErrorModel::ErrorType::ConsecutiveTypeErrors << true;
    QTest::newRow("1") << SeamSeriesErrorModel::ErrorType::AccumulatedTypeErrors << true;
    QTest::newRow("2") << SeamSeriesErrorModel::ErrorType::ConsecutiveSeamErrors << false;
    QTest::newRow("3") << SeamSeriesErrorModel::ErrorType::AccumulatedSeamErrors << false;
}

void SeamSeriesErrorModelTest::testIsType()
{
    SeamSeriesErrorModel model{this};
    const auto product = new Product(QUuid::createUuid(), this);
    const auto seamSeries = product->createSeamSeries();
    model.setCurrentSeamSeries(seamSeries);

    QFETCH(SeamSeriesErrorModel::ErrorType, errorType);
    model.createError(errorType);

    const auto index = model.index(0);
    QVERIFY(index.isValid());
    QTEST(index.data(Qt::UserRole + 2).toBool(), "isTypeError");
}

void SeamSeriesErrorModelTest::testCurrentSeamSeries()
{
    SeamSeriesErrorModel model{this};
    QSignalSpy currentSeamSeriesChangedSpy(&model, &SeamSeriesErrorModel::currentSeamSeriesChanged);
    QVERIFY(currentSeamSeriesChangedSpy.isValid());
    QSignalSpy modelResetSpy(&model, &SeamSeriesErrorModel::modelReset);
    QVERIFY(modelResetSpy.isValid());

    const auto product = new Product(QUuid::createUuid(), this);
    const auto seamSeries = product->createSeamSeries();
    model.setCurrentSeamSeries(seamSeries);
    QCOMPARE(model.currentSeamSeries(), seamSeries);
    QCOMPARE(currentSeamSeriesChangedSpy.count(), 1);
    QCOMPARE(modelResetSpy.count(), 1);

    model.setCurrentSeamSeries(seamSeries);
    QCOMPARE(currentSeamSeriesChangedSpy.count(), 1);
    QCOMPARE(modelResetSpy.count(), 1);

    const auto seamSeries2 = product->createSeamSeries();
    QSignalSpy destroyedSpy(seamSeries2, &SeamSeries::destroyed);

    model.setCurrentSeamSeries(seamSeries2);
    QCOMPARE(model.currentSeamSeries(), seamSeries2);
    QCOMPARE(currentSeamSeriesChangedSpy.count(), 2);
    QCOMPARE(modelResetSpy.count(), 2);

    seamSeries2->deleteLater();
    QVERIFY(destroyedSpy.wait());
    QVERIFY(!model.currentSeamSeries());
    QCOMPARE(currentSeamSeriesChangedSpy.count(), 3);
    QCOMPARE(modelResetSpy.count(), 3);
}

void SeamSeriesErrorModelTest::testAttributeModel()
{
    SeamSeriesErrorModel model{this};
    QSignalSpy attributeModelChangedSpy(&model, &SeamSeriesErrorModel::attributeModelChanged);
    QVERIFY(attributeModelChangedSpy.isValid());

    std::unique_ptr<AttributeModel> am = std::make_unique<AttributeModel>();
    QSignalSpy modelResetSpy(am.get(), &AttributeModel::modelReset);
    QVERIFY(modelResetSpy.isValid());

    am->load(QFINDTESTDATA("testdata/errors_data/attributes.json"));
    QVERIFY(modelResetSpy.wait());
    QCOMPARE(modelResetSpy.count(), 1);

    model.setAttributeModel(am.get());
    QCOMPARE(model.attributeModel(), am.get());
    QCOMPARE(attributeModelChangedSpy.count(), 1);

    model.setAttributeModel(am.get());
    QCOMPARE(attributeModelChangedSpy.count(), 1);

    const auto product = new Product(QUuid::createUuid(), this);
    const auto seamSeries = product->createSeamSeries();
    model.setCurrentSeamSeries(seamSeries);

    auto error = model.createError(SeamSeriesErrorModel::ErrorType::ConsecutiveTypeErrors);
    QCOMPARE(error->threshold(), 10);

    am.reset();
    QCOMPARE(model.attributeModel(), nullptr);
    QCOMPARE(attributeModelChangedSpy.count(), 2);
}

void SeamSeriesErrorModelTest::testVariantId()
{
    SeamSeriesErrorModel model{this};
    QCOMPARE(model.variantId(SeamSeriesErrorModel::ErrorType::ConsecutiveTypeErrors), QUuid("37E21057-EFD4-4C18-A298-BE9F804C6C04"));
    QCOMPARE(model.variantId(SeamSeriesErrorModel::ErrorType::AccumulatedTypeErrors), QUuid("C19E43C7-EBC0-4771-A701-BA102511AD9F"));
    QCOMPARE(model.variantId(SeamSeriesErrorModel::ErrorType::ConsecutiveSeamErrors), QUuid("753EA75B-4B82-418E-B1F2-0EEA0D4C0D50"));
    QCOMPARE(model.variantId(SeamSeriesErrorModel::ErrorType::AccumulatedSeamErrors), QUuid("B47EBF8B-0D37-48BD-8A6A-0D6E09885C1F"));
}

void SeamSeriesErrorModelTest::testName()
{
    SeamSeriesErrorModel model{this};
    QCOMPARE(model.name(SeamSeriesErrorModel::ErrorType::ConsecutiveTypeErrors), QStringLiteral("Consecutive Type Errors"));
    QCOMPARE(model.name(SeamSeriesErrorModel::ErrorType::AccumulatedTypeErrors), QStringLiteral("Accumulated Type Errors"));
    QCOMPARE(model.name(SeamSeriesErrorModel::ErrorType::ConsecutiveSeamErrors), QStringLiteral("Consecutive Seam Errors"));
    QCOMPARE(model.name(SeamSeriesErrorModel::ErrorType::AccumulatedSeamErrors), QStringLiteral("Accumulated Seam Errors"));
}

void SeamSeriesErrorModelTest::testNameFromId()
{
    SeamSeriesErrorModel model{this};
    QCOMPARE(model.nameFromId("37E21057-EFD4-4C18-A298-BE9F804C6C04"), QStringLiteral("Consecutive Type Errors"));
    QCOMPARE(model.nameFromId("C19E43C7-EBC0-4771-A701-BA102511AD9F"), QStringLiteral("Accumulated Type Errors"));
    QCOMPARE(model.nameFromId("753EA75B-4B82-418E-B1F2-0EEA0D4C0D50"), QStringLiteral("Consecutive Seam Errors"));
    QCOMPARE(model.nameFromId("B47EBF8B-0D37-48BD-8A6A-0D6E09885C1F"), QStringLiteral("Accumulated Seam Errors"));
    QCOMPARE(model.nameFromId(QUuid::createUuid()), QStringLiteral(""));
}

void SeamSeriesErrorModelTest::testIsTypeError()
{
    SeamSeriesErrorModel model{this};
    QCOMPARE(model.isTypeError("37E21057-EFD4-4C18-A298-BE9F804C6C04"), true);
    QCOMPARE(model.isTypeError("C19E43C7-EBC0-4771-A701-BA102511AD9F"), true);
    QCOMPARE(model.isTypeError("753EA75B-4B82-418E-B1F2-0EEA0D4C0D50"), false);
    QCOMPARE(model.isTypeError("B47EBF8B-0D37-48BD-8A6A-0D6E09885C1F"), false);
}

void SeamSeriesErrorModelTest::testCreateError()
{
    SeamSeriesErrorModel model{this};
    QSignalSpy rowsInsertedSpy(&model, &SeamSeriesErrorModel::rowsInserted);
    QVERIFY(rowsInsertedSpy.isValid());

    auto error = model.createError(SeamSeriesErrorModel::ErrorType::ConsecutiveTypeErrors);
    QVERIFY(!error);

    const auto product = new Product(QUuid::createUuid(), this);
    const auto seamSeries = product->createSeamSeries();
    model.setCurrentSeamSeries(seamSeries);

    QCOMPARE(model.rowCount(), 0);
    QCOMPARE(seamSeries->overlyingErrors().size(), 0);

    auto error1 = model.createError(SeamSeriesErrorModel::ErrorType::ConsecutiveTypeErrors);
    QCOMPARE(error1->variantId(), QUuid{"37E21057-EFD4-4C18-A298-BE9F804C6C04"});
    QCOMPARE(model.rowCount(), 1);
    QCOMPARE(seamSeries->overlyingErrors().size(), 1);
    QCOMPARE(rowsInsertedSpy.count(), 1);

    auto error2 = model.createError(SeamSeriesErrorModel::ErrorType::AccumulatedTypeErrors);
    QCOMPARE(error2->variantId(), QUuid{"C19E43C7-EBC0-4771-A701-BA102511AD9F"});
    QCOMPARE(model.rowCount(), 2);
    QCOMPARE(seamSeries->overlyingErrors().size(), 2);
    QCOMPARE(rowsInsertedSpy.count(), 2);

    auto error3 = model.createError(SeamSeriesErrorModel::ErrorType::ConsecutiveSeamErrors);
    QCOMPARE(error3->variantId(), QUuid{"753EA75B-4B82-418E-B1F2-0EEA0D4C0D50"});
    QCOMPARE(model.rowCount(), 3);
    QCOMPARE(seamSeries->overlyingErrors().size(), 3);
    QCOMPARE(rowsInsertedSpy.count(), 3);

    auto error4 = model.createError(SeamSeriesErrorModel::ErrorType::AccumulatedSeamErrors);
    QCOMPARE(error4->variantId(), QUuid{"B47EBF8B-0D37-48BD-8A6A-0D6E09885C1F"});
    QCOMPARE(model.rowCount(), 4);
    QCOMPARE(seamSeries->overlyingErrors().size(), 4);
    QCOMPARE(rowsInsertedSpy.count(), 4);
}

void SeamSeriesErrorModelTest::testRemoveError()
{
    SeamSeriesErrorModel model{this};
    QSignalSpy rowsRemovedSpy(&model, &SeamSeriesErrorModel::rowsRemoved);
    QVERIFY(rowsRemovedSpy.isValid());

    const auto product = new Product(QUuid::createUuid(), this);
    const auto seamSeries = product->createSeamSeries();
    model.setCurrentSeamSeries(seamSeries);

    auto error1 = model.createError(SeamSeriesErrorModel::ErrorType::ConsecutiveTypeErrors);
    auto error2 = model.createError(SeamSeriesErrorModel::ErrorType::AccumulatedTypeErrors);
    auto error3 = model.createError(SeamSeriesErrorModel::ErrorType::ConsecutiveSeamErrors);
    auto error4 = model.createError(SeamSeriesErrorModel::ErrorType::AccumulatedSeamErrors);

    QCOMPARE(model.index(0).data(Qt::UserRole).value<SeamSeriesError*>(), error1);
    QCOMPARE(model.index(1).data(Qt::UserRole).value<SeamSeriesError*>(), error2);
    QCOMPARE(model.index(2).data(Qt::UserRole).value<SeamSeriesError*>(), error3);
    QCOMPARE(model.index(3).data(Qt::UserRole).value<SeamSeriesError*>(), error4);
    QCOMPARE(model.rowCount(), 4);

    model.removeError(error2);
    QCOMPARE(rowsRemovedSpy.count(), 1);
    QCOMPARE(model.rowCount(), 3);

    QCOMPARE(model.index(0).data(Qt::UserRole).value<SeamSeriesError*>(), error1);
    QCOMPARE(model.index(1).data(Qt::UserRole).value<SeamSeriesError*>(), error3);
    QCOMPARE(model.index(2).data(Qt::UserRole).value<SeamSeriesError*>(), error4);

    model.removeError(error4);
    QCOMPARE(rowsRemovedSpy.count(), 2);
    QCOMPARE(model.rowCount(), 2);

    QCOMPARE(model.index(0).data(Qt::UserRole).value<SeamSeriesError*>(), error1);
    QCOMPARE(model.index(1).data(Qt::UserRole).value<SeamSeriesError*>(), error3);

    model.removeError(error1);
    QCOMPARE(rowsRemovedSpy.count(), 3);
    QCOMPARE(model.rowCount(), 1);

    QCOMPARE(model.index(0).data(Qt::UserRole).value<SeamSeriesError*>(), error3);

    model.removeError(error3);
    QCOMPARE(rowsRemovedSpy.count(), 4);
    QCOMPARE(model.rowCount(), 0);
}

QTEST_GUILESS_MAIN(SeamSeriesErrorModelTest)
#include "seamSeriesErrorModelTest.moc"
