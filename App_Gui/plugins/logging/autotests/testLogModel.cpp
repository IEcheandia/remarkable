#include <QTest>
#include <QSignalSpy>

#include <cmath>

#include "../logModel.h"

#include "message/loggerGlobal.interface.h"

std::unique_ptr<precitec::ModuleLogger> g_pLogger = nullptr;

using precitec::interface::wmLogItem;
using precitec::interface::wmLogParam;
using precitec::gui::components::logging::LogModel;

typedef std::vector<precitec::interface::wmLogParam> LogParamList;

Q_DECLARE_METATYPE(LogParamList)

class TestLogModel : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void initTestCase();
    void testCtor();
    void testRoleNames();
    void testFormat_data();
    void testFormat();
    void testAddingData();
    void testThreadedReceiver();
    void testPaused();
};

void TestLogModel::initTestCase()
{
    qputenv("WM_STATION_NAME", "TESTLOGMODEL");
    g_pLogger = std::unique_ptr<precitec::ModuleLogger>(new precitec::ModuleLogger(precitec::system::module::ModuleName[precitec::system::module::Modules::UserInterfaceModul]));
}

void TestLogModel::testCtor()
{
    LogModel model;
    QCOMPARE(model.rowCount(QModelIndex()), 0);
    QVERIFY(model.moduleModel());
    QCOMPARE(model.latestWarningOrError(), QString{});
    QCOMPARE(model.property("maximumLogMessage").toInt(), 5000);
    QCOMPARE(model.canClear(), true);
    QCOMPARE(model.property("canClear").toBool(), true);
    QCOMPARE(model.isPaused(), false);
    QCOMPARE(model.property("paused").toBool(), false);
}

void TestLogModel::testRoleNames()
{
    LogModel model;
    const auto roles = model.roleNames();
    QCOMPARE(roles.size(), 4);
    QCOMPARE(roles.value(Qt::DisplayRole), QByteArrayLiteral("display"));
    QCOMPARE(roles.value(Qt::UserRole), QByteArrayLiteral("dateTime"));
    QCOMPARE(roles.value(Qt::UserRole+1), QByteArrayLiteral("module"));
    QCOMPARE(roles.value(Qt::UserRole+2), QByteArrayLiteral("level"));
}

void TestLogModel::testFormat_data()
{
    QTest::addColumn<QByteArray>("message");
    QTest::addColumn<LogParamList>("logParams");
    QTest::addColumn<QString>("expected");

    QTest::newRow("no params") << QByteArrayLiteral("This is a test message") << LogParamList{} << QStringLiteral("This is a test message");
    QTest::newRow("with percent") << QByteArrayLiteral("This is a %% message") << LogParamList{} << QStringLiteral("This is a % message");
    QTest::newRow("no marker") << QByteArrayLiteral("Message without marker") << LogParamList{wmLogParam{1}} << QStringLiteral("Message without marker");
    QTest::newRow("marker last char") << QByteArrayLiteral("Message with end%") << LogParamList{wmLogParam{1}} << QStringLiteral("Message with end%");
    QTest::newRow("%d") << QByteArrayLiteral("This is an %d message") << LogParamList{wmLogParam{1}} << QStringLiteral("This is an 1 message");
    QTest::newRow("%% and %d") << QByteArrayLiteral("This is a %% and %d message") << LogParamList{wmLogParam{1}} << QStringLiteral("This is a % and 1 message");
    QTest::newRow("%i") << QByteArrayLiteral("This is an %i message") << LogParamList{wmLogParam{1}} << QStringLiteral("This is an 1 message");
    QTest::newRow("%u") << QByteArrayLiteral("This is an %u message") << LogParamList{wmLogParam{-1}} << QStringLiteral("This is an 4294967295 message");
    QTest::newRow("%x") << QByteArrayLiteral("Red is 0x%x %x %x") << LogParamList{wmLogParam{255}, wmLogParam{0}, wmLogParam{0}} << QStringLiteral("Red is 0xff 0 0");
    QTest::newRow("%f") << QByteArrayLiteral("Pi is %f") << LogParamList{wmLogParam{M_PI}} << QStringLiteral("Pi is 3.14159");
    QTest::newRow("%s") << QByteArrayLiteral("%s %s") << LogParamList{wmLogParam{std::string{"Te%st"}}, wmLogParam{std::string{"me"}}} << QStringLiteral("Te%st me");
    QTest::newRow("%% after %s") << QByteArrayLiteral("%s%%") << LogParamList{wmLogParam{std::string{"Test"}}} << QStringLiteral("Test%");
}

void TestLogModel::testFormat()
{
    LogModel model;
    QFETCH(QByteArray, message);
    wmLogItem logItem(Poco::Timestamp(), 0, message.toStdString());
    QFETCH(LogParamList, logParams);
    for (const auto &param : logParams)
    {
        logItem.addParam(param);
    }

    QTEST(model.format(logItem), "expected");
}

void TestLogModel::testAddingData()
{
    LogModel model;
    QSignalSpy latestWarningChangedSpy(&model, &LogModel::latestWarningOrErrorChanged);
    QVERIFY(latestWarningChangedSpy.isValid());
    QSignalSpy modelResetSpy(&model, &LogModel::modelReset);
    QVERIFY(modelResetSpy.isValid());
    QSignalSpy rowsInsertedSpy(&model, &LogModel::rowsInserted);
    QVERIFY(rowsInsertedSpy.isValid());
    QSignalSpy rowsRemovedSpy(&model, &LogModel::rowsRemoved);
    QVERIFY(rowsRemovedSpy.isValid());

    QCOMPARE(model.rowCount(QModelIndex()), 0);
    model.addItems({
        wmLogItem{Poco::Timestamp(), 0, std::string("This is my log message")},
        wmLogItem{Poco::Timestamp(), 0, std::string("This is a warning log message"), std::string(), std::string("Test Module"), 2},
        wmLogItem{Poco::Timestamp(), 0, std::string("This is an error log message"), std::string(), std::string("Foo Module"), 4},
        wmLogItem{Poco::Timestamp(), 0, std::string("This is another log message"), std::string(), std::string("Test Module")}
    });
    QCOMPARE(rowsInsertedSpy.count(), 1);
    QCOMPARE(rowsInsertedSpy.last().at(0).toModelIndex(), QModelIndex());
    QCOMPARE(rowsInsertedSpy.last().at(1).toInt(), 0);
    QCOMPARE(rowsInsertedSpy.last().at(2).toInt(), 3);
    QCOMPARE(rowsRemovedSpy.count(), 0);
    QCOMPARE(latestWarningChangedSpy.count(), 1);
    QCOMPARE(model.rowCount(QModelIndex()), 4);
    QCOMPARE(model.index(0, 0).data(Qt::DisplayRole).toString(), QStringLiteral("This is my log message"));
    QCOMPARE(model.index(1, 0).data(Qt::DisplayRole).toString(), QStringLiteral("This is a warning log message"));
    QCOMPARE(model.index(2, 0).data(Qt::DisplayRole).toString(), QStringLiteral("This is an error log message"));
    QCOMPARE(model.index(3, 0).data(Qt::DisplayRole).toString(), QStringLiteral("This is another log message"));
    // not existing index
    QCOMPARE(model.index(4, 0).data(Qt::DisplayRole).toString(), QString());

    // module name
    QCOMPARE(model.index(0, 0).data(Qt::UserRole + 1).toByteArray(), QByteArrayLiteral("DummyModule"));
    QCOMPARE(model.index(1, 0).data(Qt::UserRole + 1).toByteArray(), QByteArrayLiteral("Test Module"));
    QCOMPARE(model.index(2, 0).data(Qt::UserRole + 1).toByteArray(), QByteArrayLiteral("Foo Module"));
    QCOMPARE(model.index(3, 0).data(Qt::UserRole + 1).toByteArray(), QByteArrayLiteral("Test Module"));

    // log level
    QCOMPARE(model.index(0, 0).data(Qt::UserRole + 2).value<LogModel::LogLevel>(), LogModel::LogLevel::Info);
    QCOMPARE(model.index(1, 0).data(Qt::UserRole + 2).value<LogModel::LogLevel>(), LogModel::LogLevel::Warning);
    QCOMPARE(model.index(2, 0).data(Qt::UserRole + 2).value<LogModel::LogLevel>(), LogModel::LogLevel::Error);
    QCOMPARE(model.index(3, 0).data(Qt::UserRole + 2).value<LogModel::LogLevel>(), LogModel::LogLevel::Info);
    QCOMPARE(model.latestWarningOrError(), model.index(2, 0).data().toString());

    // TODO: test timestamps

    // let's add some items so that items need to be removed
    QSignalSpy maximumLogMessageChangedSpy(&model, &LogModel::maximumLogMessageChanged);
    QVERIFY(maximumLogMessageChangedSpy.isValid());
    model.setProperty("maximumLogMessage", 5);
    QCOMPARE(model.property("maximumLogMessage").toInt(), 5);
    QCOMPARE(maximumLogMessageChangedSpy.count(), 1);

    model.addItems({
        wmLogItem{Poco::Timestamp(), 0, std::string("2This is my log message"), std::string("Key.Does.Not.Exist")},
        wmLogItem{Poco::Timestamp(), 0, std::string("2This is my log message2"), std::string(), std::string("Test Module")},
        wmLogItem{Poco::Timestamp(), 0, std::string("2This is my log message3"), std::string(), std::string("Foo Module")},
        wmLogItem{Poco::Timestamp(), 0, std::string("2This is my log message4"), std::string(), std::string("Test Module")}
    });
    QCOMPARE(rowsRemovedSpy.count(), 1);
    QCOMPARE(rowsRemovedSpy.first().at(0).toModelIndex(), QModelIndex());
    QCOMPARE(rowsRemovedSpy.first().at(1).toInt(), 0);
    QCOMPARE(rowsRemovedSpy.first().at(2).toInt(), 2);
    QCOMPARE(rowsInsertedSpy.count(), 2);
    QCOMPARE(rowsInsertedSpy.last().at(0).toModelIndex(), QModelIndex());
    QCOMPARE(rowsInsertedSpy.last().at(1).toInt(), 1);
    QCOMPARE(rowsInsertedSpy.last().at(2).toInt(), 4);
    QCOMPARE(model.rowCount(QModelIndex()), 5);

    QCOMPARE(model.index(0, 0).data(Qt::DisplayRole).toString(), QStringLiteral("This is another log message"));
    QCOMPARE(model.index(1, 0).data(Qt::DisplayRole).toString(), QStringLiteral("2This is my log message"));
    QCOMPARE(model.index(2, 0).data(Qt::DisplayRole).toString(), QStringLiteral("2This is my log message2"));
    QCOMPARE(model.index(3, 0).data(Qt::DisplayRole).toString(), QStringLiteral("2This is my log message3"));
    QCOMPARE(model.index(4, 0).data(Qt::DisplayRole).toString(), QStringLiteral("2This is my log message4"));

    // add more items than supported by the model
    // TODO: should we really allow more elements than capacity?
    QCOMPARE(modelResetSpy.count(), 0);
    model.addItems({
        wmLogItem{Poco::Timestamp(), 0, std::string("3This is my warning message"), std::string(), std::string("Test Module"), 2},
        wmLogItem{Poco::Timestamp(), 0, std::string("3This is my log message2"), std::string(), std::string("Test Module")},
        wmLogItem{Poco::Timestamp(), 0, std::string("3This is my log message3"), std::string(), std::string("Foo Module")},
        wmLogItem{Poco::Timestamp(), 0, std::string("3This is my warning message4"), std::string(), std::string("Test Module"), 2},
        wmLogItem{Poco::Timestamp(), 0, std::string("3This is my log message5"), std::string(), std::string("Test Module")},
        wmLogItem{Poco::Timestamp(), 0, std::string("3This is my log message6"), std::string(), std::string("Test Module")}
    });
    QCOMPARE(modelResetSpy.count(), 1);
    QCOMPARE(rowsInsertedSpy.count(), 2);
    QCOMPARE(rowsRemovedSpy.count(), 1);
    QCOMPARE(latestWarningChangedSpy.count(), 2);
    QCOMPARE(model.latestWarningOrError(), model.index(3, 0).data().toString());
    QCOMPARE(model.rowCount(QModelIndex()), 6);
    QCOMPARE(model.index(1, 0).data(Qt::DisplayRole).toString(), QStringLiteral("3This is my log message2"));
    QCOMPARE(model.index(2, 0).data(Qt::DisplayRole).toString(), QStringLiteral("3This is my log message3"));
    QCOMPARE(model.index(3, 0).data(Qt::DisplayRole).toString(), QStringLiteral("3This is my warning message4"));
    QCOMPARE(model.index(4, 0).data(Qt::DisplayRole).toString(), QStringLiteral("3This is my log message5"));
    QCOMPARE(model.index(5, 0).data(Qt::DisplayRole).toString(), QStringLiteral("3This is my log message6"));

    // let's add nothing
    model.addItems({});
    QCOMPARE(modelResetSpy.count(), 1);
    QCOMPARE(rowsInsertedSpy.count(), 2);
    QCOMPARE(rowsRemovedSpy.count(), 1);
    QCOMPARE(latestWarningChangedSpy.count(), 2);

    // let's clear
    model.clear();
    QCOMPARE(modelResetSpy.count(), 2);
    QCOMPARE(latestWarningChangedSpy.count(), 3);
    QCOMPARE(model.rowCount({}), 0);
    QCOMPARE(model.latestWarningOrError(), QString());

    // add exactly as many as max possible
    model.addItems({
        wmLogItem{Poco::Timestamp(), 0, std::string("4This is my warning message"), std::string(), std::string("Test Module"), 2},
        wmLogItem{Poco::Timestamp(), 0, std::string("4This is my log message2"), std::string(), std::string("Test Module")},
        wmLogItem{Poco::Timestamp(), 0, std::string("4This is my log message3"), std::string(), std::string("Foo Module")},
        wmLogItem{Poco::Timestamp(), 0, std::string("4This is my warning message4"), std::string(), std::string("Test Module"), 2},
        wmLogItem{Poco::Timestamp(), 0, std::string("4This is my log message5"), std::string(), std::string("Test Module")}
    });
    QCOMPARE(modelResetSpy.count(), 3);
    QCOMPARE(rowsInsertedSpy.count(), 2);
    QCOMPARE(rowsRemovedSpy.count(), 1);
    QCOMPARE(model.rowCount({}), 5);
}

void TestLogModel::testThreadedReceiver()
{
    LogModel model;
    model.setStation(QByteArrayLiteral("TESTLOGMODEL"));
    QSignalSpy rowsInsertedSpy(&model, &LogModel::rowsInserted);
    QVERIFY(rowsInsertedSpy.isValid());
    // no data yet, so we don't get any messages
    QVERIFY(!rowsInsertedSpy.wait(200));

    // let's add some data
    wmLog(precitec::eDebug, "This is my log message");
    wmLog(precitec::eWarning, "This is a warning log message");
    wmLog(precitec::eError, "This is an error log message");
    wmLog(precitec::eDebug, "This is another log message");
    QVERIFY(rowsInsertedSpy.wait());
    QTRY_COMPARE(model.rowCount(QModelIndex()), 4);
    // more logs
    wmLog(precitec::eDebug, "3This is my warning message");
    wmLog(precitec::eDebug, "3This is my log message2");
    wmLog(precitec::eDebug, "3This is my log message3");
    wmLog(precitec::eDebug, "3This is my warning message4");
    wmLog(precitec::eDebug, "3This is my log message5");
    wmLog(precitec::eDebug, "3This is my log message6");
    QVERIFY(rowsInsertedSpy.wait());
    QCOMPARE(model.rowCount(QModelIndex()), 10);
    QCOMPARE(model.index(0, 0).data(Qt::DisplayRole).toString(), QStringLiteral("This is my log message"));
    QCOMPARE(model.index(1, 0).data(Qt::DisplayRole).toString(), QStringLiteral("This is a warning log message"));
    QCOMPARE(model.index(2, 0).data(Qt::DisplayRole).toString(), QStringLiteral("This is an error log message"));
    QCOMPARE(model.index(3, 0).data(Qt::DisplayRole).toString(), QStringLiteral("This is another log message"));
    QCOMPARE(model.index(4, 0).data(Qt::DisplayRole).toString(), QStringLiteral("3This is my warning message"));
    QCOMPARE(model.index(5, 0).data(Qt::DisplayRole).toString(), QStringLiteral("3This is my log message2"));
    QCOMPARE(model.index(6, 0).data(Qt::DisplayRole).toString(), QStringLiteral("3This is my log message3"));
    QCOMPARE(model.index(7, 0).data(Qt::DisplayRole).toString(), QStringLiteral("3This is my warning message4"));
    QCOMPARE(model.index(8, 0).data(Qt::DisplayRole).toString(), QStringLiteral("3This is my log message5"));
    QCOMPARE(model.index(9, 0).data(Qt::DisplayRole).toString(), QStringLiteral("3This is my log message6"));
}

void TestLogModel::testPaused()
{
    LogModel model;
    model.setStation(QByteArrayLiteral("TESTLOGMODEL"));
    QSignalSpy rowsInsertedSpy(&model, &LogModel::rowsInserted);
    QVERIFY(rowsInsertedSpy.isValid());

    // test setting of paused
    QSignalSpy pausedChangedSpy{&model, &LogModel::pausedChanged};
    QVERIFY(pausedChangedSpy.isValid());
    QCOMPARE(model.isPaused(), false);
    model.setPaused(true);
    QCOMPARE(model.isPaused(), true);
    QCOMPARE(pausedChangedSpy.count(), 1);

    // adding log messages
    wmLog(precitec::eDebug, "This is my log message");
    wmLog(precitec::eDebug, "This is a warning log message");
    wmLog(precitec::eDebug, "This is an error log message");
    wmLog(precitec::eDebug, "This is another log message");
    // while paused we don't get any messages
    QVERIFY(!rowsInsertedSpy.wait(200));
    QCOMPARE(rowsInsertedSpy.count(), 0);
    QCOMPARE(model.rowCount({}), 0);

    // unpause
    model.setPaused(false);
    QCOMPARE(model.isPaused(), false);
    QCOMPARE(pausedChangedSpy.count(), 2);
    QCOMPARE(rowsInsertedSpy.count(), 1);

    QCOMPARE(model.rowCount(QModelIndex()), 4);

    // test the automatic deletion of items, when more than possible
    model.setPaused(true);
    model.setProperty("maximumLogMessage", 5);
    QSignalSpy latestWarningChangedSpy{&model, &LogModel::latestWarningOrErrorChanged};
    QVERIFY(latestWarningChangedSpy.isValid());
    wmLog(precitec::eDebug, "This is my log message 1");
    wmLog(precitec::eWarning, "This is a warning log message 2");
    wmLog(precitec::eDebug, "This is an error log message 3");
    wmLog(precitec::eDebug, "This is another log message 4");
    wmLog(precitec::eDebug, "This is another log message 5");
    QVERIFY(latestWarningChangedSpy.wait());
    wmLog(precitec::eDebug, "This is my log message 6");
    wmLog(precitec::eWarning, "This is a warning log message 7");
    QVERIFY(latestWarningChangedSpy.wait());
    wmLog(precitec::eDebug, "This is my log message 8");
    wmLog(precitec::eWarning, "This is a warning log message 9");
    wmLog(precitec::eDebug, "This is an error log message 10");
    wmLog(precitec::eDebug, "This is another log message 11");
    wmLog(precitec::eDebug, "This is another log message 12");
    QVERIFY(latestWarningChangedSpy.wait());
    model.setPaused(false);

    QCOMPARE(model.rowCount(QModelIndex()), 5);
    QCOMPARE(model.index(0, 0).data(Qt::DisplayRole).toString(), QStringLiteral("This is my log message 8"));
    QCOMPARE(model.index(1, 0).data(Qt::DisplayRole).toString(), QStringLiteral("This is a warning log message 9"));
    QCOMPARE(model.index(2, 0).data(Qt::DisplayRole).toString(), QStringLiteral("This is an error log message 10"));
    QCOMPARE(model.index(3, 0).data(Qt::DisplayRole).toString(), QStringLiteral("This is another log message 11"));
    QCOMPARE(model.index(4, 0).data(Qt::DisplayRole).toString(), QStringLiteral("This is another log message 12"));
}

QTEST_GUILESS_MAIN(TestLogModel)
#include "testLogModel.moc"
