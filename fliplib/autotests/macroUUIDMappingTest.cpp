#include <QTest>
#include <QSignalSpy>
#include "fliplib/macroUUIDMapping.h"

#include <Poco/UUID.h>

class MacroUUIDMappingTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testGetDeterministicUuidFromAnotherTwo();
    void testMacroUUIDMapping();
};

void MacroUUIDMappingTest::testGetDeterministicUuidFromAnotherTwo()
{
    Poco::UUID lUuid("34402b81-f532-4c2f-a3fa-917238672ec2");
    Poco::UUID rUuid("609d64ff-0f63-4191-b5f2-b67e13116582");

    QCOMPARE(QString::fromStdString(
                 fliplib::MacroUUIDMapping::getDeterministicUuidFromAnotherTwo(lUuid, rUuid).toString()),
             QString("7f0e7ddf-82de-3437-8bfc-8b1884668a22"));
}

void MacroUUIDMappingTest::testMacroUUIDMapping()
{
    // given
    auto macro = std::make_shared<fliplib::Macro>();
    macro->macroId = Poco::UUID("12302b81-f532-4c2f-a3fa-917238672ec2");
    macro->id = Poco::UUID("12302b81-f532-4c2f-a3fa-917238672ec2");

    fliplib::InstanceFilter instanceFilter;
    instanceFilter.id = Poco::UUID("34402b81-f532-4c2f-a3fa-917238672ec1");

    // then
    QCOMPARE(fliplib::MacroUUIDMapping::getDeterministicUuidFromAnotherTwo(macro->id, instanceFilter.id),
             Poco::UUID("882dd1be-ecd9-34f1-84b8-5f4962d59c17"));
}

QTEST_GUILESS_MAIN(MacroUUIDMappingTest)
#include "macroUUIDMappingTest.moc"