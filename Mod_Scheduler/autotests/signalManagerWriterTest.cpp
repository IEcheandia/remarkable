#include <QTest>
#include <QObject>

#include <memory>

#include "Scheduler/signalManagerWriter.h"
#include "Scheduler/testTask.h"
#include "Scheduler/testTrigger.h"

using precitec::scheduler::SignalManager;
using precitec::scheduler::SignalManagerFactory;
using precitec::scheduler::SignalManagerWriter;
using precitec::scheduler::TestTask;
using precitec::scheduler::TestTrigger;

class SignalManagerWriterTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testInitializedWrite();
    void testTwoSignalManagersWrite();
};

void SignalManagerWriterTest::testInitializedWrite()
{
    SignalManagerWriter writer;
    QVERIFY(writer.write(std::cout, {}));
}

void SignalManagerWriterTest::testTwoSignalManagersWrite()
{
    nlohmann::json json1 = R"({
                          "Task":
                          {
                                "Name": "TestTask",
                                "Settings": {"IdAddressSender": "192.168.6.62"}
                          },
                          "Trigger":
                          {
                                "Name": "TestTrigger",
                                "Settings": {}
                          },
                          "Uuid": "ea975c11-8acc-4c30-b5cc-85f1b34dc13b"})"_json;

    std::set<SignalManager> signalManagers = {SignalManagerFactory::make(json1)};
    SignalManagerWriter writer;
    std::stringstream stringStream;
    writer.write(stringStream, signalManagers);

    QCOMPARE(QString::fromStdString(stringStream.str()),
             QString("[{\"Task\":{\"Name\":\"TestTask\","
                     "\"Settings\":{\"IdAddressSender\":\"192.168.6.62\"}},"
                     "\"Trigger\":{\"Name\":\"TestTrigger\",\"Settings\":{}},"
                     "\"Uuid\":\"ea975c11-8acc-4c30-b5cc-85f1b34dc13b\"}]"));
}

QTEST_GUILESS_MAIN(SignalManagerWriterTest)
#include "signalManagerWriterTest.moc"