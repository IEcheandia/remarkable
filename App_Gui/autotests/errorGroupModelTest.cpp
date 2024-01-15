#include <QTest>

#include "../src/errorGroupModel.h"

using precitec::gui::ErrorGroupModel;

class ErrorGroupModelTest : public QObject
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
};

void ErrorGroupModelTest::testCtor()
{
    ErrorGroupModel model{this};
    QCOMPARE(model.rowCount(), 4);
}

void ErrorGroupModelTest::testRoleNames()
{
    ErrorGroupModel model{this};
    const auto roleNames = model.roleNames();
    QCOMPARE(roleNames.size(), 4);
    QCOMPARE(roleNames[Qt::DisplayRole], QByteArrayLiteral("name"));
    QCOMPARE(roleNames[Qt::UserRole], QByteArrayLiteral("description"));
    QCOMPARE(roleNames[Qt::UserRole + 1], QByteArrayLiteral("image"));
    QCOMPARE(roleNames[Qt::UserRole + 2], QByteArrayLiteral("type"));
}

void ErrorGroupModelTest::testDisplayRole_data()
{
    QTest::addColumn<int>("row");
    QTest::addColumn<QString>("name");

    QTest::newRow("0")  <<  0 << QStringLiteral("Length Outside Boundary");
    QTest::newRow("1")  <<  1 << QStringLiteral("Length Inside Boundary");
    QTest::newRow("2")  <<  2 << QStringLiteral("Area Outside Boundary");
    QTest::newRow("3")  <<  3 << QStringLiteral("Peak Outside Boundary");

}

void ErrorGroupModelTest::testDisplayRole()
{
    ErrorGroupModel model{this};

    QFETCH(int, row);
    const auto index = model.index(row, 0);
    QVERIFY(index.isValid());
    QTEST(index.data().toString(), "name");
}

void ErrorGroupModelTest::testDescription_data()
{
    QTest::addColumn<int>("row");
    QTest::addColumn<QString>("description");

    QTest::newRow("0")  <<  0 << QStringLiteral("Verifies that the <b>length</b> of a section or a sum of sections with values <b>outside</b> a set boundary range is <b>below</b> a given <b>threshold</b> value");
    QTest::newRow("1")  <<  1 << QStringLiteral("Verifies that the <b>length</b> of a section with values <b>within</b> a set boundary range is <b>above</b> a given <b>threshold</b> value");
    QTest::newRow("2")  <<  2 << QStringLiteral("Verifies that the <b>area</b> of a section or a sum of sections with values <b>outside</b> a set boundary range is <b>below</b> a given <b>threshold</b> value");
    QTest::newRow("3")  <<  3 << QStringLiteral("Verifies that no result <b>value</b> lies more than a given <b>threshold</b> value <b>outside</b> a set boundary range");
}

void ErrorGroupModelTest::testDescription()
{
    ErrorGroupModel model{this};

    QFETCH(int, row);
    const auto index = model.index(row, 0);
    QVERIFY(index.isValid());
    QTEST(index.data(Qt::UserRole).toString(), "description");
}

void ErrorGroupModelTest::testImage_data()
{
    QTest::addColumn<int>("row");
    QTest::addColumn<QString>("image");

    QTest::newRow("0")  <<  0 << QStringLiteral("../images/error-LOB-StatBoundFail.png");
    QTest::newRow("1")  <<  1 << QStringLiteral("../images/error-LIB-StatBoundFail.png");
    QTest::newRow("2")  <<  2 << QStringLiteral("../images/error-AOB-StatBoundFail.png");
    QTest::newRow("3")  <<  3 << QStringLiteral("../images/error-POB-StatBoundFail.png");
}

void ErrorGroupModelTest::testImage()
{
    ErrorGroupModel model{this};

    QFETCH(int, row);
    const auto index = model.index(row, 0);
    QVERIFY(index.isValid());
    QTEST(index.data(Qt::UserRole + 1).toString(), "image");
}

void ErrorGroupModelTest::testType_data()
{
    QTest::addColumn<int>("row");
    QTest::addColumn<ErrorGroupModel::ErrorGroup>("type");

    QTest::newRow("0")  <<  0 << ErrorGroupModel::ErrorGroup::LengthOutsideBoundary;
    QTest::newRow("1")  <<  1 << ErrorGroupModel::ErrorGroup::LengthInsideBoundary;
    QTest::newRow("2")  <<  2 << ErrorGroupModel::ErrorGroup::AreaOutsideBoundary;
    QTest::newRow("3")  <<  3 << ErrorGroupModel::ErrorGroup::PeakOutsideBoundary;
}

void ErrorGroupModelTest::testType()
{
    ErrorGroupModel model{this};

    QFETCH(int, row);
    const auto index = model.index(row, 0);
    QVERIFY(index.isValid());
    QTEST(index.data(Qt::UserRole + 2).value<ErrorGroupModel::ErrorGroup>(), "type");
}

QTEST_GUILESS_MAIN(ErrorGroupModelTest)
#include "errorGroupModelTest.moc"
