#include <QTest>
#include <QSignalSpy>

#include "../src/measureTaskNumberValidator.h"

#include "product.h"
#include "seamSeries.h"
#include "seam.h"

using precitec::gui::MeasureTaskNumberValidator;
using precitec::storage::Product;

class MeasureTaskNumberValidatorTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testCtor();
    void testValidateSeamSeries();
    void testValidateSeam();
};

void MeasureTaskNumberValidatorTest::testCtor()
{
    MeasureTaskNumberValidator validator{};
    QVERIFY(!validator.measureTask());
    QString test = QStringLiteral("1");
    int pos;
    QCOMPARE(validator.validate(test, pos), QValidator::Invalid);
}

void MeasureTaskNumberValidatorTest::testValidateSeamSeries()
{
    Product product{QUuid::createUuid()};
    product.createFirstSeamSeries();

    MeasureTaskNumberValidator validator{};
    QSignalSpy measureTaskChangedSpy{&validator, &MeasureTaskNumberValidator::measureTaskChanged};
    QVERIFY(measureTaskChangedSpy.isValid());
    validator.setMeasureTask(product.seamSeries().front());
    QCOMPARE(validator.measureTask(), product.seamSeries().front());
    QCOMPARE(measureTaskChangedSpy.count(), 1);
    // setting same should not change
    validator.setMeasureTask(product.seamSeries().front());
    QCOMPARE(measureTaskChangedSpy.count(), 1);

    // validate...
    // same number should be fine
    QString test = QStringLiteral("1");
    int pos;
    QCOMPARE(validator.validate(test, pos), QValidator::Acceptable);
    // 256 should be fine
    test = QStringLiteral("256");
    QCOMPARE(validator.validate(test, pos), QValidator::Acceptable);
    // 257 should be intermediate
    test = QStringLiteral("257");
    QCOMPARE(validator.validate(test, pos), QValidator::Intermediate);
    // 0 should be intermediate
    test = QStringLiteral("0");
    QCOMPARE(validator.validate(test, pos), QValidator::Intermediate);
    // negative numbers should be intermediate
    test = QStringLiteral("-1");
    QCOMPARE(validator.validate(test, pos), QValidator::Intermediate);

    // any text should be invalid
    test = QStringLiteral("test");
    QCOMPARE(validator.validate(test, pos), QValidator::Invalid);

    // 2 should be acceptable
    test = QStringLiteral("2");
    QCOMPARE(validator.validate(test, pos), QValidator::Acceptable);

    // let's create a second seam series
    product.createSeamSeries();
    // now 2 should be intermediate
    QCOMPARE(validator.validate(test, pos), QValidator::Intermediate);

    // let's remove the seamSeries
    product.destroySeamSeries(product.seamSeries().front());
    QVERIFY(measureTaskChangedSpy.wait());
    QCOMPARE(measureTaskChangedSpy.count(), 2);
    QVERIFY(!validator.measureTask());
}

void MeasureTaskNumberValidatorTest::testValidateSeam()
{
    Product product{QUuid::createUuid()};
    product.createFirstSeamSeries();
    auto seam = product.seamSeries().front()->createSeam();

    MeasureTaskNumberValidator validator{};
    QSignalSpy measureTaskChangedSpy{&validator, &MeasureTaskNumberValidator::measureTaskChanged};
    QVERIFY(measureTaskChangedSpy.isValid());
    validator.setMeasureTask(seam);
    QCOMPARE(validator.measureTask(), seam);
    QCOMPARE(measureTaskChangedSpy.count(), 1);
    // setting same should not change
    validator.setMeasureTask(seam);
    QCOMPARE(measureTaskChangedSpy.count(), 1);

    // validate...
    // same number should be fine
    QString test = QStringLiteral("1");
    int pos;
    QCOMPARE(validator.validate(test, pos), QValidator::Acceptable);
    // 256 should be fine
    test = QStringLiteral("256");
    QCOMPARE(validator.validate(test, pos), QValidator::Acceptable);
    // 257 should be intermediate
    test = QStringLiteral("257");
    QCOMPARE(validator.validate(test, pos), QValidator::Intermediate);
    // 0 should be intermediate
    test = QStringLiteral("0");
    QCOMPARE(validator.validate(test, pos), QValidator::Intermediate);
    // negative numbers should be intermediate
    test = QStringLiteral("-1");
    QCOMPARE(validator.validate(test, pos), QValidator::Intermediate);

    // any text should be invalid
    test = QStringLiteral("test");
    QCOMPARE(validator.validate(test, pos), QValidator::Invalid);

    // 2 should be acceptable
    test = QStringLiteral("2");
    QCOMPARE(validator.validate(test, pos), QValidator::Acceptable);

    // let's create a second seam
    product.seamSeries().front()->createSeam();
    // now 2 should be intermediate
    QCOMPARE(validator.validate(test, pos), QValidator::Intermediate);

    // let's remove the seamSeries
    product.seamSeries().front()->destroySeam(seam);
    QVERIFY(measureTaskChangedSpy.wait());
    QCOMPARE(measureTaskChangedSpy.count(), 2);
    QVERIFY(!validator.measureTask());
}

QTEST_GUILESS_MAIN(MeasureTaskNumberValidatorTest)
#include "measureTaskNumberValidatorTest.moc"
