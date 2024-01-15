#include <QTest>
#include <QSignalSpy>

#include "../src/intervalErrorConfigModel.h"
#include "../src/intervalError.h"
#include "precitec/dataSet.h"
#include "precitec/infiniteSet.h"

using precitec::gui::IntervalErrorConfigModel;
using precitec::gui::components::plotter::DataSet;
using precitec::gui::components::plotter::InfiniteSet;
using precitec::storage::IntervalError;

class IntervalErrorConfigModelTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testCtor();
    void testRoleNames();
    void testIntervalError();
    void testVisualReference();
    void testData_data();
    void testData();
    void testSetData();
    void testMinMaxFromReference();
};

void IntervalErrorConfigModelTest::testCtor()
{
    IntervalErrorConfigModel model{this};

    QCOMPARE(model.rowCount(), 3);
    QVERIFY(!model.intervalError());
    QVERIFY(model.visualReference());
    QVERIFY(model.visualReference()->isEmpty());
    QCOMPARE(model.visualReference()->name(), QStringLiteral(""));
    QCOMPARE(model.visualReference()->drawingOrder(), DataSet::DrawingOrder::OnTop);
    QCOMPARE(model.m_lowerBoundaries.size(), 3);
    QCOMPARE(model.m_upperBoundaries.size(), 3);
    QCOMPARE(model.m_shiftedLowerBoundaries.size(), 3);
    QCOMPARE(model.m_shiftedUpperBoundaries.size(), 3);

    QCOMPARE(model.m_lowerBoundaries.at(0)->name(), QStringLiteral("Level 1 Min"));
    QCOMPARE(model.m_lowerBoundaries.at(1)->name(), QStringLiteral("Level 2 Min"));
    QCOMPARE(model.m_lowerBoundaries.at(2)->name(), QStringLiteral("Level 3 Min"));
    QCOMPARE(model.m_lowerBoundaries.at(0)->color(), QColor("#75D480"));
    QCOMPARE(model.m_lowerBoundaries.at(1)->color(), QColor("#E6D453"));
    QCOMPARE(model.m_lowerBoundaries.at(2)->color(), QColor("#E68E73"));

    QCOMPARE(model.m_upperBoundaries.at(0)->name(), QStringLiteral("Level 1 Max"));
    QCOMPARE(model.m_upperBoundaries.at(1)->name(), QStringLiteral("Level 2 Max"));
    QCOMPARE(model.m_upperBoundaries.at(2)->name(), QStringLiteral("Level 3 Max"));
    QCOMPARE(model.m_upperBoundaries.at(0)->color(), QColor("#75D480"));
    QCOMPARE(model.m_upperBoundaries.at(1)->color(), QColor("#E6D453"));
    QCOMPARE(model.m_upperBoundaries.at(2)->color(), QColor("#E68E73"));

    QCOMPARE(model.m_shiftedLowerBoundaries.at(0)->name(), QStringLiteral("Level 1 Min Shift"));
    QCOMPARE(model.m_shiftedLowerBoundaries.at(1)->name(), QStringLiteral("Level 2 Min Shift"));
    QCOMPARE(model.m_shiftedLowerBoundaries.at(2)->name(), QStringLiteral("Level 3 Min Shift"));
    QCOMPARE(model.m_shiftedLowerBoundaries.at(0)->color(), QColor("#75D480"));
    QCOMPARE(model.m_shiftedLowerBoundaries.at(1)->color(), QColor("#E6D453"));
    QCOMPARE(model.m_shiftedLowerBoundaries.at(2)->color(), QColor("#E68E73"));

    QCOMPARE(model.m_shiftedUpperBoundaries.at(0)->name(), QStringLiteral("Level 1 Max Shift"));
    QCOMPARE(model.m_shiftedUpperBoundaries.at(1)->name(), QStringLiteral("Level 2 Max Shift"));
    QCOMPARE(model.m_shiftedUpperBoundaries.at(2)->name(), QStringLiteral("Level 3 Max Shift"));
    QCOMPARE(model.m_shiftedUpperBoundaries.at(0)->color(), QColor("#75D480"));
    QCOMPARE(model.m_shiftedUpperBoundaries.at(1)->color(), QColor("#E6D453"));
    QCOMPARE(model.m_shiftedUpperBoundaries.at(2)->color(), QColor("#E68E73"));
}

void IntervalErrorConfigModelTest::testRoleNames()
{
    IntervalErrorConfigModel model{this};
    const auto roleNames = model.roleNames();
    QCOMPARE(roleNames.size(), 13);
    QCOMPARE(roleNames[Qt::DisplayRole], QByteArrayLiteral("color"));
    QCOMPARE(roleNames[Qt::UserRole], QByteArrayLiteral("min"));
    QCOMPARE(roleNames[Qt::UserRole + 1], QByteArrayLiteral("max"));
    QCOMPARE(roleNames[Qt::UserRole + 2], QByteArrayLiteral("threshold"));
    QCOMPARE(roleNames[Qt::UserRole + 3], QByteArrayLiteral("lower"));
    QCOMPARE(roleNames[Qt::UserRole + 4], QByteArrayLiteral("upper"));
    QCOMPARE(roleNames[Qt::UserRole + 5], QByteArrayLiteral("shiftedLower"));
    QCOMPARE(roleNames[Qt::UserRole + 6], QByteArrayLiteral("shiftedUpper"));
    QCOMPARE(roleNames[Qt::UserRole + 7], QByteArrayLiteral("qnMin"));
    QCOMPARE(roleNames[Qt::UserRole + 8], QByteArrayLiteral("qnMax"));
    QCOMPARE(roleNames[Qt::UserRole + 9], QByteArrayLiteral("qnThreshold"));
    QCOMPARE(roleNames[Qt::UserRole + 10], QByteArrayLiteral("secondThreshold"));
    QCOMPARE(roleNames[Qt::UserRole + 11], QByteArrayLiteral("qnSecondThreshold"));
    
}

void IntervalErrorConfigModelTest::testIntervalError()
{
    IntervalErrorConfigModel model{this};
    QVERIFY(!model.intervalError());

    QSignalSpy intervalErrorChangedSpy(&model, &IntervalErrorConfigModel::intervalErrorChanged);
    QVERIFY(intervalErrorChangedSpy.isValid());

    QSignalSpy modelResetSpy(&model, &IntervalErrorConfigModel::modelReset);
    QVERIFY(modelResetSpy.isValid());

    QCOMPARE(intervalErrorChangedSpy.count(), 0);
    QCOMPARE(modelResetSpy.count(), 0);

    auto intervalError = new IntervalError{this};
    QSignalSpy destroyedSpy(intervalError, &IntervalError::destroyed);
    model.setIntervalError(intervalError);
    QCOMPARE(model.intervalError(), intervalError);
    QCOMPARE(intervalErrorChangedSpy.count(), 1);
    QCOMPARE(modelResetSpy.count(), 1);

    model.setIntervalError(intervalError);
    QCOMPARE(intervalErrorChangedSpy.count(), 1);
    QCOMPARE(modelResetSpy.count(), 1);

    intervalError->deleteLater();
    QVERIFY(destroyedSpy.wait());
    QVERIFY(!model.intervalError());
    QCOMPARE(intervalErrorChangedSpy.count(), 2);
}

void IntervalErrorConfigModelTest::testVisualReference()
{
    IntervalErrorConfigModel model{this};
    QVERIFY(model.visualReference());
    QVERIFY(model.visualReference()->isEmpty());
    QCOMPARE(model.visualReference()->name(), QStringLiteral(""));
    QCOMPARE(model.visualReference()->drawingOrder(), DataSet::DrawingOrder::OnTop);

    QSignalSpy visualReferenceChangedSpy(&model, &IntervalErrorConfigModel::visualReferenceChanged);
    QVERIFY(visualReferenceChangedSpy.isValid());

    auto dataSet = new DataSet{this};
    dataSet->setName(QStringLiteral("Some Result"));
    dataSet->setColor("blue");
    dataSet->addSample(QVector2D{1.3, 3.6});
    dataSet->addSample(QVector2D{2.4, 7.5});
    dataSet->addSample(QVector2D{3.6,-2.3});

    model.setVisualReference(dataSet);
    QCOMPARE(visualReferenceChangedSpy.count(), 1);
    QCOMPARE(model.visualReference()->sampleCount(), 3);
    QCOMPARE(model.visualReference()->name(), QStringLiteral("Some Result"));
    QCOMPARE(model.visualReference()->color(), QColor{"blue"});

    model.setVisualReference(nullptr);
    QCOMPARE(visualReferenceChangedSpy.count(), 2);
    QVERIFY(model.visualReference()->isEmpty());
    QCOMPARE(model.visualReference()->name(), QStringLiteral(""));
    QCOMPARE(model.visualReference()->color(), QColor{"white"});

    auto intervalError = new IntervalError{this};
    model.setIntervalError(intervalError);
    model.setVisualReference(dataSet);
    QCOMPARE(visualReferenceChangedSpy.count(), 3);
    QCOMPARE(model.visualReference()->sampleCount(), 3);
    QCOMPARE(model.visualReference()->name(), QStringLiteral("Some Result"));
    QCOMPARE(model.visualReference()->color(), QColor{"blue"});

    intervalError->setResultValue(15);
    QCOMPARE(visualReferenceChangedSpy.count(), 4);
    QVERIFY(model.visualReference()->isEmpty());
    QCOMPARE(model.visualReference()->name(), QStringLiteral(""));
    QCOMPARE(model.visualReference()->color(), QColor{"white"});
}

void IntervalErrorConfigModelTest::testData_data()
{
    QTest::addColumn<int>("row");
    QTest::addColumn<QColor>("color");
    QTest::addColumn<QColor>("opacity_color");
    QTest::addColumn<float>("min");
    QTest::addColumn<float>("max");
    QTest::addColumn<float>("threshold");
    QTest::addColumn<float>("shift_min");
    QTest::addColumn<float>("shift_max");

    QTest::newRow("0") << 0 << QColor("#ff75D480") << QColor("#9675D480") << -3.7f << 12.3f << 7.4f << -3.7f + 2.6f << 12.3f + 2.6f;
    QTest::newRow("1") << 1 << QColor("#ffE6D453") << QColor("#96E6D453") << 2.5f << 13.8f << 6.4f << 2.5f + 2.6f << 13.8f + 2.6f;
    QTest::newRow("2") << 2 << QColor("#ffE68E73") << QColor("#96E68E73") << -12.9f << -2.0f << 17.4f << -12.9f + 2.6f << -2.0f + 2.6f;
}

void IntervalErrorConfigModelTest::testData()
{
    auto error = new IntervalError{this};
    error->setShift(2.6);
    error->setMin(0, -3.7);
    error->setMax(0, 12.3);
    error->setThreshold(0, 7.4);
    error->setMin(1, 2.5);
    error->setMax(1, 13.8);
    error->setThreshold(1, 6.4);
    error->setMin(2, -12.9);
    error->setMax(2, -2.0);
    error->setThreshold(2, 17.4);

    IntervalErrorConfigModel model{this};

    QFETCH(int, row);
    const auto index = model.index(row, 0);
    QVERIFY(index.isValid());
    QTEST(index.data().value<QColor>(), "color");

    QCOMPARE(index.data(Qt::UserRole), QVariant{});
    QCOMPARE(index.data(Qt::UserRole + 1), QVariant{});
    QCOMPARE(index.data(Qt::UserRole + 2), QVariant{});
    QCOMPARE(index.data(Qt::UserRole + 3), QVariant{});
    QCOMPARE(index.data(Qt::UserRole + 4), QVariant{});
    QCOMPARE(index.data(Qt::UserRole + 5), QVariant{});
    QCOMPARE(index.data(Qt::UserRole + 6), QVariant{});

    model.setIntervalError(error);

    QTEST(index.data(Qt::UserRole).toFloat(), "min");
    QTEST(index.data(Qt::UserRole + 1).toFloat(), "max");
    QTEST(index.data(Qt::UserRole + 2).toFloat(), "threshold");
    QVERIFY(index.data(Qt::UserRole + 3).value<InfiniteSet*>());
    QTEST(index.data(Qt::UserRole + 3).value<InfiniteSet*>()->value(), "min");
    QCOMPARE(index.data(Qt::UserRole + 3).value<InfiniteSet*>()->isEnabled(), true);
    QTEST(index.data(Qt::UserRole + 3).value<InfiniteSet*>()->color(), "opacity_color");
    QVERIFY(index.data(Qt::UserRole + 4).value<InfiniteSet*>());
    QTEST(index.data(Qt::UserRole + 4).value<InfiniteSet*>()->value(), "max");
    QCOMPARE(index.data(Qt::UserRole + 4).value<InfiniteSet*>()->isEnabled(), true);
    QTEST(index.data(Qt::UserRole + 4).value<InfiniteSet*>()->color(), "opacity_color");
    QVERIFY(index.data(Qt::UserRole + 5).value<InfiniteSet*>());
    QTEST(index.data(Qt::UserRole + 5).value<InfiniteSet*>()->value(), "shift_min");
    QCOMPARE(index.data(Qt::UserRole + 5).value<InfiniteSet*>()->isEnabled(), true);
    QTEST(index.data(Qt::UserRole + 5).value<InfiniteSet*>()->color(), "color");
    QVERIFY(index.data(Qt::UserRole + 6).value<InfiniteSet*>());
    QTEST(index.data(Qt::UserRole + 6).value<InfiniteSet*>()->value(), "shift_max");
    QCOMPARE(index.data(Qt::UserRole + 6).value<InfiniteSet*>()->isEnabled(), true);
    QTEST(index.data(Qt::UserRole + 6).value<InfiniteSet*>()->color(), "color");

    error->setShift(0);
    QTEST(index.data(Qt::UserRole + 3).value<InfiniteSet*>()->value(), "min");
    QCOMPARE(index.data(Qt::UserRole + 3).value<InfiniteSet*>()->isEnabled(), true);
    QTEST(index.data(Qt::UserRole + 3).value<InfiniteSet*>()->color(), "color");
    QVERIFY(index.data(Qt::UserRole + 4).value<InfiniteSet*>());
    QTEST(index.data(Qt::UserRole + 4).value<InfiniteSet*>()->value(), "max");
    QCOMPARE(index.data(Qt::UserRole + 4).value<InfiniteSet*>()->isEnabled(), true);
    QTEST(index.data(Qt::UserRole + 4).value<InfiniteSet*>()->color(), "color");
    QVERIFY(index.data(Qt::UserRole + 5).value<InfiniteSet*>());
    QTEST(index.data(Qt::UserRole + 5).value<InfiniteSet*>()->value(), "min");
    QCOMPARE(index.data(Qt::UserRole + 5).value<InfiniteSet*>()->isEnabled(), false);
    QTEST(index.data(Qt::UserRole + 5).value<InfiniteSet*>()->color(), "color");
    QVERIFY(index.data(Qt::UserRole + 6).value<InfiniteSet*>());
    QTEST(index.data(Qt::UserRole + 6).value<InfiniteSet*>()->value(), "max");
    QCOMPARE(index.data(Qt::UserRole + 6).value<InfiniteSet*>()->isEnabled(), false);
    QTEST(index.data(Qt::UserRole + 6).value<InfiniteSet*>()->color(), "color");
}

void IntervalErrorConfigModelTest::testSetData()
{
    IntervalErrorConfigModel model{this};

    QSignalSpy dataChangedSpy(&model, &IntervalErrorConfigModel::dataChanged);
    QVERIFY(dataChangedSpy.isValid());

    QVERIFY(!model.setData(model.index(0), QVariant{5}, Qt::UserRole));

    auto error = new IntervalError{this};

    model.setIntervalError(error);
    QCOMPARE(dataChangedSpy.count(), 0);

    for (auto i = 0; i < 3; i++)
    {
        const auto index = model.index(i, 0);

        for (auto j = 0; j < 3; j++)
        {
            QVERIFY(index.isValid());
            QVERIFY(model.setData(index, 5.3, Qt::UserRole + j));
            QCOMPARE(index.data(Qt::UserRole + j), 5.3);
            QCOMPARE(dataChangedSpy.count(), 1);

            auto args = dataChangedSpy.takeFirst();
            QCOMPARE(dataChangedSpy.count(), 0);
            QCOMPARE(args.size(), 3);
            auto topLeft = args.at(0).toModelIndex();
            QVERIFY(topLeft.isValid());
            QCOMPARE(topLeft.row(), 0);
            QCOMPARE(topLeft.column(), 0);
            auto bottomRight = args.at(1).toModelIndex();
            QVERIFY(bottomRight.isValid());
            QCOMPARE(bottomRight.row(), 2);
            QCOMPARE(bottomRight.column(), 0);
            auto roles = args.at(2).value<QVector<int>>();
            QCOMPARE(roles.size(), 1);
            QVERIFY(roles.contains(Qt::UserRole + j));
        }
    }
}

void IntervalErrorConfigModelTest::testMinMaxFromReference()
{
    IntervalErrorConfigModel model{this};
    QSignalSpy dataChangedSpy(&model, &IntervalErrorConfigModel::dataChanged);
    QVERIFY(dataChangedSpy.isValid());

    QSignalSpy visualReferenceChangedSpy(&model, &IntervalErrorConfigModel::visualReferenceChanged);
    QVERIFY(visualReferenceChangedSpy.isValid());

    model.setMinFromReference(0);
    model.setMaxFromReference(2);

    QCOMPARE(dataChangedSpy.count(), 0);

    auto error = new IntervalError{this};
    model.setIntervalError(error);

    model.setMinFromReference(0);
    model.setMaxFromReference(2);

    QCOMPARE(dataChangedSpy.count(), 0);

    auto dataSet = new DataSet{this};
    dataSet->addSample(QVector2D{1.3, 3.6});
    dataSet->addSample(QVector2D{2.4, 7.5});
    dataSet->addSample(QVector2D{3.6,-2.3});

    model.setVisualReference(dataSet);

    model.setMinFromReference(0);

    QCOMPARE(dataChangedSpy.count(), 1);

    QCOMPARE(model.index(0, 0).data(Qt::UserRole), -2.3f);

    auto args = dataChangedSpy.takeFirst();
    QCOMPARE(dataChangedSpy.count(), 0);
    QCOMPARE(args.size(), 3);
    auto topLeft = args.at(0).toModelIndex();
    QVERIFY(topLeft.isValid());
    QCOMPARE(topLeft.row(), 0);
    QCOMPARE(topLeft.column(), 0);
    auto bottomRight = args.at(1).toModelIndex();
    QVERIFY(bottomRight.isValid());
    QCOMPARE(bottomRight.row(), 2);
    QCOMPARE(bottomRight.column(), 0);
    auto roles = args.at(2).value<QVector<int>>();
    QCOMPARE(roles.size(), 1);
    QVERIFY(roles.contains(Qt::UserRole));

    model.setMaxFromReference(2);

    QCOMPARE(dataChangedSpy.count(), 1);

    QCOMPARE(model.index(2, 0).data(Qt::UserRole + 1), 7.5);

    auto args2 = dataChangedSpy.takeFirst();
    QCOMPARE(dataChangedSpy.count(), 0);
    QCOMPARE(args2.size(), 3);
    auto topLeft2 = args2.at(0).toModelIndex();
    QVERIFY(topLeft2.isValid());
    QCOMPARE(topLeft2.row(), 0);
    QCOMPARE(topLeft2.column(), 0);
    auto bottomRight2 = args2.at(1).toModelIndex();
    QVERIFY(bottomRight2.isValid());
    QCOMPARE(bottomRight2.row(), 2);
    QCOMPARE(bottomRight2.column(), 0);
    auto roles2 = args2.at(2).value<QVector<int>>();
    QCOMPARE(roles2.size(), 1);
    QVERIFY(roles2.contains(Qt::UserRole + 1));
}

QTEST_GUILESS_MAIN(IntervalErrorConfigModelTest)
#include "intervalErrorConfigModelTest.moc"
