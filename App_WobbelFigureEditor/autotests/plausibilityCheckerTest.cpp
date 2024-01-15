#include <QTest>
#include <QSignalSpy>

#include "../src/plausibilityChecker.h"
#include "../src/FigureAnalyzer.h"

using precitec::scanmaster::components::wobbleFigureEditor::PlausibilityChecker;
using precitec::scantracker::components::wobbleFigureEditor::FigureAnalyzer;

class PlausibilityCheckerTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void initTestCase();
    void testCtor();
    void testRowCount();
    void testData_data();
    void testData();
    void testHeight();
    void testWidth();
    void testCurrentRow_data();
    void testCurrentRow();

private:
    QTemporaryDir m_dir;
};

void PlausibilityCheckerTest::initTestCase()
{
    QVERIFY(m_dir.isValid());
    QVERIFY(QDir{}.mkpath(m_dir.filePath(QStringLiteral("config"))));
    qputenv("WM_BASE_DIR", m_dir.path().toUtf8());
}

void PlausibilityCheckerTest::testCtor()
{
    PlausibilityChecker checker;

    QCOMPARE(checker.height(), 0.0);
    QCOMPARE(checker.width(), 0.0);
    QCOMPARE(checker.currentRow(), -1);
    QCOMPARE(checker.m_plausibilityInformation.at(0).amplitude, 1);
    QCOMPARE(checker.m_plausibilityInformation.at(0).frequency, 780);
    QCOMPARE(checker.m_plausibilityInformation.at(0).conformity, 0.7);
    QCOMPARE(checker.m_plausibilityInformation.at(1).amplitude, 2);
    QCOMPARE(checker.m_plausibilityInformation.at(1).frequency, 780);
    QCOMPARE(checker.m_plausibilityInformation.at(1).conformity, 0.65);
    QCOMPARE(checker.m_plausibilityInformation.at(2).amplitude, 6);
    QCOMPARE(checker.m_plausibilityInformation.at(2).frequency, 312);
    QCOMPARE(checker.m_plausibilityInformation.at(2).conformity, 0.9);
}

void PlausibilityCheckerTest::testRowCount()
{
    PlausibilityChecker checker;

    QCOMPARE(checker.rowCount(), 3);
}

void PlausibilityCheckerTest::testData_data()
{
    QTest::addColumn<QVector<int>>("amplitude");
    QTest::addColumn<QVector<int>>("frequency");
    QTest::addColumn<QVector<double>>("conformity");

    QTest::newRow("Information") << QVector<int> {
        1,
        2,
        6
    } << QVector<int> {
        780,
        780,
        312
    } << QVector<double> {
        0.70,
        0.65,
        0.90
    };
}

void PlausibilityCheckerTest::testData()
{
    PlausibilityChecker checker;

    QFETCH(QVector<int>, amplitude);
    QFETCH(QVector<int>, frequency);
    QFETCH(QVector<double>, conformity);

    for (int i = 0; i < checker.rowCount(); i++)
    {
        QCOMPARE(checker.data(checker.index(i)), amplitude.at(i));
        QCOMPARE(checker.data(checker.index(i), Qt::UserRole), frequency.at(i));
        QCOMPARE(checker.data(checker.index(i), Qt::UserRole + 1), conformity.at(i));
    }
}

void PlausibilityCheckerTest::testHeight()
{
    PlausibilityChecker checker;

    QSignalSpy heightChanged{&checker, &PlausibilityChecker::heightChanged};
    QVERIFY(heightChanged.isValid());
    QCOMPARE(heightChanged.count(), 0);

    checker.setHeight(0.0);
    QCOMPARE(heightChanged.count(), 0);

    checker.setHeight(2.5);
    QCOMPARE(checker.height(), 2.5);
    QCOMPARE(heightChanged.count(), 1);

    checker.setHeight(20.5);
    QCOMPARE(checker.height(), 20.5);
    QCOMPARE(heightChanged.count(), 2);
}

void PlausibilityCheckerTest::testWidth()
{
    PlausibilityChecker checker;

    QSignalSpy widthChanged{&checker, &PlausibilityChecker::widthChanged};
    QVERIFY(widthChanged.isValid());
    QCOMPARE(widthChanged.count(), 0);

    checker.setWidth(0.0);
    QCOMPARE(widthChanged.count(), 0);

    checker.setWidth(0.5);
    QCOMPARE(checker.width(), 0.5);
    QCOMPARE(widthChanged.count(), 1);

    checker.setWidth(23.5);
    QCOMPARE(checker.width(), 23.5);
    QCOMPARE(widthChanged.count(), 2);
}

void PlausibilityCheckerTest::testCurrentRow_data()
{
    QTest::addColumn<QVector<double>>("width");
    QTest::addColumn<QVector<double>>("height");
    QTest::addColumn<QVector<int>>("currentRow");

    QTest::newRow("Information") << QVector<double> {
        0.000000001,
        0.5,
        0.000000001,
        0.99,
        0.000000001,
        1.000000001,
        0.000000001,
        2.0,
        0.000000001,
        4.75,
        0.000000001,
        6.0,
        0.000000001,
        25.0
    } << QVector<double> {
        0.000000001,
        0.000000001,
        0.75,
        0.000000001,
        1.0,
        0.000000001,
        1.5,
        0.000000001,
        2.000000001,
        0.000000001,
        5.99,
        0.000000001,
        15.0,
        25.0
    } << QVector<int> {
        -1,
        0,
        0,
        0,
        0,
        1,
        1,
        1,
        2,
        2,
        2,
        2,
        2,
        2
    };
}

void PlausibilityCheckerTest::testCurrentRow()
{
    PlausibilityChecker checker;

    QFETCH(QVector<double>, width);
    QFETCH(QVector<double>, height);
    QFETCH(QVector<int>, currentRow);

    QCOMPARE(width.size(), height.size());
    QCOMPARE(width.size(), currentRow.size());

    for (int i = 0; i < width.size(); i++)
    {
        checker.setWidth(width.at(i));
        checker.setHeight(height.at(i));
        QCOMPARE(checker.currentRow(), currentRow.at(i));
    }
}

QTEST_GUILESS_MAIN(PlausibilityCheckerTest)
#include "plausibilityCheckerTest.moc"

