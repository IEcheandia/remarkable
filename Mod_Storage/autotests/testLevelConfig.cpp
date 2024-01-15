#include <QTest>
#include <QSignalSpy>
#include <QJsonDocument>
#include <QJsonObject>

#include "../src/levelConfig.h"
#include "../src/intervalError.h"

using precitec::storage::LevelConfig;
using precitec::storage::IntervalError;

class TestLevelConfig : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void testCtor();
    void testMin();
    void testMax();
    void testThreshold();
    void testDuplicate();
    void testFromJson();
    void testToJson();
};

void TestLevelConfig::testCtor()
{
    LevelConfig config;

    QCOMPARE(config.min(), 0.0);
    QCOMPARE(config.max(), 0.0);
    QCOMPARE(config.threshold(), 0.0);
}

void TestLevelConfig::testMin()
{
    LevelConfig config;
    QCOMPARE(config.min(), 0.0);

    QSignalSpy minChangedSpy(&config, &LevelConfig::minChanged);
    QVERIFY(minChangedSpy.isValid());

    config.setMin(0.0);
    QCOMPARE(minChangedSpy.count(), 0);

    config.setMin(5.3);
    QCOMPARE(minChangedSpy.count(), 1);
    QCOMPARE(config.min(), 5.3);

    config.setMin(5.3);
    QCOMPARE(minChangedSpy.count(), 1);
}

void TestLevelConfig::testMax()
{
    LevelConfig config;
    QCOMPARE(config.max(), 0.0);

    QSignalSpy maxChangedSpy(&config, &LevelConfig::maxChanged);
    QVERIFY(maxChangedSpy.isValid());

    config.setMax(0.0);
    QCOMPARE(maxChangedSpy.count(), 0);

    config.setMax(5.3);
    QCOMPARE(maxChangedSpy.count(), 1);
    QCOMPARE(config.max(), 5.3);

    config.setMax(5.3);
    QCOMPARE(maxChangedSpy.count(), 1);
}

void TestLevelConfig::testThreshold()
{
    LevelConfig config;
    QCOMPARE(config.threshold(), 0.0);

    QSignalSpy thresholdChangedSpy(&config, &LevelConfig::thresholdChanged);
    QVERIFY(thresholdChangedSpy.isValid());

    config.setThreshold(0.0);
    QCOMPARE(thresholdChangedSpy.count(), 0);

    config.setThreshold(5.3);
    QCOMPARE(thresholdChangedSpy.count(), 1);
    QCOMPARE(config.threshold(), 5.3);

    config.setThreshold(5.3);
    QCOMPARE(thresholdChangedSpy.count(), 1);
}

void TestLevelConfig::testDuplicate()
{
    auto interval = new IntervalError{this};

    LevelConfig config;
    config.setMin(2.3);
    config.setMax(7.6);
    config.setThreshold(3.24);

    auto config2 = config.duplicate(interval);
    QCOMPARE(config2->parent(), interval);
    QCOMPARE(config2->min(), config.min());
    QCOMPARE(config2->max(), config.max());
    QCOMPARE(config2->threshold(), config.threshold());
}

void TestLevelConfig::testFromJson()
{
    const QString dir = QFINDTESTDATA("testdata/errors/level.json");

    QFile file(dir);
    if (!file.open(QIODevice::ReadOnly))
    {
        QVERIFY(false);
    }
    const QByteArray data = file.readAll();
    if (data.isEmpty())
    {
        QVERIFY(false);
    }
    const auto document = QJsonDocument::fromJson(data);
    if (document.isNull())
    {
        QVERIFY(false);
    }

    auto interval = new IntervalError{this};
    auto config = LevelConfig::fromJson(document.object(), interval);

    QCOMPARE(config->min(), 32.23);
    QCOMPARE(config->max(), 42.88);
    QCOMPARE(config->threshold(), 3.001);
}

void TestLevelConfig::testToJson()
{
    LevelConfig config;
    config.setMin(3.12);
    config.setMax(15.76);
    config.setThreshold(8.43);

    auto json = config.toJson();

    auto interval = new IntervalError{this};
    auto config2 = LevelConfig::fromJson(json, interval);
    QCOMPARE(config2->min(), config.min());
    QCOMPARE(config2->max(), config.max());
    QCOMPARE(config2->threshold(), config.threshold());
}

QTEST_GUILESS_MAIN(TestLevelConfig)
#include "testLevelConfig.moc"
