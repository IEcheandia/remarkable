#include <QTest>
#include <QSignalSpy>

#include "fliplib/graphMacroExtender.h"

class TestGraphMacroExtender : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testCtor();
};

void TestGraphMacroExtender::testCtor()
{
    fliplib::GraphMacroExtender graphMacroExtender;
}

QTEST_GUILESS_MAIN(TestGraphMacroExtender)
#include "graphMacroExtenderTest.moc"
