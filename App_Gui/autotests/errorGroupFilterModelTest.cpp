#include <QTest>
#include <QSignalSpy>

#include "../src/errorGroupFilterModel.h"
#include "../src/errorGroupModel.h"
#include "../src/simpleErrorModel.h"

using precitec::gui::ErrorGroupFilterModel;
using precitec::gui::ErrorGroupModel;
using precitec::gui::SimpleErrorModel;

class ErrorGroupFilterModelTest : public QObject
{
 Q_OBJECT
private Q_SLOTS:
    void testCtor();
    void testFilterGroup();
    void testInterval();
    void testFilter_data();
    void testFilter();
};

void ErrorGroupFilterModelTest::testCtor()
{
    ErrorGroupFilterModel model{this};
    QCOMPARE(model.filterGroup(), -1);
    QCOMPARE(model.interval(), false);
}

void ErrorGroupFilterModelTest::testFilterGroup()
{
    ErrorGroupFilterModel model{this};
    QSignalSpy filterGroupChangedSpy{&model, &ErrorGroupFilterModel::filterGroupChanged};
    QVERIFY(filterGroupChangedSpy.isValid());

    QCOMPARE(model.filterGroup(), -1);

    model.setFilterGroup(-1);
    QCOMPARE(filterGroupChangedSpy.count(), 0);

    model.setFilterGroup(3);
    QCOMPARE(model.filterGroup(), 3);
    QCOMPARE(filterGroupChangedSpy.count(), 1);

    model.setFilterGroup(3);
    QCOMPARE(filterGroupChangedSpy.count(), 1);
}

void ErrorGroupFilterModelTest::testInterval()
{
    ErrorGroupFilterModel model{this};
    QSignalSpy intervalChangedSpy{&model, &ErrorGroupFilterModel::intervalChanged};
    QVERIFY(intervalChangedSpy.isValid());

    QCOMPARE(model.interval(), false);

    model.setInterval(false);
    QCOMPARE(intervalChangedSpy.count(), 0);

    model.setInterval(true);
    QCOMPARE(model.interval(), true);
    QCOMPARE(intervalChangedSpy.count(), 1);

    model.setInterval(true);
    QCOMPARE(intervalChangedSpy.count(), 1);
}

void ErrorGroupFilterModelTest::testFilter_data()
{
    QTest::addColumn<ErrorGroupModel::ErrorGroup>("filterGroup");
    QTest::addColumn<bool>("interval");
    QTest::addColumn<int>("count");
    QTest::addColumn<QVector<SimpleErrorModel::ErrorType>>("components");

    QTest::newRow("Length Outside Boundary, no interval") << ErrorGroupModel::ErrorGroup::LengthOutsideBoundary << false << 6 << QVector<SimpleErrorModel::ErrorType>{
        SimpleErrorModel::ErrorType::LengthOutsideStaticBoundary,
        SimpleErrorModel::ErrorType::LengthOutsideReferenceBoundary,
        SimpleErrorModel::ErrorType::AccumulatedLengthOutsideStaticBoundary,
        SimpleErrorModel::ErrorType::AccumulatedLengthOutsideReferenceBoundary,
        SimpleErrorModel::ErrorType::DualOutlierStaticBoundary,
        SimpleErrorModel::ErrorType::DualOutlierReferenceBoundary
    };
    QTest::newRow("Length Inside Boundary, no interval") << ErrorGroupModel::ErrorGroup::LengthInsideBoundary << false << 2 << QVector<SimpleErrorModel::ErrorType>{
        SimpleErrorModel::ErrorType::LengthInsideStaticBoundary,
        SimpleErrorModel::ErrorType::LengthInsideReferenceBoundary
    };
    QTest::newRow("Area Outside Boundary, no interval") << ErrorGroupModel::ErrorGroup::AreaOutsideBoundary << false << 4 << QVector<SimpleErrorModel::ErrorType>{
        SimpleErrorModel::ErrorType::AreaStaticBoundary,
        SimpleErrorModel::ErrorType::AreaReferenceBoundary,
        SimpleErrorModel::ErrorType::AccumulatedAreaStaticBoundary,
        SimpleErrorModel::ErrorType::AccumulatedAreaReferenceBoundary
    };
    QTest::newRow("Peak Outside Boundary, no interval") << ErrorGroupModel::ErrorGroup::PeakOutsideBoundary << false << 2 << QVector<SimpleErrorModel::ErrorType>{
        SimpleErrorModel::ErrorType::PeakStaticBoundary,
        SimpleErrorModel::ErrorType::PeakReferenceBoundary
    };
    QTest::newRow("Length Outside Boundary, interval") << ErrorGroupModel::ErrorGroup::LengthOutsideBoundary << true << 3 << QVector<SimpleErrorModel::ErrorType>{
        SimpleErrorModel::ErrorType::LengthOutsideStaticBoundary,
        SimpleErrorModel::ErrorType::AccumulatedLengthOutsideStaticBoundary,
        SimpleErrorModel::ErrorType::DualOutlierStaticBoundary
    };
    QTest::newRow("Length Inside Boundary, interval") << ErrorGroupModel::ErrorGroup::LengthInsideBoundary << true << 1 << QVector<SimpleErrorModel::ErrorType>{
        SimpleErrorModel::ErrorType::LengthInsideStaticBoundary
    };
    QTest::newRow("Area Outside Boundary, interval") << ErrorGroupModel::ErrorGroup::AreaOutsideBoundary << true << 2 << QVector<SimpleErrorModel::ErrorType>{
        SimpleErrorModel::ErrorType::AreaStaticBoundary,
        SimpleErrorModel::ErrorType::AccumulatedAreaStaticBoundary
    };
    QTest::newRow("Peak Outside Boundary, interval") << ErrorGroupModel::ErrorGroup::PeakOutsideBoundary << true << 1 << QVector<SimpleErrorModel::ErrorType>{
        SimpleErrorModel::ErrorType::PeakStaticBoundary
    };
}

void ErrorGroupFilterModelTest::testFilter()
{
    ErrorGroupFilterModel filterModel{this};
    QCOMPARE(filterModel.rowCount(), 0);

    SimpleErrorModel control{this};
    filterModel.setSourceModel(&control);
    QCOMPARE(filterModel.rowCount(), 0);


    QFETCH(bool, interval);
    QFETCH(ErrorGroupModel::ErrorGroup, filterGroup);

    filterModel.setInterval(interval);
    filterModel.setFilterGroup(int(filterGroup));

    QTEST(filterModel.rowCount(), "count");

    QFETCH(QVector<SimpleErrorModel::ErrorType>, components);
    QCOMPARE(components.count(), filterModel.rowCount());
    for (int i = 0; i < filterModel.rowCount(); i++)
    {
        QCOMPARE(filterModel.index(i, 0).data(Qt::UserRole + 3).value<SimpleErrorModel::ErrorType>(), components.at(i));
    }
}

QTEST_GUILESS_MAIN(ErrorGroupFilterModelTest)
#include "errorGroupFilterModelTest.moc"

