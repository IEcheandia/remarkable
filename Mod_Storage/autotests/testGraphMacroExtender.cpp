#include <QTest>
#include <QSignalSpy>

#include "fliplib/graphMacroExtender.h"

#include "fliplib/GraphBuilderFactory.h"
#include "fliplib/macroUUIDMapping.h"
//#include "../src/graphExporter.h"
class TestGraphMacroExtender : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void testExtendedTargetGraphWithoutProperSettings();
    void testExtendedTargetGraphPipesWithXMLFileSettings();
};

void TestGraphMacroExtender::testExtendedTargetGraphWithoutProperSettings() {
    // given
    fliplib::GraphMacroExtender graphMacroExtender;
    fliplib::GraphContainer graphContainer;
    // when
    graphMacroExtender.extendTargetGraph(&graphContainer);
    // then
    QCOMPARE(graphContainer.pipes.size(), 0);
}

void TestGraphMacroExtender::testExtendedTargetGraphPipesWithXMLFileSettings()
{
    // given
    auto graphBuilder = fliplib::GraphBuilderFactory().create();

    std::unique_ptr<fliplib::GraphContainer> targetGraph =
                    std::make_unique<fliplib::GraphContainer>(fliplib::GraphContainer(graphBuilder->buildGraphDescription(
                        QFINDTESTDATA("testdata/graphs/macro/targetGraph5655Test.xml").toStdString())));
    std::unique_ptr<fliplib::GraphContainer> extendedTargetGraph = std::make_unique<fliplib::GraphContainer>(*targetGraph);

    std::unique_ptr<fliplib::GraphContainer> macroGraph =
                    std::make_unique<fliplib::GraphContainer>(fliplib::GraphContainer(graphBuilder->buildGraphDescription(
                    QFINDTESTDATA("testdata/graphs/macro/macroGraph5655Test.xml").toStdString())));
    std::unique_ptr<fliplib::GraphContainer> copyMacroGraph = std::make_unique<fliplib::GraphContainer>(*macroGraph);

    const auto macro = extendedTargetGraph->macros.begin().base();

    // when
    fliplib::GraphMacroExtender graphMacroExtender;
    graphMacroExtender.setMacroAndCorrespondentMacroGraph(macro, std::move(macroGraph));
    graphMacroExtender.extendTargetGraph(extendedTargetGraph.get());

    // then
    // please see autotests/graphs/macroGraph5655Test.xml and targetGraph5655Test.xml
    QCOMPARE(extendedTargetGraph->pipes.begin()->senderConnectorId, targetGraph->pipes.begin()->senderConnectorId);
    QCOMPARE(extendedTargetGraph->pipes.begin()->receiverConnectorId, copyMacroGraph->pipes.begin()->receiverConnectorId);
    QCOMPARE(extendedTargetGraph->pipes.rbegin()->senderConnectorId, targetGraph->pipes.rbegin()->senderConnectorId);
    QCOMPARE(extendedTargetGraph->pipes.rbegin()->receiverConnectorId, copyMacroGraph->pipes.rbegin()->receiverConnectorId);

//    to create corespondent extended target graph file and see it in GUI:
//    precitec::storage::GraphExporter exporter{*extendedTargetGraph};
//    QTemporaryDir dir;
//    QVERIFY(dir.isValid());
//    exporter.setFileName(dir.filePath(QStringLiteral("extendedTargetGraphContainer.xml")));
//    exporter.exportToXml();
//    QCOMPARE(1, 1);
}

QTEST_GUILESS_MAIN(TestGraphMacroExtender)
#include "testGraphMacroExtender.moc"
