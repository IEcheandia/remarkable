#include <QTest>
#include <QSignalSpy>
#include <QJsonDocument>
#include <QJsonObject>

#include "../src/resultSetting.h"

using precitec::storage::ResultSetting;

class TestResultConfig : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testCtor();
    void testSetVisualization();
    void testSerializationOfVisualization_data();
    void testSerializationOfVisualization();
    void testSerializationOfVisualizationInvalid_data();
    void testSerializationOfVisualizationInvalid();
    void testSerializationWithMissingVisualization();
    void testSetVisualizationThroughUpdateValue();
};

void TestResultConfig::testCtor()
{
    ResultSetting setting;
    QCOMPARE(setting.visualization(), ResultSetting::Visualization::Plot2D);
    QCOMPARE(setting.property("visualization").value<ResultSetting::Visualization>(), ResultSetting::Visualization::Plot2D);
}

void TestResultConfig::testSetVisualization()
{
    ResultSetting setting;
    QSignalSpy visualizationChangedSpy{&setting, &ResultSetting::visualizationChanged};
    QVERIFY(visualizationChangedSpy.isValid());

    QCOMPARE(setting.visualization(), ResultSetting::Visualization::Plot2D);
    QCOMPARE(setting.property("visualization").value<ResultSetting::Visualization>(), ResultSetting::Visualization::Plot2D);
    // go to binary
    setting.setVisualization(ResultSetting::Visualization::Binary);
    QCOMPARE(visualizationChangedSpy.count(), 1);
    QCOMPARE(setting.visualization(), ResultSetting::Visualization::Binary);
    QCOMPARE(setting.property("visualization").value<ResultSetting::Visualization>(), ResultSetting::Visualization::Binary);
    // setting same should not change
    setting.setVisualization(ResultSetting::Visualization::Binary);
    QCOMPARE(visualizationChangedSpy.count(), 1);

    // go to 2D
    setting.setVisualization(ResultSetting::Visualization::Plot2D);
    QCOMPARE(visualizationChangedSpy.count(), 2);
    QCOMPARE(setting.visualization(), ResultSetting::Visualization::Plot2D);
    QCOMPARE(setting.property("visualization").value<ResultSetting::Visualization>(), ResultSetting::Visualization::Plot2D);
    // setting same should not change
    setting.setVisualization(ResultSetting::Visualization::Plot2D);
    QCOMPARE(visualizationChangedSpy.count(), 2);

    // go to 3D
    setting.setVisualization(ResultSetting::Visualization::Plot3D);
    QCOMPARE(visualizationChangedSpy.count(), 3);
    QCOMPARE(setting.visualization(), ResultSetting::Visualization::Plot3D);
    QCOMPARE(setting.property("visualization").value<ResultSetting::Visualization>(), ResultSetting::Visualization::Plot3D);
    // setting same should not change
    setting.setVisualization(ResultSetting::Visualization::Plot3D);
    QCOMPARE(visualizationChangedSpy.count(), 3);
}

void TestResultConfig::testSerializationOfVisualization_data()
{
    QTest::addColumn<ResultSetting::Visualization>("visualization");

    QTest::newRow("binary") << ResultSetting::Visualization::Binary;
    QTest::newRow("plot2d") << ResultSetting::Visualization::Plot2D;
    QTest::newRow("plot3d") << ResultSetting::Visualization::Plot3D;
}

void TestResultConfig::testSerializationOfVisualization()
{
    ResultSetting setting;
    QFETCH(ResultSetting::Visualization, visualization);
    setting.setVisualization(visualization);
    const auto json = setting.toJson();

    auto loaded = ResultSetting::fromJson(json, &setting);
    QVERIFY(loaded);
    QCOMPARE(loaded->visualization(), visualization);
}

void TestResultConfig::testSerializationOfVisualizationInvalid_data()
{
    QTest::addColumn<int>("value");
    QTest::addColumn<ResultSetting::Visualization>("expected");

    QTest::newRow("-1") << -1 << ResultSetting::Visualization::Binary;
    QTest::newRow("3") << 3 << ResultSetting::Visualization::Plot3D;
    QTest::newRow("100") << 100 << ResultSetting::Visualization::Plot3D;
}

void TestResultConfig::testSerializationOfVisualizationInvalid()
{
    ResultSetting setting;
    auto json = setting.toJson();
    auto it = json.find(QLatin1String("visualization"));
    QVERIFY(it != json.end());
    QFETCH(int, value);
    it.value() = value;
    QCOMPARE(json.value(QLatin1String("visualization")).toInt(), value);
    auto loaded = ResultSetting::fromJson(json, &setting);
    QVERIFY(loaded);
    QTEST(loaded->visualization(), "expected");
}

void TestResultConfig::testSerializationWithMissingVisualization()
{
    ResultSetting setting;
    setting.setVisualization(ResultSetting::Visualization::Binary);
    auto json = setting.toJson();
    QCOMPARE(json.contains(QLatin1String("visualization")), true);
    json.erase(json.find(QLatin1String("visualization")));
    QCOMPARE(json.contains(QLatin1String("visualization")), false);
    auto loaded = ResultSetting::fromJson(json, &setting);
    QVERIFY(loaded);
    QCOMPARE(loaded->visualization(), ResultSetting::Visualization::Plot2D);
}

void TestResultConfig::testSetVisualizationThroughUpdateValue()
{
    ResultSetting setting;
    QSignalSpy visualizationChangedSpy{&setting, &ResultSetting::visualizationChanged};
    QVERIFY(visualizationChangedSpy.isValid());
    QCOMPARE(setting.visualization(), ResultSetting::Visualization::Plot2D);

    setting.updateValue(0, ResultSetting::Type::Visualization);
    QCOMPARE(setting.visualization(), ResultSetting::Visualization::Binary);
    QCOMPARE(visualizationChangedSpy.count(), 1);
    // same should not change
    setting.updateValue(0, ResultSetting::Type::Visualization);
    QCOMPARE(visualizationChangedSpy.count(), 1);

    setting.updateValue(1, ResultSetting::Type::Visualization);
    QCOMPARE(setting.visualization(), ResultSetting::Visualization::Plot2D);
    QCOMPARE(visualizationChangedSpy.count(), 2);

    // invalid value
    setting.updateValue(3, ResultSetting::Type::Visualization);
    setting.updateValue(-1, ResultSetting::Type::Visualization);
    QCOMPARE(setting.visualization(), ResultSetting::Visualization::Plot2D);
    QCOMPARE(visualizationChangedSpy.count(), 2);

    // and plot 3d
    setting.updateValue(2, ResultSetting::Type::Visualization);
    QCOMPARE(setting.visualization(), ResultSetting::Visualization::Plot3D);
    QCOMPARE(visualizationChangedSpy.count(), 3);
}

QTEST_GUILESS_MAIN(TestResultConfig)
#include "testResultConfig.moc"
