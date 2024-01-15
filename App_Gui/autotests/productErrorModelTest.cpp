#include <QTest>
#include <QSignalSpy>

#include "../src/productErrorModel.h"
#include "product.h"
#include "productError.h"
#include "attributeModel.h"

using precitec::gui::ProductErrorModel;
using precitec::storage::Product;
using precitec::storage::ProductError;
using precitec::storage::AttributeModel;

class ProductErrorModelTest : public QObject
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
    void testCurrentProduct();
    void testAttributeModel();
    void testVariantId();
    void testName();
    void testNameFromId();
    void testIsTypeError();
    void testCreateError();
    void testRemoveError();
};

void ProductErrorModelTest::testCtor()
{
    ProductErrorModel model{this};
    QVERIFY(!model.currentProduct());
    QVERIFY(!model.attributeModel());
    QCOMPARE(model.rowCount(), 0);
}

void ProductErrorModelTest::testRoleNames()
{
    ProductErrorModel model{this};
    const auto roleNames = model.roleNames();
    QCOMPARE(roleNames.size(), 4);
    QCOMPARE(roleNames[Qt::DisplayRole], QByteArrayLiteral("name"));
    QCOMPARE(roleNames[Qt::UserRole ], QByteArrayLiteral("error"));
    QCOMPARE(roleNames[Qt::UserRole + 1], QByteArrayLiteral("type"));
    QCOMPARE(roleNames[Qt::UserRole + 2], QByteArrayLiteral("isTypeError"));
}

void ProductErrorModelTest::testDisplayRole_data()
{
    QTest::addColumn<ProductErrorModel::ErrorType>("errorType");
    QTest::addColumn<QString>("name");

    QTest::newRow("0") << ProductErrorModel::ErrorType::ConsecutiveTypeErrors << QStringLiteral("Consecutive Type Errors");
    QTest::newRow("1") << ProductErrorModel::ErrorType::AccumulatedTypeErrors << QStringLiteral("Accumulated Type Errors");
    QTest::newRow("2") << ProductErrorModel::ErrorType::ConsecutiveSeamErrors << QStringLiteral("Consecutive Seam Errors");
    QTest::newRow("3") << ProductErrorModel::ErrorType::AccumulatedSeamErrors << QStringLiteral("Accumulated Seam Errors");
}

void ProductErrorModelTest::testDisplayRole()
{
    ProductErrorModel model{this};
    const auto product = new Product(QUuid::createUuid(), this);
    model.setCurrentProduct(product);

    QFETCH(ProductErrorModel::ErrorType, errorType);
    auto error = model.createError(errorType);

    const auto index = model.index(0);
    QVERIFY(index.isValid());

    QTEST(index.data().toString(), "name");

    error->setName(QStringLiteral("My Error"));
    QCOMPARE(index.data().toString(), QStringLiteral("My Error"));
}

void ProductErrorModelTest::testError()
{
    ProductErrorModel model{this};
    const auto product = new Product(QUuid::createUuid(), this);
    model.setCurrentProduct(product);

    auto error1 = model.createError(ProductErrorModel::ErrorType::ConsecutiveTypeErrors);
    auto error2 = model.createError(ProductErrorModel::ErrorType::AccumulatedTypeErrors);
    auto error3 = model.createError(ProductErrorModel::ErrorType::ConsecutiveSeamErrors);
    auto error4 = model.createError(ProductErrorModel::ErrorType::AccumulatedSeamErrors);

    QCOMPARE(model.index(0).data(Qt::UserRole).value<ProductError*>(), error1);
    QCOMPARE(model.index(1).data(Qt::UserRole).value<ProductError*>(), error2);
    QCOMPARE(model.index(2).data(Qt::UserRole).value<ProductError*>(), error3);
    QCOMPARE(model.index(3).data(Qt::UserRole).value<ProductError*>(), error4);
}

void ProductErrorModelTest::testType_data()
{
    QTest::addColumn<ProductErrorModel::ErrorType>("errorType");
    QTest::addColumn<QString>("name");

    QTest::newRow("0") << ProductErrorModel::ErrorType::ConsecutiveTypeErrors << QStringLiteral("Consecutive Type Errors");
    QTest::newRow("1") << ProductErrorModel::ErrorType::AccumulatedTypeErrors << QStringLiteral("Accumulated Type Errors");
    QTest::newRow("2") << ProductErrorModel::ErrorType::ConsecutiveSeamErrors << QStringLiteral("Consecutive Seam Errors");
    QTest::newRow("3") << ProductErrorModel::ErrorType::AccumulatedSeamErrors << QStringLiteral("Accumulated Seam Errors");
}

void ProductErrorModelTest::testType()
{
    ProductErrorModel model{this};
    const auto product = new Product(QUuid::createUuid(), this);
    model.setCurrentProduct(product);

    QFETCH(ProductErrorModel::ErrorType, errorType);
    auto error = model.createError(errorType);

    const auto index = model.index(0);
    QVERIFY(index.isValid());

    QTEST(index.data(Qt::UserRole + 1).toString(), "name");

    error->setName(QStringLiteral("My Error"));

    QTEST(index.data(Qt::UserRole + 1).toString(), "name");
}

void ProductErrorModelTest::testIsType_data()
{
    QTest::addColumn<ProductErrorModel::ErrorType>("errorType");
    QTest::addColumn<bool>("isTypeError");

    QTest::newRow("0") << ProductErrorModel::ErrorType::ConsecutiveTypeErrors << true;
    QTest::newRow("1") << ProductErrorModel::ErrorType::AccumulatedTypeErrors << true;
    QTest::newRow("2") << ProductErrorModel::ErrorType::ConsecutiveSeamErrors << false;
    QTest::newRow("3") << ProductErrorModel::ErrorType::AccumulatedSeamErrors << false;
}

void ProductErrorModelTest::testIsType()
{
    ProductErrorModel model{this};
    const auto product = new Product(QUuid::createUuid(), this);
    model.setCurrentProduct(product);

    QFETCH(ProductErrorModel::ErrorType, errorType);
    model.createError(errorType);

    const auto index = model.index(0);
    QVERIFY(index.isValid());
    QTEST(index.data(Qt::UserRole + 2).toBool(), "isTypeError");
}

void ProductErrorModelTest::testCurrentProduct()
{
    ProductErrorModel model{this};
    QSignalSpy currentProductChangedSpy(&model, &ProductErrorModel::currentProductChanged);
    QVERIFY(currentProductChangedSpy.isValid());
    QSignalSpy modelResetSpy(&model, &ProductErrorModel::modelReset);
    QVERIFY(modelResetSpy.isValid());

    const auto product = new Product(QUuid::createUuid(), this);
    model.setCurrentProduct(product);
    QCOMPARE(model.currentProduct(), product);
    QCOMPARE(currentProductChangedSpy.count(), 1);
    QCOMPARE(modelResetSpy.count(), 1);

    model.setCurrentProduct(product);
    QCOMPARE(currentProductChangedSpy.count(), 1);
    QCOMPARE(modelResetSpy.count(), 1);

    const auto product2 = new Product(QUuid::createUuid(), this);
    QSignalSpy destroyedSpy(product2, &Product::destroyed);

    model.setCurrentProduct(product2);
    QCOMPARE(model.currentProduct(), product2);
    QCOMPARE(currentProductChangedSpy.count(), 2);
    QCOMPARE(modelResetSpy.count(), 2);

    product2->deleteLater();
    QVERIFY(destroyedSpy.wait());
    QVERIFY(!model.currentProduct());
    QCOMPARE(currentProductChangedSpy.count(), 3);
    QCOMPARE(modelResetSpy.count(), 3);
}

void ProductErrorModelTest::testAttributeModel()
{
    ProductErrorModel model{this};
    QSignalSpy attributeModelChangedSpy(&model, &ProductErrorModel::attributeModelChanged);
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
    model.setCurrentProduct(product);

    auto error = model.createError(ProductErrorModel::ErrorType::ConsecutiveTypeErrors);
    QCOMPARE(error->threshold(), 10);

    am.reset();
    QCOMPARE(model.attributeModel(), nullptr);
    QCOMPARE(attributeModelChangedSpy.count(), 2);
}

void ProductErrorModelTest::testVariantId()
{
    ProductErrorModel model{this};
    QCOMPARE(model.variantId(ProductErrorModel::ErrorType::ConsecutiveTypeErrors), QUuid("37E21057-EFD4-4C18-A298-BE9F804C6C04"));
    QCOMPARE(model.variantId(ProductErrorModel::ErrorType::AccumulatedTypeErrors), QUuid("C19E43C7-EBC0-4771-A701-BA102511AD9F"));
    QCOMPARE(model.variantId(ProductErrorModel::ErrorType::ConsecutiveSeamErrors), QUuid("753EA75B-4B82-418E-B1F2-0EEA0D4C0D50"));
    QCOMPARE(model.variantId(ProductErrorModel::ErrorType::AccumulatedSeamErrors), QUuid("B47EBF8B-0D37-48BD-8A6A-0D6E09885C1F"));
}

void ProductErrorModelTest::testName()
{
    ProductErrorModel model{this};
    QCOMPARE(model.name(ProductErrorModel::ErrorType::ConsecutiveTypeErrors), QStringLiteral("Consecutive Type Errors"));
    QCOMPARE(model.name(ProductErrorModel::ErrorType::AccumulatedTypeErrors), QStringLiteral("Accumulated Type Errors"));
    QCOMPARE(model.name(ProductErrorModel::ErrorType::ConsecutiveSeamErrors), QStringLiteral("Consecutive Seam Errors"));
    QCOMPARE(model.name(ProductErrorModel::ErrorType::AccumulatedSeamErrors), QStringLiteral("Accumulated Seam Errors"));
}

void ProductErrorModelTest::testNameFromId()
{
    ProductErrorModel model{this};
    QCOMPARE(model.nameFromId("37E21057-EFD4-4C18-A298-BE9F804C6C04"), QStringLiteral("Consecutive Type Errors"));
    QCOMPARE(model.nameFromId("C19E43C7-EBC0-4771-A701-BA102511AD9F"), QStringLiteral("Accumulated Type Errors"));
    QCOMPARE(model.nameFromId("753EA75B-4B82-418E-B1F2-0EEA0D4C0D50"), QStringLiteral("Consecutive Seam Errors"));
    QCOMPARE(model.nameFromId("B47EBF8B-0D37-48BD-8A6A-0D6E09885C1F"), QStringLiteral("Accumulated Seam Errors"));
    QCOMPARE(model.nameFromId(QUuid::createUuid()), QStringLiteral(""));
}

void ProductErrorModelTest::testIsTypeError()
{
    ProductErrorModel model{this};
    QCOMPARE(model.isTypeError("37E21057-EFD4-4C18-A298-BE9F804C6C04"), true);
    QCOMPARE(model.isTypeError("C19E43C7-EBC0-4771-A701-BA102511AD9F"), true);
    QCOMPARE(model.isTypeError("753EA75B-4B82-418E-B1F2-0EEA0D4C0D50"), false);
    QCOMPARE(model.isTypeError("B47EBF8B-0D37-48BD-8A6A-0D6E09885C1F"), false);
}

void ProductErrorModelTest::testCreateError()
{
    ProductErrorModel model{this};
    QSignalSpy rowsInsertedSpy(&model, &ProductErrorModel::rowsInserted);
    QVERIFY(rowsInsertedSpy.isValid());

    auto error = model.createError(ProductErrorModel::ErrorType::ConsecutiveTypeErrors);
    QVERIFY(!error);

    const auto product = new Product(QUuid::createUuid(), this);
    model.setCurrentProduct(product);

    QCOMPARE(model.rowCount(), 0);
    QCOMPARE(product->overlyingErrors().size(), 0);

    auto error1 = model.createError(ProductErrorModel::ErrorType::ConsecutiveTypeErrors);
    QCOMPARE(error1->variantId(), QUuid{"37E21057-EFD4-4C18-A298-BE9F804C6C04"});
    QCOMPARE(model.rowCount(), 1);
    QCOMPARE(product->overlyingErrors().size(), 1);
    QCOMPARE(rowsInsertedSpy.count(), 1);

    auto error2 = model.createError(ProductErrorModel::ErrorType::AccumulatedTypeErrors);
    QCOMPARE(error2->variantId(), QUuid{"C19E43C7-EBC0-4771-A701-BA102511AD9F"});
    QCOMPARE(model.rowCount(), 2);
    QCOMPARE(product->overlyingErrors().size(), 2);
    QCOMPARE(rowsInsertedSpy.count(), 2);

    auto error3 = model.createError(ProductErrorModel::ErrorType::ConsecutiveSeamErrors);
    QCOMPARE(error3->variantId(), QUuid{"753EA75B-4B82-418E-B1F2-0EEA0D4C0D50"});
    QCOMPARE(model.rowCount(), 3);
    QCOMPARE(product->overlyingErrors().size(), 3);
    QCOMPARE(rowsInsertedSpy.count(), 3);

    auto error4 = model.createError(ProductErrorModel::ErrorType::AccumulatedSeamErrors);
    QCOMPARE(error4->variantId(), QUuid{"B47EBF8B-0D37-48BD-8A6A-0D6E09885C1F"});
    QCOMPARE(model.rowCount(), 4);
    QCOMPARE(product->overlyingErrors().size(), 4);
    QCOMPARE(rowsInsertedSpy.count(), 4);
}

void ProductErrorModelTest::testRemoveError()
{
    ProductErrorModel model{this};
    QSignalSpy rowsRemovedSpy(&model, &ProductErrorModel::rowsRemoved);
    QVERIFY(rowsRemovedSpy.isValid());

    const auto product = new Product(QUuid::createUuid(), this);
    model.setCurrentProduct(product);

    auto error1 = model.createError(ProductErrorModel::ErrorType::ConsecutiveTypeErrors);
    auto error2 = model.createError(ProductErrorModel::ErrorType::AccumulatedTypeErrors);
    auto error3 = model.createError(ProductErrorModel::ErrorType::ConsecutiveSeamErrors);
    auto error4 = model.createError(ProductErrorModel::ErrorType::AccumulatedSeamErrors);

    QCOMPARE(model.index(0).data(Qt::UserRole).value<ProductError*>(), error1);
    QCOMPARE(model.index(1).data(Qt::UserRole).value<ProductError*>(), error2);
    QCOMPARE(model.index(2).data(Qt::UserRole).value<ProductError*>(), error3);
    QCOMPARE(model.index(3).data(Qt::UserRole).value<ProductError*>(), error4);
    QCOMPARE(model.rowCount(), 4);

    model.removeError(error2);
    QCOMPARE(rowsRemovedSpy.count(), 1);
    QCOMPARE(model.rowCount(), 3);

    QCOMPARE(model.index(0).data(Qt::UserRole).value<ProductError*>(), error1);
    QCOMPARE(model.index(1).data(Qt::UserRole).value<ProductError*>(), error3);
    QCOMPARE(model.index(2).data(Qt::UserRole).value<ProductError*>(), error4);

    model.removeError(error4);
    QCOMPARE(rowsRemovedSpy.count(), 2);
    QCOMPARE(model.rowCount(), 2);

    QCOMPARE(model.index(0).data(Qt::UserRole).value<ProductError*>(), error1);
    QCOMPARE(model.index(1).data(Qt::UserRole).value<ProductError*>(), error3);

    model.removeError(error1);
    QCOMPARE(rowsRemovedSpy.count(), 3);
    QCOMPARE(model.rowCount(), 1);

    QCOMPARE(model.index(0).data(Qt::UserRole).value<ProductError*>(), error3);

    model.removeError(error3);
    QCOMPARE(rowsRemovedSpy.count(), 4);
    QCOMPARE(model.rowCount(), 0);
}

QTEST_GUILESS_MAIN(ProductErrorModelTest)
#include "productErrorModelTest.moc"
