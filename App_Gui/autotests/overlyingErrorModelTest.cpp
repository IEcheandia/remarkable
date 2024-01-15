#include <QTest>
#include <QSignalSpy>

#include "../src/overlyingErrorModel.h"
#include "attributeModel.h"

using precitec::gui::OverlyingErrorModel;
using precitec::storage::AttributeModel;

class OverlyingErrorModelTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testCtor();
    void testRoleNames();
    void testDisplayRole_data();
    void testDisplayRole();
    void testDescription_data();
    void testDescription();
    void testImage_data();
    void testImage();
    void testType_data();
    void testType();
    void testIsType_data();
    void testIsType();
    void testAttributeModel();
    void testVariantId();
    void testName();
    void testNameFromId();
    void testIsTypeError();
};

void OverlyingErrorModelTest::testCtor()
{
    OverlyingErrorModel control{this};
    QVERIFY(!control.attributeModel());
    QCOMPARE(control.rowCount(), 4);
}

void OverlyingErrorModelTest::testRoleNames()
{
    OverlyingErrorModel control{this};
    const auto roleNames = control.roleNames();
    QCOMPARE(roleNames.size(), 5);
    QCOMPARE(roleNames[Qt::DisplayRole], QByteArrayLiteral("name"));
    QCOMPARE(roleNames[Qt::UserRole ], QByteArrayLiteral("description"));
    QCOMPARE(roleNames[Qt::UserRole + 1], QByteArrayLiteral("image"));
    QCOMPARE(roleNames[Qt::UserRole + 2], QByteArrayLiteral("type"));
    QCOMPARE(roleNames[Qt::UserRole + 3], QByteArrayLiteral("isTypeError"));
}

void OverlyingErrorModelTest::testDisplayRole_data()
{
    QTest::addColumn<int>("row");
    QTest::addColumn<QString>("name");

    QTest::newRow("0") << 0 << QStringLiteral("Consecutive Type Errors");
    QTest::newRow("1") << 1 << QStringLiteral("Accumulated Type Errors");
    QTest::newRow("2") << 2 << QStringLiteral("Consecutive Seam Errors");
    QTest::newRow("3") << 3 << QStringLiteral("Accumulated Seam Errors");
}

void OverlyingErrorModelTest::testDisplayRole()
{
    OverlyingErrorModel control{this};

    QFETCH(int, row);
    const auto index = control.index(row, 0);
    QVERIFY(index.isValid());
    QTEST(index.data().toString(), "name");
}

void OverlyingErrorModelTest::testDescription_data()
{
    QTest::addColumn<int>("row");
    QTest::addColumn<QString>("description");

    QTest::newRow("0") << 0 << QStringLiteral("The number of <b>Consecutive Seams</b> in which a <b>Certain Type</b> of error occurs exceeds a predefined <b>Threshold</b> value");
    QTest::newRow("1") << 1 << QStringLiteral("The number of <b>Accumulated Seams</b> in which a <b>Certain Type</b> of error occurs exceeds a predefined <b>Threshold</b> value");
    QTest::newRow("2") << 2 << QStringLiteral("The number of <b>Consecutive Seams</b> in which <b>Any Type</b> of error occurs exceeds a predefined <b>Threshold</b> value");
    QTest::newRow("3") << 3 << QStringLiteral("The number of <b>Accumulated Seams</b> in which <b>Any Type</b> of error occurs exceeds a predefined <b>Threshold</b> value");
}

void OverlyingErrorModelTest::testDescription()
{
    OverlyingErrorModel control{this};

    QFETCH(int, row);
    const auto index = control.index(row, 0);
    QVERIFY(index.isValid());
    QTEST(index.data(Qt::UserRole).toString(), "description");
}

void OverlyingErrorModelTest::testImage_data()
{
    QTest::addColumn<int>("row");
    QTest::addColumn<QString>("image");

    QTest::newRow("0") << 0 << QStringLiteral("../images/error-ConsecSeamErr.png");
    QTest::newRow("1") << 1 << QStringLiteral("../images/error-AccuSeamErr.png");
    QTest::newRow("2") << 2 << QStringLiteral("../images/error-ConsecSeamErr.png");
    QTest::newRow("3") << 3 << QStringLiteral("../images/error-AccuSeamErr.png");
}

void OverlyingErrorModelTest::testImage()
{
    OverlyingErrorModel control{this};

    QFETCH(int, row);
    const auto index = control.index(row, 0);
    QVERIFY(index.isValid());
    QTEST(index.data(Qt::UserRole + 1).toString(), "image");
}

void OverlyingErrorModelTest::testType_data()
{
    QTest::addColumn<int>("row");
    QTest::addColumn<OverlyingErrorModel::ErrorType>("type");

    QTest::newRow("0") << 0 << OverlyingErrorModel::ErrorType::ConsecutiveTypeErrors;
    QTest::newRow("1") << 1 << OverlyingErrorModel::ErrorType::AccumulatedTypeErrors;
    QTest::newRow("2") << 2 << OverlyingErrorModel::ErrorType::ConsecutiveSeamErrors;
    QTest::newRow("3") << 3 << OverlyingErrorModel::ErrorType::AccumulatedSeamErrors;
}

void OverlyingErrorModelTest::testType()
{
    OverlyingErrorModel control{this};

    QFETCH(int, row);
    const auto index = control.index(row, 0);
    QVERIFY(index.isValid());
    QTEST(index.data(Qt::UserRole + 2).value<OverlyingErrorModel::ErrorType>(), "type");
}

void OverlyingErrorModelTest::testIsType_data()
{
    QTest::addColumn<int>("row");
    QTest::addColumn<bool>("isTypeError");

    QTest::newRow("0") << 0 << true;
    QTest::newRow("1") << 1 << true;
    QTest::newRow("2") << 2 << false;
    QTest::newRow("3") << 3 << false;
}

void OverlyingErrorModelTest::testIsType()
{
    OverlyingErrorModel control{this};

    QFETCH(int, row);
    const auto index = control.index(row, 0);
    QVERIFY(index.isValid());
    QTEST(index.data(Qt::UserRole + 3).toBool(), "isTypeError");
}

void OverlyingErrorModelTest::testAttributeModel()
{
    OverlyingErrorModel control{this};
    QSignalSpy attributeModelChangedSpy(&control, &OverlyingErrorModel::attributeModelChanged);
    QVERIFY(attributeModelChangedSpy.isValid());

    std::unique_ptr<AttributeModel> am = std::make_unique<AttributeModel>();
    control.setAttributeModel(am.get());
    QCOMPARE(control.attributeModel(), am.get());
    QCOMPARE(attributeModelChangedSpy.count(), 1);

    control.setAttributeModel(am.get());
    QCOMPARE(attributeModelChangedSpy.count(), 1);

    am.reset();
    QCOMPARE(control.attributeModel(), nullptr);
    QCOMPARE(attributeModelChangedSpy.count(), 2);
}

void OverlyingErrorModelTest::testVariantId()
{
    OverlyingErrorModel control{this};
    QCOMPARE(control.variantId(OverlyingErrorModel::ErrorType::ConsecutiveTypeErrors), QUuid("37E21057-EFD4-4C18-A298-BE9F804C6C04"));
    QCOMPARE(control.variantId(OverlyingErrorModel::ErrorType::AccumulatedTypeErrors), QUuid("C19E43C7-EBC0-4771-A701-BA102511AD9F"));
    QCOMPARE(control.variantId(OverlyingErrorModel::ErrorType::ConsecutiveSeamErrors), QUuid("753EA75B-4B82-418E-B1F2-0EEA0D4C0D50"));
    QCOMPARE(control.variantId(OverlyingErrorModel::ErrorType::AccumulatedSeamErrors), QUuid("B47EBF8B-0D37-48BD-8A6A-0D6E09885C1F"));
}

void OverlyingErrorModelTest::testName()
{
    OverlyingErrorModel control{this};
    QCOMPARE(control.name(OverlyingErrorModel::ErrorType::ConsecutiveTypeErrors), QStringLiteral("Consecutive Type Errors"));
    QCOMPARE(control.name(OverlyingErrorModel::ErrorType::AccumulatedTypeErrors), QStringLiteral("Accumulated Type Errors"));
    QCOMPARE(control.name(OverlyingErrorModel::ErrorType::ConsecutiveSeamErrors), QStringLiteral("Consecutive Seam Errors"));
    QCOMPARE(control.name(OverlyingErrorModel::ErrorType::AccumulatedSeamErrors), QStringLiteral("Accumulated Seam Errors"));
}

void OverlyingErrorModelTest::testNameFromId()
{
    OverlyingErrorModel control{this};
    QCOMPARE(control.nameFromId("37E21057-EFD4-4C18-A298-BE9F804C6C04"), QStringLiteral("Consecutive Type Errors"));
    QCOMPARE(control.nameFromId("C19E43C7-EBC0-4771-A701-BA102511AD9F"), QStringLiteral("Accumulated Type Errors"));
    QCOMPARE(control.nameFromId("753EA75B-4B82-418E-B1F2-0EEA0D4C0D50"), QStringLiteral("Consecutive Seam Errors"));
    QCOMPARE(control.nameFromId("B47EBF8B-0D37-48BD-8A6A-0D6E09885C1F"), QStringLiteral("Accumulated Seam Errors"));
    QCOMPARE(control.nameFromId(QUuid::createUuid()), QStringLiteral(""));
}

void OverlyingErrorModelTest::testIsTypeError()
{
    OverlyingErrorModel control{this};
    QCOMPARE(control.isTypeError("37E21057-EFD4-4C18-A298-BE9F804C6C04"), true);
    QCOMPARE(control.isTypeError("C19E43C7-EBC0-4771-A701-BA102511AD9F"), true);
    QCOMPARE(control.isTypeError("753EA75B-4B82-418E-B1F2-0EEA0D4C0D50"), false);
    QCOMPARE(control.isTypeError("B47EBF8B-0D37-48BD-8A6A-0D6E09885C1F"), false);
}

QTEST_GUILESS_MAIN(OverlyingErrorModelTest)
#include "overlyingErrorModelTest.moc"

