#include <QTest>
#include <QSignalSpy>

#include "../src/intervalErrorSimpleConfigModel.h"
#include "../src/intervalError.h"

using precitec::gui::IntervalErrorSimpleConfigModel;
using precitec::storage::IntervalError;

class IntervalErrorSimpleConfigModelTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testCtor();
    void testRoleNames();
    void testIntervalError();
    void testData_data();
    void testData();
    void testSetData();
};

void IntervalErrorSimpleConfigModelTest::testCtor()
{
    IntervalErrorSimpleConfigModel model{this};

    QCOMPARE(model.rowCount(), 3);
    QVERIFY(!model.intervalError());
}

void IntervalErrorSimpleConfigModelTest::testRoleNames()
{
    IntervalErrorSimpleConfigModel model{this};
    const auto roleNames = model.roleNames();
    QCOMPARE(roleNames.size(), 9);
    QCOMPARE(roleNames[Qt::DisplayRole], QByteArrayLiteral("color"));
    QCOMPARE(roleNames[Qt::UserRole], QByteArrayLiteral("min"));
    QCOMPARE(roleNames[Qt::UserRole + 1], QByteArrayLiteral("max"));
    QCOMPARE(roleNames[Qt::UserRole + 2], QByteArrayLiteral("threshold"));
    QCOMPARE(roleNames[Qt::UserRole + 3], QByteArrayLiteral("qnMin"));
    QCOMPARE(roleNames[Qt::UserRole + 4], QByteArrayLiteral("qnMax"));
    QCOMPARE(roleNames[Qt::UserRole + 5], QByteArrayLiteral("qnThreshold"));
    QCOMPARE(roleNames[Qt::UserRole + 6], QByteArrayLiteral("secondThreshold"));
    QCOMPARE(roleNames[Qt::UserRole + 7], QByteArrayLiteral("qnSecondThreshold"));
}

void IntervalErrorSimpleConfigModelTest::testIntervalError()
{
    IntervalErrorSimpleConfigModel model{this};
    QVERIFY(!model.intervalError());

    QSignalSpy intervalErrorChangedSpy(&model, &IntervalErrorSimpleConfigModel::intervalErrorChanged);
    QVERIFY(intervalErrorChangedSpy.isValid());

    QSignalSpy modelResetSpy(&model, &IntervalErrorSimpleConfigModel::modelReset);
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

void IntervalErrorSimpleConfigModelTest::testData_data()
{
    QTest::addColumn<int>("row");
    QTest::addColumn<QColor>("color");
    QTest::addColumn<qreal>("min");
    QTest::addColumn<qreal>("max");
    QTest::addColumn<qreal>("threshold");

    QTest::newRow("0") << 0 << QColor("#ff75D480") << -3.7 << 12.3 << 7.4;
    QTest::newRow("1") << 1 << QColor("#ffE6D453") << 2.5 << 13.8 << 6.4;
    QTest::newRow("2") << 2 << QColor("#ffE68E73") << -12.9 << -2.0 << 17.4;
}

void IntervalErrorSimpleConfigModelTest::testData()
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

    IntervalErrorSimpleConfigModel model{this};

    QFETCH(int, row);
    const auto index = model.index(row, 0);
    QVERIFY(index.isValid());
    QTEST(index.data().value<QColor>(), "color");

    QCOMPARE(index.data(Qt::UserRole), QVariant{});
    QCOMPARE(index.data(Qt::UserRole + 1), QVariant{});
    QCOMPARE(index.data(Qt::UserRole + 2), QVariant{});

    model.setIntervalError(error);

    QTEST(index.data(Qt::UserRole).toReal(), "min");
    QTEST(index.data(Qt::UserRole + 1).toReal(), "max");
    QTEST(index.data(Qt::UserRole + 2).toReal(), "threshold");
}

void IntervalErrorSimpleConfigModelTest::testSetData()
{
    IntervalErrorSimpleConfigModel model{this};

    QSignalSpy dataChangedSpy(&model, &IntervalErrorSimpleConfigModel::dataChanged);
    QVERIFY(dataChangedSpy.isValid());

    QVERIFY(!model.setData(model.index(0), QVariant{5}, Qt::UserRole));

    auto error = new IntervalError{this};

    model.setIntervalError(error);
    QCOMPARE(dataChangedSpy.count(), 0);

    for (auto i = 0; i < 3; i++)
    {
        const auto index = model.index(i);

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
            QCOMPARE(topLeft.row(), i);
            QCOMPARE(topLeft.column(), 0);
            auto bottomRight = args.at(1).toModelIndex();
            QVERIFY(bottomRight.isValid());
            QCOMPARE(bottomRight.row(), i);
            QCOMPARE(bottomRight.column(), 0);
            auto roles = args.at(2).value<QVector<int>>();
            QCOMPARE(roles.size(), 1);
            QVERIFY(roles.contains(Qt::UserRole + j));
        }
    }
}

QTEST_GUILESS_MAIN(IntervalErrorSimpleConfigModelTest)
#include "intervalErrorSimpleConfigModelTest.moc"

