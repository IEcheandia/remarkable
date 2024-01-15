#include <QTest>
#include <QTemporaryDir>

#include "../include/viWeldHead/Scanlab/Scanlab.h"

/*
 * This class is used to check the definition of the pre position for accelerate the scanner to avoid burning in at the beginning.
 */
class FillVelocityTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void initTestCase();
    void testCtor();
    void testTransfromFromMMPerSToBitPerMS();
    void testFillMarkVelocity();
    void testFillJumpVelocity();

private:
    QTemporaryDir m_dir;
};

void FillVelocityTest::initTestCase()
{
    QVERIFY(m_dir.isValid());
    QVERIFY(QDir{}.mkpath(m_dir.filePath(QStringLiteral("config"))));
    qputenv("WM_BASE_DIR", m_dir.path().toUtf8());
}

void FillVelocityTest::testCtor()
{
    auto scanlabClass = new precitec::hardware::Scanlab();
    QCOMPARE(scanlabClass->m_oCalibValueBitsPerMM, 4000.0);
    QCOMPARE(scanlabClass->m_velocities.size(), 0);
    QCOMPARE(scanlabClass->m_lastValidMarkSpeedInBitsPerMs, 0.0);
    QCOMPARE(scanlabClass->m_lastValidJumpSpeedInBitsPerMs, 0.0);

    delete scanlabClass;
}

void FillVelocityTest::testTransfromFromMMPerSToBitPerMS()
{
    auto scanlabClass = new precitec::hardware::Scanlab();
    const double& velocityInMMPerS = 10000.0;
    QCOMPARE(scanlabClass->transformFromMMPerSToBitPerMS(velocityInMMPerS), velocityInMMPerS / 1000.0 * scanlabClass->m_oCalibValueBitsPerMM);

    delete scanlabClass;
}

void FillVelocityTest::testFillMarkVelocity()
{
    auto scanlabClass = new precitec::hardware::Scanlab();
    scanlabClass->m_lastValidMarkSpeedInBitsPerMs = 1000.0;

    scanlabClass->fillMarkVelocity(-1.0, 100.0);
    QCOMPARE(scanlabClass->m_velocities.back(), 1000.0);
    QCOMPARE(scanlabClass->m_lastValidMarkSpeedInBitsPerMs, 1000.0);
    QCOMPARE(scanlabClass->m_velocities.size(), 1);

    scanlabClass->fillMarkVelocity(-2.0, 100.0);
    QCOMPARE(scanlabClass->m_velocities.back(), 100.0);
    QCOMPARE(scanlabClass->m_lastValidMarkSpeedInBitsPerMs, 100.0);
    QCOMPARE(scanlabClass->m_velocities.size(), 2);

    scanlabClass->fillMarkVelocity(-1.0, 100.0);
    QCOMPARE(scanlabClass->m_velocities.back(), 100.0);
    QCOMPARE(scanlabClass->m_lastValidMarkSpeedInBitsPerMs, 100.0);
    QCOMPARE(scanlabClass->m_velocities.size(), 3);

    scanlabClass->fillMarkVelocity(1.0, 100.0);
    QCOMPARE(scanlabClass->m_velocities.back(), scanlabClass->transformFromMMPerSToBitPerMS(1.0));
    QCOMPARE(scanlabClass->m_lastValidMarkSpeedInBitsPerMs, scanlabClass->transformFromMMPerSToBitPerMS(1.0));
    QCOMPARE(scanlabClass->m_velocities.size(), 4);

    scanlabClass->fillMarkVelocity(5000.0, 100.0);
    QCOMPARE(scanlabClass->m_velocities.back(), scanlabClass->transformFromMMPerSToBitPerMS(5000.0));
    QCOMPARE(scanlabClass->m_lastValidMarkSpeedInBitsPerMs, scanlabClass->transformFromMMPerSToBitPerMS(5000.0));
    QCOMPARE(scanlabClass->m_velocities.size(), 5);

    scanlabClass->fillMarkVelocity(-1.0, 100.0);
    QCOMPARE(scanlabClass->m_velocities.back(), scanlabClass->transformFromMMPerSToBitPerMS(5000.0));
    QCOMPARE(scanlabClass->m_lastValidMarkSpeedInBitsPerMs, scanlabClass->transformFromMMPerSToBitPerMS(5000.0));
    QCOMPARE(scanlabClass->m_velocities.size(), 6);

    scanlabClass->fillMarkVelocity(-2.0, 100.0);
    QCOMPARE(scanlabClass->m_velocities.back(), 100.0);
    QCOMPARE(scanlabClass->m_lastValidMarkSpeedInBitsPerMs, 100.0);
    QCOMPARE(scanlabClass->m_velocities.size(), 7);

    delete scanlabClass;
}

void FillVelocityTest::testFillJumpVelocity()
{
    auto scanlabClass = new precitec::hardware::Scanlab();
    scanlabClass->m_lastValidJumpSpeedInBitsPerMs = 2000.0;

    scanlabClass->fillJumpVelocity(-1.0, 100.0);
    QCOMPARE(scanlabClass->m_velocities.back(), 2000.0);
    QCOMPARE(scanlabClass->m_lastValidJumpSpeedInBitsPerMs, 2000.0);
    QCOMPARE(scanlabClass->m_velocities.size(), 1);

    scanlabClass->fillJumpVelocity(-2.0, 100.0);
    QCOMPARE(scanlabClass->m_velocities.back(), 100.0);
    QCOMPARE(scanlabClass->m_lastValidJumpSpeedInBitsPerMs, 100.0);
    QCOMPARE(scanlabClass->m_velocities.size(), 2);

    scanlabClass->fillJumpVelocity(-1.0, 100.0);
    QCOMPARE(scanlabClass->m_velocities.back(), 100.0);
    QCOMPARE(scanlabClass->m_lastValidJumpSpeedInBitsPerMs, 100.0);
    QCOMPARE(scanlabClass->m_velocities.size(), 3);

    scanlabClass->fillJumpVelocity(1.0, 100.0);
    QCOMPARE(scanlabClass->m_velocities.back(), scanlabClass->transformFromMMPerSToBitPerMS(1.0));
    QCOMPARE(scanlabClass->m_lastValidJumpSpeedInBitsPerMs, scanlabClass->transformFromMMPerSToBitPerMS(1.0));
    QCOMPARE(scanlabClass->m_velocities.size(), 4);

    scanlabClass->fillJumpVelocity(5100.0, 100.0);
    QCOMPARE(scanlabClass->m_velocities.back(), scanlabClass->transformFromMMPerSToBitPerMS(5100.0));
    QCOMPARE(scanlabClass->m_lastValidJumpSpeedInBitsPerMs, scanlabClass->transformFromMMPerSToBitPerMS(5100.0));
    QCOMPARE(scanlabClass->m_velocities.size(), 5);

    scanlabClass->fillJumpVelocity(-1.0, 100.0);
    QCOMPARE(scanlabClass->m_velocities.back(), scanlabClass->transformFromMMPerSToBitPerMS(5100.0));
    QCOMPARE(scanlabClass->m_lastValidJumpSpeedInBitsPerMs, scanlabClass->transformFromMMPerSToBitPerMS(5100.0));
    QCOMPARE(scanlabClass->m_velocities.size(), 6);

    scanlabClass->fillJumpVelocity(-2.0, 100.0);
    QCOMPARE(scanlabClass->m_velocities.back(), 100.0);
    QCOMPARE(scanlabClass->m_lastValidJumpSpeedInBitsPerMs, 100.0);
    QCOMPARE(scanlabClass->m_velocities.size(), 7);

    delete scanlabClass;
}

QTEST_GUILESS_MAIN(FillVelocityTest)
#include "fillVelocityTest.moc"
