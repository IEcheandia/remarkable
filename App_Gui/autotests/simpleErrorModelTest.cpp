#include <QTest>
#include <QSignalSpy>

#include "../src/simpleErrorModel.h"
#include "../src/errorGroupModel.h"
#include "attributeModel.h"
#include "seam.h"
#include "product.h"
#include "seamError.h"
#include "intervalError.h"

using precitec::gui::SimpleErrorModel;
using precitec::gui::ErrorGroupModel;
using precitec::storage::AttributeModel;
using precitec::storage::Product;
using precitec::storage::Seam;
using precitec::storage::SeamError;

class SimpleErrorModelTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testCtor();
    void testRoleNames();
    void testDisplayRole_data();
    void testDisplayRole();
    void testTitle_data();
    void testTitle();
    void testDescription_data();
    void testDescription();
    void testImage_data();
    void testImage();
    void testType_data();
    void testType();
    void testGroup_data();
    void testGroup();
    void testBoundary_data();
    void testBoundary();
    void testSetAttributeModel();
    void testSetCurrentSeam();
    void testAddError();
    void testAddIntervalError();
};

void SimpleErrorModelTest::testCtor()
{
    SimpleErrorModel control{this};
    QVERIFY(!control.currentSeam());
    QVERIFY(!control.attributeModel());
    QCOMPARE(control.rowCount(), 14);
}

void SimpleErrorModelTest::testRoleNames()
{
    SimpleErrorModel control{this};
    const auto roleNames = control.roleNames();
    QCOMPARE(roleNames.size(), 7);
    QCOMPARE(roleNames[Qt::DisplayRole], QByteArrayLiteral("name"));
    QCOMPARE(roleNames[Qt::UserRole], QByteArrayLiteral("title"));
    QCOMPARE(roleNames[Qt::UserRole + 1], QByteArrayLiteral("description"));
    QCOMPARE(roleNames[Qt::UserRole + 2], QByteArrayLiteral("image"));
    QCOMPARE(roleNames[Qt::UserRole + 3], QByteArrayLiteral("type"));
    QCOMPARE(roleNames[Qt::UserRole + 4], QByteArrayLiteral("group"));
    QCOMPARE(roleNames[Qt::UserRole + 5], QByteArrayLiteral("boundary"));
}

void SimpleErrorModelTest::testDisplayRole_data()
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
    QTest::newRow("12")  <<  12 << QStringLiteral("Dual Length Outside Static Boundary Failure");
    QTest::newRow("13")  <<  13 << QStringLiteral("Dual Length Outside Reference Boundary Failure");
}

void SimpleErrorModelTest::testDisplayRole()
{
    SimpleErrorModel control{this};

    QFETCH(int, row);
    const auto index = control.index(row, 0);
    QVERIFY(index.isValid());
    QTEST(index.data().toString(), "name");
}

void SimpleErrorModelTest::testTitle_data()
{
    QTest::addColumn<int>("row");
    QTest::addColumn<QString>("title");

    QTest::newRow("0")  <<  0 << QStringLiteral("Static Boundary Failure");
    QTest::newRow("1")  <<  1 << QStringLiteral("Reference Boundary Failure");
    QTest::newRow("2")  <<  2 << QStringLiteral("Accumulated Static Boundary Failure");
    QTest::newRow("3")  <<  3 << QStringLiteral("Accumulated Reference Boundary Failure");
    QTest::newRow("4")  <<  4 << QStringLiteral("Static Boundary Inside Failure");
    QTest::newRow("5")  <<  5 << QStringLiteral("Reference Boundary Inside Failure");
    QTest::newRow("6")  <<  6 << QStringLiteral("Static Boundary Area Failure");
    QTest::newRow("7")  <<  7 << QStringLiteral("Reference Boundary Area Failure");
    QTest::newRow("8")  <<  8 << QStringLiteral("Accumulated Static Boundary Area Failure");
    QTest::newRow("9")  <<  9 << QStringLiteral("Accumulated Reference Boundary Area Failure");
    QTest::newRow("10") <<  10 << QStringLiteral("Static Boundary Peak Failure");
    QTest::newRow("11") <<  11 << QStringLiteral("Reference Boundary Peak Failure");
    QTest::newRow("12") <<  12 << QStringLiteral("Static Boundary Dual Outlier Failure");
    QTest::newRow("13") <<  13 << QStringLiteral("Reference Boundary Dual Outlier Failure");
}

void SimpleErrorModelTest::testTitle()
{
    SimpleErrorModel control{this};

    QFETCH(int, row);
    const auto index = control.index(row, 0);
    QVERIFY(index.isValid());
    QTEST(index.data(Qt::UserRole).toString(), "title");
}

void SimpleErrorModelTest::testDescription_data()
{
    QTest::addColumn<int>("row");
    QTest::addColumn<QString>("description");

    QTest::newRow("0") << 0 << QStringLiteral("The length of a single, continuous sector outside the range defined by <b>Min</b> and <b>Max</b> is greater than the <b>Threshold</b> value");
    QTest::newRow("1") << 1 << QStringLiteral("The length of a single, continuous sector outside of a <b>Reference Curve</b> is greater than the <b>Threshold</b> value");
    QTest::newRow("2") << 2 << QStringLiteral("The accumulated length of all sectors outside the range defined by <b>Min</b> and <b>Max</b> is greater than the <b>Threshold</b> value");
    QTest::newRow("3") << 3 << QStringLiteral("The accumulated length of all sectors outside of a <b>Reference Curve</b> is greater than the <b>Threshold</b> value");
    QTest::newRow("4") << 4 << QStringLiteral("The length of every single, continuous sector inside the range defined by <b>Min</b> and <b>Max</b> is shorter than the <b>Threshold</b> value");
    QTest::newRow("5") << 5 << QStringLiteral("The length of every single, continuous sector inside of a <b>Reference Curve</b> is shorter than the <b>Threshold</b> value");
    QTest::newRow("6") << 6 << QStringLiteral("The area of every single, continuous sector outside the range defined by <b>Min</b> and <b>Max</b> is greater than the <b>Threshold</b> value");
    QTest::newRow("7") << 7 << QStringLiteral("The area of a single, continuous sector outside of a <b>Reference Curve</b> is greater than the <b>Threshold</b> value");
    QTest::newRow("8") << 8 << QStringLiteral("The accumulated area of all sectors outside the range defined by <b>Min</b> and <b>Max</b> is greater than the <b>Threshold</b> value");
    QTest::newRow("9") << 9 << QStringLiteral("The accumulated area of all sectors outside of a <b>Reference Curve</b> is greater than the <b>Threshold</b> value");
    QTest::newRow("10") << 10 << QStringLiteral("A single value is detected outside the range defined by <b>Min</b> and <b>Max</b>");
    QTest::newRow("11") << 11 << QStringLiteral("A single value is detected outside of a <b>Reference Curve</b>");
    QTest::newRow("12") << 12 << QStringLiteral("Two sectors over <b>Threshold A</b> length outside the range defined by <b>Min</b> and <b>Max</b> are less than <b>Threshold B</b> length after another");
    QTest::newRow("13") << 13 << QStringLiteral("Two sectors over <b>Threshold A</b> length outside of a <b>Reference Curve</b> are less than <b>Threshold B</b> length after another");
}

void SimpleErrorModelTest::testDescription()
{
    SimpleErrorModel control{this};

    QFETCH(int, row);
    const auto index = control.index(row, 0);
    QVERIFY(index.isValid());
    QTEST(index.data(Qt::UserRole + 1).toString(), "description");
}

void SimpleErrorModelTest::testImage_data()
{
    QTest::addColumn<int>("row");
    QTest::addColumn<QString>("image");

    QTest::newRow("0") << 0 << QStringLiteral("../images/error-LOB-StatBoundFail.png");
    QTest::newRow("1") << 1 << QStringLiteral("../images/error-LOB-RefBoundFail.png");
    QTest::newRow("2") << 2 << QStringLiteral("../images/error-LOB-AccStatBoundFail.png");
    QTest::newRow("3") << 3 << QStringLiteral("../images/error-LOB-AccRefBoundFail.png");
    QTest::newRow("4") << 4 << QStringLiteral("../images/error-LIB-StatBoundFail.png");
    QTest::newRow("5") << 5 << QStringLiteral("../images/error-LIB-RefBoundFail.png");
    QTest::newRow("6") << 6 << QStringLiteral("../images/error-AOB-StatBoundFail.png");
    QTest::newRow("7") << 7 << QStringLiteral("../images/error-AOB-RefBoundFail.png");
    QTest::newRow("8") << 8 << QStringLiteral("../images/error-AOB-AccStatBoundFail.png");
    QTest::newRow("9") << 9 << QStringLiteral("../images/error-AOB-AccRefBoundFail.png");
    QTest::newRow("10") << 10 << QStringLiteral("../images/error-POB-StatBoundFail.png");
    QTest::newRow("11") << 11 << QStringLiteral("../images/error-POB-RefBoundFail.png");
    QTest::newRow("12") << 12 << QStringLiteral("../images/error-LOB-DualStatBoundFail.png");
    QTest::newRow("13") << 13 << QStringLiteral("../images/error-LOB-DualRefBoundFail.png");
}

void SimpleErrorModelTest::testImage()
{
    SimpleErrorModel control{this};

    QFETCH(int, row);
    const auto index = control.index(row, 0);
    QVERIFY(index.isValid());
    QTEST(index.data(Qt::UserRole + 2).toString(), "image");
}

void SimpleErrorModelTest::testType_data()
{
    QTest::addColumn<int>("row");
    QTest::addColumn<SimpleErrorModel::ErrorType>("type");

    QTest::newRow("0") << 0 << SimpleErrorModel::ErrorType::LengthOutsideStaticBoundary;
    QTest::newRow("1") << 1 << SimpleErrorModel::ErrorType::LengthOutsideReferenceBoundary;
    QTest::newRow("2") << 2 << SimpleErrorModel::ErrorType::AccumulatedLengthOutsideStaticBoundary;
    QTest::newRow("3") << 3 << SimpleErrorModel::ErrorType::AccumulatedLengthOutsideReferenceBoundary;
    QTest::newRow("4") << 4 << SimpleErrorModel::ErrorType::LengthInsideStaticBoundary;
    QTest::newRow("5") << 5 << SimpleErrorModel::ErrorType::LengthInsideReferenceBoundary;
    QTest::newRow("6") << 6 << SimpleErrorModel::ErrorType::AreaStaticBoundary;
    QTest::newRow("7") << 7 << SimpleErrorModel::ErrorType::AreaReferenceBoundary;
    QTest::newRow("8") << 8 << SimpleErrorModel::ErrorType::AccumulatedAreaStaticBoundary;
    QTest::newRow("9") << 9 << SimpleErrorModel::ErrorType::AccumulatedAreaReferenceBoundary;
    QTest::newRow("10") << 10 << SimpleErrorModel::ErrorType::PeakStaticBoundary;
    QTest::newRow("11") << 11 << SimpleErrorModel::ErrorType::PeakReferenceBoundary;
    QTest::newRow("12") << 12 << SimpleErrorModel::ErrorType::DualOutlierStaticBoundary;
    QTest::newRow("13") << 13 << SimpleErrorModel::ErrorType::DualOutlierReferenceBoundary;
}

void SimpleErrorModelTest::testType()
{
    SimpleErrorModel control{this};

    QFETCH(int, row);
    const auto index = control.index(row, 0);
    QVERIFY(index.isValid());
    QTEST(index.data(Qt::UserRole + 3).value<SimpleErrorModel::ErrorType>(), "type");
}

void SimpleErrorModelTest::testGroup_data()
{
    QTest::addColumn<int>("row");
    QTest::addColumn<ErrorGroupModel::ErrorGroup>("group");

    QTest::newRow("0") << 0 << ErrorGroupModel::ErrorGroup::LengthOutsideBoundary;
    QTest::newRow("1") << 1 << ErrorGroupModel::ErrorGroup::LengthOutsideBoundary;
    QTest::newRow("2") << 2 << ErrorGroupModel::ErrorGroup::LengthOutsideBoundary;
    QTest::newRow("3") << 3 << ErrorGroupModel::ErrorGroup::LengthOutsideBoundary;
    QTest::newRow("4") << 4 << ErrorGroupModel::ErrorGroup::LengthInsideBoundary;
    QTest::newRow("5") << 5 << ErrorGroupModel::ErrorGroup::LengthInsideBoundary;
    QTest::newRow("6") << 6 << ErrorGroupModel::ErrorGroup::AreaOutsideBoundary;
    QTest::newRow("7") << 7 << ErrorGroupModel::ErrorGroup::AreaOutsideBoundary;
    QTest::newRow("8") << 8 << ErrorGroupModel::ErrorGroup::AreaOutsideBoundary;
    QTest::newRow("9") << 9 << ErrorGroupModel::ErrorGroup::AreaOutsideBoundary;
    QTest::newRow("10") << 10 << ErrorGroupModel::ErrorGroup::PeakOutsideBoundary;
    QTest::newRow("11") << 11 << ErrorGroupModel::ErrorGroup::PeakOutsideBoundary;
    QTest::newRow("12") << 12 << ErrorGroupModel::ErrorGroup::LengthOutsideBoundary;
    QTest::newRow("13") << 13 << ErrorGroupModel::ErrorGroup::LengthOutsideBoundary;
}

void SimpleErrorModelTest::testGroup()
{
    SimpleErrorModel control{this};

    QFETCH(int, row);
    const auto index = control.index(row, 0);
    QVERIFY(index.isValid());
    QTEST(index.data(Qt::UserRole + 4).value<ErrorGroupModel::ErrorGroup>(), "group");
}

void SimpleErrorModelTest::testBoundary_data()
{
    QTest::addColumn<int>("row");
    QTest::addColumn<SeamError::BoundaryType>("boundary");

    QTest::newRow("0") << 0 << SeamError::BoundaryType::Static;
    QTest::newRow("1") << 1 << SeamError::BoundaryType::Reference;
    QTest::newRow("2") << 2 << SeamError::BoundaryType::Static;
    QTest::newRow("3") << 3 << SeamError::BoundaryType::Reference;
    QTest::newRow("4") << 4 << SeamError::BoundaryType::Static;
    QTest::newRow("5") << 5 << SeamError::BoundaryType::Reference;
    QTest::newRow("6") << 6 << SeamError::BoundaryType::Static;
    QTest::newRow("7") << 7 << SeamError::BoundaryType::Reference;
    QTest::newRow("8") << 8 << SeamError::BoundaryType::Static;
    QTest::newRow("9") << 9 << SeamError::BoundaryType::Reference;
    QTest::newRow("10") << 10 << SeamError::BoundaryType::Static;
    QTest::newRow("11") << 11 << SeamError::BoundaryType::Reference;
    QTest::newRow("12") << 12 << SeamError::BoundaryType::Static;
    QTest::newRow("13") << 13 << SeamError::BoundaryType::Reference;
}

void SimpleErrorModelTest::testBoundary()
{
    SimpleErrorModel control{this};

    QFETCH(int, row);
    const auto index = control.index(row, 0);
    QVERIFY(index.isValid());
    QTEST(index.data(Qt::UserRole + 5).value<SeamError::BoundaryType>(), "boundary");
}

void SimpleErrorModelTest::testSetAttributeModel()
{
    SimpleErrorModel control{this};
    QSignalSpy attributeModelChangedSpy(&control, &SimpleErrorModel::attributeModelChanged);
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

    auto error = control.addError(SimpleErrorModel::ErrorType::AccumulatedLengthOutsideStaticBoundary);
    QVERIFY(error);
    QCOMPARE(error->max(), 3);

    auto interval_error = control.addIntervalError(SimpleErrorModel::ErrorType::AccumulatedLengthOutsideStaticBoundary);
    QVERIFY(interval_error);
    QCOMPARE(interval_error->max(0), 3);
    QCOMPARE(interval_error->max(1), 3);
    QCOMPARE(interval_error->max(2), 3);

    am.reset();
    QCOMPARE(control.attributeModel(), nullptr);
    QCOMPARE(attributeModelChangedSpy.count(), 2);
}

void SimpleErrorModelTest::testSetCurrentSeam()
{
    SimpleErrorModel control{this};

    QSignalSpy seamChangedSpy(&control, &SimpleErrorModel::currentSeamChanged);
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

void SimpleErrorModelTest::testAddError()
{
    SimpleErrorModel control{this};

    auto nullError = control.addError(SimpleErrorModel::ErrorType::LengthOutsideStaticBoundary);
    QVERIFY(!nullError);

    Product p{QUuid::createUuid()};
    p.createFirstSeamSeries();
    auto s = p.createSeam();
    QVERIFY(s);
    control.setCurrentSeam(s);

    auto failureConnected = control.addError(SimpleErrorModel::ErrorType::LengthOutsideStaticBoundary);
    QVERIFY(failureConnected);
    QCOMPARE(failureConnected->name(), QStringLiteral("Length Outside Static Boundary Failure"));
    QCOMPARE(failureConnected->variantId(), QUuid{"3B5FE50F-6FD5-4FBC-BD78-06B892E1F97D"});

    auto failureAll = control.addError(SimpleErrorModel::ErrorType::AccumulatedLengthOutsideStaticBoundary);
    QVERIFY(failureAll);
    QCOMPARE(failureAll->name(), QStringLiteral("Accumulated Length Outside Static Boundary Failure"));
    QCOMPARE(failureAll->variantId(), QUuid{"CEDEACB4-D4BB-4FDC-945A-BDC54E5848A6"});

    auto envelopeConnected = control.addError(SimpleErrorModel::ErrorType::LengthOutsideReferenceBoundary);
    QVERIFY(envelopeConnected);
    QCOMPARE(envelopeConnected->name(), QStringLiteral("Length Outside Reference Boundary Failure"));
    QCOMPARE(envelopeConnected->variantId(), QUuid{"5EB04560-2641-4E64-A016-14207E59A370"});

    auto envelopeAll = control.addError(SimpleErrorModel::ErrorType::AccumulatedLengthOutsideReferenceBoundary);
    QVERIFY(envelopeAll);
    QCOMPARE(envelopeAll->name(), QStringLiteral("Accumulated Length Outside Reference Boundary Failure"));
    QCOMPARE(envelopeAll->variantId(), QUuid{"F8F4E0A8-D259-40F9-B134-68AA24E0A06C"});

    auto inverted = control.addError(SimpleErrorModel::ErrorType::LengthInsideStaticBoundary);
    QVERIFY(inverted);
    QCOMPARE(inverted->name(), QStringLiteral("Length Inside Static Boundary Failure"));
    QCOMPARE(inverted->variantId(), QUuid{"3AF9EF6A-A4E9-4234-8BA5-7B42D3E58B2C"});
    
    auto invertedEnvelope = control.addError(SimpleErrorModel::ErrorType::LengthInsideReferenceBoundary);
    QVERIFY(invertedEnvelope);
    QCOMPARE(invertedEnvelope->name(), QStringLiteral("Length Inside Reference Boundary Failure"));
    QCOMPARE(invertedEnvelope->variantId(), QUuid{"4A6AE9B0-3A1A-427F-8D58-2D0205452377"});
    
    auto dual = control.addError(SimpleErrorModel::ErrorType::DualOutlierStaticBoundary);
    QVERIFY(dual);
    QCOMPARE(dual->name(), QStringLiteral("Dual Length Outside Static Boundary Failure"));
    QCOMPARE(dual->variantId(), QUuid{"55DCC3D9-FE50-4792-8E27-460AADDDD09F"});
    
    auto dualEnvelope = control.addError(SimpleErrorModel::ErrorType::DualOutlierReferenceBoundary);
    QVERIFY(dualEnvelope);
    QCOMPARE(dualEnvelope->name(), QStringLiteral("Dual Length Outside Reference Boundary Failure"));
    QCOMPARE(dualEnvelope->variantId(), QUuid{"C0C80DA1-4E9D-4EC0-859A-8D43A0674571"});
}

void SimpleErrorModelTest::testAddIntervalError()
{
    SimpleErrorModel control{this};

    auto nullError = control.addIntervalError(SimpleErrorModel::ErrorType::LengthOutsideStaticBoundary);
    QVERIFY(!nullError);

    Product p{QUuid::createUuid()};
    p.createFirstSeamSeries();
    auto s = p.createSeam();
    QVERIFY(s);
    control.setCurrentSeam(s);

    auto failureConnected = control.addIntervalError(SimpleErrorModel::ErrorType::LengthOutsideStaticBoundary);
    QVERIFY(failureConnected);
    QCOMPARE(failureConnected->name(), QStringLiteral("Length Outside Static Boundary Failure"));
    QCOMPARE(failureConnected->variantId(), QUuid{"3B5FE50F-6FD5-4FBC-BD78-06B892E1F97D"});

    auto failureAll = control.addIntervalError(SimpleErrorModel::ErrorType::AccumulatedLengthOutsideStaticBoundary);
    QVERIFY(failureAll);
    QCOMPARE(failureAll->name(), QStringLiteral("Accumulated Length Outside Static Boundary Failure"));
    QCOMPARE(failureAll->variantId(), QUuid{"CEDEACB4-D4BB-4FDC-945A-BDC54E5848A6"});

    auto envelopeConnected = control.addIntervalError(SimpleErrorModel::ErrorType::LengthOutsideReferenceBoundary);
    QVERIFY(envelopeConnected);
    QCOMPARE(envelopeConnected->name(), QStringLiteral("Length Outside Reference Boundary Failure"));
    QCOMPARE(envelopeConnected->variantId(), QUuid{"5EB04560-2641-4E64-A016-14207E59A370"});

    auto envelopeAll = control.addIntervalError(SimpleErrorModel::ErrorType::AccumulatedLengthOutsideReferenceBoundary);
    QVERIFY(envelopeAll);
    QCOMPARE(envelopeAll->name(), QStringLiteral("Accumulated Length Outside Reference Boundary Failure"));
    QCOMPARE(envelopeAll->variantId(), QUuid{"F8F4E0A8-D259-40F9-B134-68AA24E0A06C"});

    auto inverted = control.addIntervalError(SimpleErrorModel::ErrorType::LengthInsideStaticBoundary);
    QVERIFY(inverted);
    QCOMPARE(inverted->name(), QStringLiteral("Length Inside Static Boundary Failure"));
    QCOMPARE(inverted->variantId(), QUuid{"3AF9EF6A-A4E9-4234-8BA5-7B42D3E58B2C"});
    
    auto invertedEnvelope = control.addIntervalError(SimpleErrorModel::ErrorType::LengthInsideReferenceBoundary);
    QVERIFY(invertedEnvelope);
    QCOMPARE(invertedEnvelope->name(), QStringLiteral("Length Inside Reference Boundary Failure"));
    QCOMPARE(invertedEnvelope->variantId(), QUuid{"4A6AE9B0-3A1A-427F-8D58-2D0205452377"});
    
    auto dual = control.addIntervalError(SimpleErrorModel::ErrorType::DualOutlierStaticBoundary);
    QVERIFY(dual);
    QCOMPARE(dual->name(), QStringLiteral("Dual Length Outside Static Boundary Failure"));
    QCOMPARE(dual->variantId(), QUuid{"55DCC3D9-FE50-4792-8E27-460AADDDD09F"});
    
    auto dualEnvelope = control.addIntervalError(SimpleErrorModel::ErrorType::DualOutlierReferenceBoundary);
    QVERIFY(dualEnvelope);
    QCOMPARE(dualEnvelope->name(), QStringLiteral("Dual Length Outside Reference Boundary Failure"));
    QCOMPARE(dualEnvelope->variantId(), QUuid{"C0C80DA1-4E9D-4EC0-859A-8D43A0674571"});
}

QTEST_GUILESS_MAIN(SimpleErrorModelTest)
#include "simpleErrorModelTest.moc"
