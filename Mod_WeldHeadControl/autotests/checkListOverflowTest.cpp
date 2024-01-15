#include <QTest>
#include <QTemporaryDir>

#include "../include/viWeldHead/Scanlab/Scanlab.h"

class CheckListOverflowTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void initTestCase();
    void testCtor();
    void testNumberOfPointsFromContour();
    void testNumberOfPossiblePointsForListMemory();
    void testIfCheckWorks();

private:
    QTemporaryDir m_dir;
};

void CheckListOverflowTest::initTestCase()
{
    QVERIFY(m_dir.isValid());
    QVERIFY(QDir{}.mkpath(m_dir.filePath(QStringLiteral("config"))));
    qputenv("WM_BASE_DIR", m_dir.path().toUtf8());
}

void CheckListOverflowTest::testCtor()
{
    auto scanlab = new precitec::hardware::Scanlab();

    QCOMPARE(scanlab->m_oCalibValueBitsPerMM, 4000.0);

    delete scanlab;
}

void CheckListOverflowTest::testNumberOfPointsFromContour()
{
    auto scanlab = new precitec::hardware::Scanlab();

    auto contourSize = 1000;
    QCOMPARE(scanlab->numberOfPointsFromContour(contourSize), 200);

    contourSize = 10;
    QCOMPARE(scanlab->numberOfPointsFromContour(contourSize), 2);

    contourSize = 500;
    QCOMPARE(scanlab->numberOfPointsFromContour(contourSize), 100);

    delete scanlab;
}

void CheckListOverflowTest::testNumberOfPossiblePointsForListMemory()
{
    auto scanlab = new precitec::hardware::Scanlab();

    QCOMPARE(scanlab->numberOfPossiblePointsForListMemory(), 62400);

    delete scanlab;
}

void CheckListOverflowTest::testIfCheckWorks()
{
    auto scanlab = new precitec::hardware::Scanlab();

    auto contourSize = 250000;
    QVERIFY(!(scanlab->numberOfPointsFromContour(contourSize) > scanlab->numberOfPossiblePointsForListMemory()));
    contourSize = 312000;                                                   //312000 = RTC6_NUMBER_OF_INFORMATION_FOR_ONE_POINT (5) * contourSize
    QVERIFY(!(scanlab->numberOfPointsFromContour(contourSize) > scanlab->numberOfPossiblePointsForListMemory()));
    contourSize = 312005;
    QVERIFY(scanlab->numberOfPointsFromContour(contourSize) > scanlab->numberOfPossiblePointsForListMemory());
    contourSize = 1000000;
    QVERIFY(scanlab->numberOfPointsFromContour(contourSize) > scanlab->numberOfPossiblePointsForListMemory());
    contourSize = 10;
    QVERIFY(!(scanlab->numberOfPointsFromContour(contourSize) > scanlab->numberOfPossiblePointsForListMemory()));

    delete scanlab;
}

QTEST_GUILESS_MAIN(CheckListOverflowTest)
#include "checkListOverflowTest.moc"
