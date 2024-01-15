#include <QTest>
#include <QSignalSpy>
#include <QTemporaryDir>

#include "../src/graphExporter.h"

#include "fliplib/GraphBuilderFactory.h"
#include "fliplib/XmlGraphBuilder.h"

#include <Poco/UUIDGenerator.h>

class GraphExporterTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testMinimal();
    void testGraphs_data();
    void testGraphs();
    void testMacro();
};

void GraphExporterTest::testMinimal()
{
    auto graphBuilder = fliplib::GraphBuilderFactory().create();
    auto origGraph = graphBuilder->buildGraphDescription(QFINDTESTDATA("testdata/graphs/minimal.xml").toStdString());

    precitec::storage::GraphExporter exporter{origGraph};
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    exporter.setFileName(dir.filePath(QStringLiteral("minimal.xml")));
    exporter.exportToXml();

    auto exportedGraph = graphBuilder->buildGraphDescription(dir.filePath(QStringLiteral("minimal.xml")).toStdString());
    QCOMPARE(exportedGraph.id, Poco::UUID("e58abf42-77a6-4456-9f78-56e002b38549"));
    QCOMPARE(exportedGraph.comment, std::string("This is a comment"));
    QCOMPARE(exportedGraph.name, std::string("Minimal"));
    QCOMPARE(exportedGraph.group, std::string(""));
    QCOMPARE(exportedGraph.sensors.size(), std::size_t(0));
    QCOMPARE(exportedGraph.errors.size(), std::size_t(0));
    QCOMPARE(exportedGraph.results.size(), std::size_t(0));
    QCOMPARE(exportedGraph.filterDescriptions.size(), std::size_t(0));
    QCOMPARE(exportedGraph.pipes.size(), std::size_t(0));
    QCOMPARE(exportedGraph.ports.size(), std::size_t(0));

    QCOMPARE(exportedGraph.filterGroups.size(), std::size_t(1));
    QCOMPARE(exportedGraph.filterGroups[0].parent, -1);
    QCOMPARE(exportedGraph.filterGroups[0].number, -1);
    QCOMPARE(exportedGraph.filterGroups[0].name, "Not grouped");

    QCOMPARE(exportedGraph.instanceFilters.size(), std::size_t(1));
    QCOMPARE(exportedGraph.instanceFilters[0].id, Poco::UUID("cb9f223c-6762-4d8e-9faa-bca71f951444"));
    QCOMPARE(exportedGraph.instanceFilters[0].filterId, Poco::UUID("be7d83e3-9b98-41f2-a484-d0e74e5e8ea2"));
    QCOMPARE(exportedGraph.instanceFilters[0].name, std::string("A Filter"));
    QCOMPARE(exportedGraph.instanceFilters[0].group, -1);
    QCOMPARE(exportedGraph.instanceFilters[0].position.x, 0);
    QCOMPARE(exportedGraph.instanceFilters[0].position.y, 0);
    QCOMPARE(exportedGraph.instanceFilters[0].position.width, 0);
    QCOMPARE(exportedGraph.instanceFilters[0].position.height, 0);
    QCOMPARE(exportedGraph.instanceFilters[0].extensions.size(), std::size_t(0));
    QCOMPARE(exportedGraph.instanceFilters[0].attributes.size(), std::size_t(1));

    QCOMPARE(exportedGraph.instanceFilters[0].attributes[0].attributeId, Poco::UUID("640BB3EE-6145-4E25-BBAB-015534C6C3C2"));
    QCOMPARE(exportedGraph.instanceFilters[0].attributes[0].instanceAttributeId, Poco::UUID("0b0e26ea-cd24-4298-af96-4b3655f62ff4"));
    QCOMPARE(exportedGraph.instanceFilters[0].attributes[0].instanceVariantId, Poco::UUID("e14b12b4-8e36-4e74-b37f-3d33b60d13a0"));
    QCOMPARE(exportedGraph.instanceFilters[0].attributes[0].variantId, Poco::UUID("640BB3EE-6145-4E25-BBAB-015534C6C3C2"));
    QCOMPARE(exportedGraph.instanceFilters[0].attributes[0].name, std::string("An attribute"));
    QCOMPARE(exportedGraph.instanceFilters[0].attributes[0].hasBlob, false);
    QCOMPARE(exportedGraph.instanceFilters[0].attributes[0].blob, -1);
    QCOMPARE(exportedGraph.instanceFilters[0].attributes[0].userLevel, 3);
    QCOMPARE(exportedGraph.instanceFilters[0].attributes[0].visible, true);
    QCOMPARE(exportedGraph.instanceFilters[0].attributes[0].publicity, true);
    QCOMPARE(exportedGraph.instanceFilters[0].attributes[0].value, Poco::DynamicAny(1));
}

void GraphExporterTest::testGraphs_data()
{
    QTest::addColumn<QString>("file");

    QTest::newRow("Inspection line finding.xml") << QFINDTESTDATA("testdata/subgraphs/inspection/Inspection line finding.xml");
    QTest::newRow("ImageSource to sink.xml") << QFINDTESTDATA("testdata/subgraphs/source/ImageSource to sink.xml");
    QTest::newRow("LaserPower to sink.xml") << QFINDTESTDATA("testdata/subgraphs/source/LaserPower to sink.xml");
    QTest::newRow("Line finding.xml") << QFINDTESTDATA("testdata/subgraphs/tracking/Line finding.xml");
    QTest::newRow("Line profile.xml") << QFINDTESTDATA("testdata/subgraphs/utility/Line profile.xml");
}

void GraphExporterTest::testGraphs()
{
    auto graphBuilder = fliplib::GraphBuilderFactory().create();
    QFETCH(QString, file);
    auto origGraph = graphBuilder->buildGraphDescription(file.toStdString());

    precitec::storage::GraphExporter exporter{origGraph};
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    exporter.setFileName(dir.filePath(QStringLiteral("graph.xml")));
    exporter.exportToXml();

    auto exportedGraph = graphBuilder->buildGraphDescription(dir.filePath(QStringLiteral("graph.xml")).toStdString());
    QCOMPARE(exportedGraph.id, origGraph.id);
    QCOMPARE(exportedGraph.comment, origGraph.comment);
    QCOMPARE(exportedGraph.name, origGraph.name);
    QCOMPARE(exportedGraph.sensors.size(), origGraph.sensors.size());
    QCOMPARE(exportedGraph.errors.size(), origGraph.errors.size());
    QCOMPARE(exportedGraph.errors.size(), std::size_t(0));
    QCOMPARE(exportedGraph.results.size(), origGraph.results.size());
    QCOMPARE(exportedGraph.results.size(), std::size_t(0));
    QCOMPARE(exportedGraph.filterDescriptions.size(), origGraph.filterDescriptions.size());
    QCOMPARE(exportedGraph.pipes.size(), origGraph.pipes.size());
    QCOMPARE(exportedGraph.ports.size(), origGraph.ports.size());
    QCOMPARE(exportedGraph.ports.size(), std::size_t(0));

    QCOMPARE(exportedGraph.filterGroups.size(), origGraph.filterGroups.size());
    for (std::size_t i = 0; i < exportedGraph.filterGroups.size(); i++)
    {
        QCOMPARE(exportedGraph.filterGroups[i].parent, origGraph.filterGroups[i].parent);
        QCOMPARE(exportedGraph.filterGroups[i].number, origGraph.filterGroups[i].number);
        QCOMPARE(exportedGraph.filterGroups[i].name, origGraph.filterGroups[i].name);
    }

    for (std::size_t i = 0; i < exportedGraph.filterDescriptions.size(); i++)
    {
        QCOMPARE(exportedGraph.filterDescriptions[i].id, origGraph.filterDescriptions[i].id);
        QCOMPARE(exportedGraph.filterDescriptions[i].name, origGraph.filterDescriptions[i].name);
        QCOMPARE(exportedGraph.filterDescriptions[i].version, origGraph.filterDescriptions[i].version);
        QCOMPARE(exportedGraph.filterDescriptions[i].component, origGraph.filterDescriptions[i].component);
        QCOMPARE(exportedGraph.filterDescriptions[i].componentId, origGraph.filterDescriptions[i].componentId);
    }

    for (std::size_t i = 0; i < exportedGraph.instanceFilters.size(); i++)
    {
        QCOMPARE(exportedGraph.instanceFilters[i].id, origGraph.instanceFilters[i].id);
        QCOMPARE(exportedGraph.instanceFilters[i].filterId, origGraph.instanceFilters[i].filterId);
        QCOMPARE(exportedGraph.instanceFilters[i].name, origGraph.instanceFilters[i].name);
        QCOMPARE(exportedGraph.instanceFilters[i].group, origGraph.instanceFilters[i].group);
        QCOMPARE(exportedGraph.instanceFilters[i].position.x, origGraph.instanceFilters[i].position.x);
        QCOMPARE(exportedGraph.instanceFilters[i].position.y, origGraph.instanceFilters[i].position.y);
        QCOMPARE(exportedGraph.instanceFilters[i].position.width, origGraph.instanceFilters[i].position.width);
        QCOMPARE(exportedGraph.instanceFilters[i].position.height, origGraph.instanceFilters[i].position.height);

        QCOMPARE(exportedGraph.instanceFilters[i].attributes.size(), origGraph.instanceFilters[i].attributes.size());
        for (std::size_t j = 0; j < exportedGraph.instanceFilters[i].attributes.size(); j++)
        {
            QCOMPARE(exportedGraph.instanceFilters[i].attributes[j].attributeId, origGraph.instanceFilters[i].attributes[j].attributeId);
            QCOMPARE(exportedGraph.instanceFilters[i].attributes[j].instanceAttributeId, origGraph.instanceFilters[i].attributes[j].instanceAttributeId);
            QCOMPARE(exportedGraph.instanceFilters[i].attributes[j].name, origGraph.instanceFilters[i].attributes[j].name);
            QCOMPARE(exportedGraph.instanceFilters[i].attributes[j].value, origGraph.instanceFilters[i].attributes[j].value);
            QCOMPARE(exportedGraph.instanceFilters[i].attributes[j].userLevel, origGraph.instanceFilters[i].attributes[j].userLevel);
            QCOMPARE(exportedGraph.instanceFilters[i].attributes[j].visible, origGraph.instanceFilters[i].attributes[j].visible);
            QCOMPARE(exportedGraph.instanceFilters[i].attributes[j].publicity, origGraph.instanceFilters[i].attributes[j].publicity);
            QCOMPARE(exportedGraph.instanceFilters[i].attributes[j].blob, origGraph.instanceFilters[i].attributes[j].blob);
            QCOMPARE(exportedGraph.instanceFilters[i].attributes[j].hasBlob, origGraph.instanceFilters[i].attributes[j].hasBlob);
            QCOMPARE(exportedGraph.instanceFilters[i].attributes[j].instanceVariantId, origGraph.instanceFilters[i].attributes[j].instanceVariantId);
            QCOMPARE(exportedGraph.instanceFilters[i].attributes[j].variantId, origGraph.instanceFilters[i].attributes[j].variantId);
            QCOMPARE(exportedGraph.instanceFilters[i].attributes[j].helpFile, origGraph.instanceFilters[i].attributes[j].helpFile);
        }

        QCOMPARE(exportedGraph.instanceFilters[i].extensions.size(), origGraph.instanceFilters[i].extensions.size());
        for (std::size_t j = 0; j < exportedGraph.instanceFilters[i].extensions.size(); j++)
        {
            QCOMPARE(exportedGraph.instanceFilters[i].extensions[j].id, origGraph.instanceFilters[i].extensions[j].id);
            QCOMPARE(exportedGraph.instanceFilters[i].extensions[j].localScope, origGraph.instanceFilters[i].extensions[j].localScope);
            QCOMPARE(exportedGraph.instanceFilters[i].extensions[j].type, origGraph.instanceFilters[i].extensions[j].type);
        }
    }

    for (std::size_t i = 0; i < exportedGraph.pipes.size(); i++)
    {
        QCOMPARE(exportedGraph.pipes[i].receiver, origGraph.pipes[i].receiver);
        QCOMPARE(exportedGraph.pipes[i].sender, origGraph.pipes[i].sender);
        QCOMPARE(exportedGraph.pipes[i].receiverConnectorName, origGraph.pipes[i].receiverConnectorName);
        QCOMPARE(exportedGraph.pipes[i].receiverConnectorGroup, origGraph.pipes[i].receiverConnectorGroup);
        QCOMPARE(exportedGraph.pipes[i].senderConnectorName, origGraph.pipes[i].senderConnectorName);
        QCOMPARE(exportedGraph.pipes[i].id, origGraph.pipes[i].id);
        QCOMPARE(exportedGraph.pipes[i].receiverConnectorId, origGraph.pipes[i].receiverConnectorId);
        QCOMPARE(exportedGraph.pipes[i].senderConnectorId, origGraph.pipes[i].senderConnectorId);
        QCOMPARE(exportedGraph.pipes[i].path, origGraph.pipes[i].path);
        QCOMPARE(exportedGraph.pipes[i].extensions.size(), origGraph.pipes[i].extensions.size());


        for (std::size_t j = 0; j < exportedGraph.pipes[i].extensions.size(); j++)
        {
            QCOMPARE(exportedGraph.pipes[i].extensions[j].id, origGraph.pipes[i].extensions[j].id);
            QCOMPARE(exportedGraph.pipes[i].extensions[j].localScope, origGraph.pipes[i].extensions[j].localScope);
            QCOMPARE(exportedGraph.pipes[i].extensions[j].type, origGraph.pipes[i].extensions[j].type);
        }
    }

    for (std::size_t i = 0; i < exportedGraph.sensors.size(); i++)
    {
        QCOMPARE(exportedGraph.sensors[i].id, origGraph.sensors[i].id);
        QCOMPARE(exportedGraph.sensors[i].name, origGraph.sensors[i].name);
        QCOMPARE(exportedGraph.sensors[i].enumType, origGraph.sensors[i].enumType);
        QCOMPARE(exportedGraph.sensors[i].visibility, origGraph.sensors[i].visibility);
        QCOMPARE(exportedGraph.sensors[i].plotable, origGraph.sensors[i].plotable);
        QCOMPARE(exportedGraph.sensors[i].saveType, origGraph.sensors[i].saveType);
        QCOMPARE(exportedGraph.sensors[i].selectable, origGraph.sensors[i].selectable);
    }

    // ports, results and errors are not tested as we don't have any test data
}

void GraphExporterTest::testMacro()
{
    // build a graph using GraphContainer as at the time of this writing there is no functionality yet to build a graph xml with macros
    fliplib::GraphContainer graph;
    graph.id = Poco::UUIDGenerator::defaultGenerator().createRandom();
    graph.name = std::string{"Graph with macro"};
    graph.fileName = std::string{"Graph_with_macro.xml"};
    graph.comment = std::string{"This is a comment"};
    graph.group = std::string{"This is a group"};

    graph.filterDescriptions.emplace_back(fliplib::FilterDescription{
        Poco::UUIDGenerator::defaultGenerator().createRandom(),
        std::string{"Filter1"},
        std::string{"Version 0.0.1"},
        std::string{"Component1"},
        Poco::UUIDGenerator::defaultGenerator().createRandom()
    });
    graph.filterDescriptions.emplace_back(fliplib::FilterDescription{
        Poco::UUIDGenerator::defaultGenerator().createRandom(),
        std::string{"Filter2"},
        std::string{"Version 0.0.1"},
        std::string{"Component2"},
        Poco::UUIDGenerator::defaultGenerator().createRandom()
    });

    fliplib::InstanceFilter filter1;
    filter1.filterId = graph.filterDescriptions.at(0).id;
    filter1.id = Poco::UUIDGenerator::defaultGenerator().createRandom();
    filter1.group = 1;
    filter1.name = std::string{"Filter with pipes to macro"};
    filter1.position.x = 0;
    filter1.position.y = 0;
    filter1.position.width = 80;
    filter1.position.height = 80;
    graph.instanceFilters.push_back(filter1);

    fliplib::InstanceFilter filter2;
    filter2.filterId = graph.filterDescriptions.at(1).id;
    filter2.id = Poco::UUIDGenerator::defaultGenerator().createRandom();
    filter2.group = 1;
    filter2.name = std::string{"Filter with pipes from macro"};
    filter2.position.x = 500;
    filter2.position.y = 0;
    filter2.position.width = 80;
    filter2.position.height = 80;
    graph.instanceFilters.push_back(filter2);

    fliplib::Macro macro1;
    macro1.macroId = Poco::UUIDGenerator::defaultGenerator().createRandom();
    macro1.id = Poco::UUIDGenerator::defaultGenerator().createRandom();
    macro1.group = 2;
    macro1.position.x = 100;
    macro1.position.y = 200;
    macro1.position.width = 160;
    macro1.position.height = 160;

    std::vector<fliplib::Macro::Connector> inConnectors1;
    std::vector<fliplib::Macro::Connector> outConnectors1;

    inConnectors1.emplace_back(fliplib::Macro::Connector{
        Poco::UUIDGenerator::defaultGenerator().createRandom(),
        std::string{"InConnector1"},
        fliplib::PipeConnector::DataType::Image
    });
    inConnectors1.emplace_back(fliplib::Macro::Connector{
        Poco::UUIDGenerator::defaultGenerator().createRandom(),
        std::string{"InConnector2"},
        fliplib::PipeConnector::DataType::Double
    });
    outConnectors1.emplace_back(fliplib::Macro::Connector{
        Poco::UUIDGenerator::defaultGenerator().createRandom(),
        std::string{"OutConnector1"},
        fliplib::PipeConnector::DataType::Image
    });
    outConnectors1.emplace_back(fliplib::Macro::Connector{
        Poco::UUIDGenerator::defaultGenerator().createRandom(),
        std::string{"OutConnector2"},
        fliplib::PipeConnector::DataType::Double
    });
    outConnectors1.emplace_back(fliplib::Macro::Connector{
        Poco::UUIDGenerator::defaultGenerator().createRandom(),
        std::string{"OutConnector3"},
        fliplib::PipeConnector::DataType::Double
    });
    macro1.inConnectors = inConnectors1;
    macro1.outConnectors = outConnectors1;

    graph.macros.push_back(macro1);

    fliplib::Macro macro2;
    macro2.macroId = Poco::UUIDGenerator::defaultGenerator().createRandom();
    macro2.id = Poco::UUIDGenerator::defaultGenerator().createRandom();
    macro2.group = 3;
    macro2.position.x = 300;
    macro2.position.y = 200;
    macro2.position.width = 160;
    macro2.position.height = 160;

    std::vector<fliplib::Macro::Connector> inConnectors2;
    std::vector<fliplib::Macro::Connector> outConnectors2;

    inConnectors2.emplace_back(fliplib::Macro::Connector{
        Poco::UUIDGenerator::defaultGenerator().createRandom(),
        std::string{"InConnector1"},
        fliplib::PipeConnector::DataType::Image
    });
    inConnectors2.emplace_back(fliplib::Macro::Connector{
        Poco::UUIDGenerator::defaultGenerator().createRandom(),
        std::string{"InConnector2"},
        fliplib::PipeConnector::DataType::Double
    });
    inConnectors2.emplace_back(fliplib::Macro::Connector{
        Poco::UUIDGenerator::defaultGenerator().createRandom(),
        std::string{"InConnector3"},
        fliplib::PipeConnector::DataType::Double
    });
    outConnectors2.emplace_back(fliplib::Macro::Connector{
        Poco::UUIDGenerator::defaultGenerator().createRandom(),
        std::string{"OutConnector"},
        fliplib::PipeConnector::DataType::Double
    });
    macro2.inConnectors = inConnectors2;
    macro2.outConnectors = outConnectors2;

    inConnectors2.at(0).position.x = 1;
    inConnectors2.at(0).position.y = 2;
    inConnectors2.at(0).position.width = 3;
    inConnectors2.at(0).position.height = 4;

    outConnectors2.at(0).position.x = 5;
    outConnectors2.at(0).position.y = 6;
    outConnectors2.at(0).position.width = 7;
    outConnectors2.at(0).position.height = 8;

    graph.inConnectors = inConnectors2;
    graph.outConnectors = outConnectors2;

    graph.macros.push_back(macro2);
    QVERIFY(macro1.macroId != macro2.macroId);

    graph.filterGroups.emplace_back(fliplib::FilterGroup{
        1,
        -1,
        std::string{"Filters"}
    });
    graph.filterGroups.emplace_back(fliplib::FilterGroup{
        2,
        -1,
        std::string{"Macro 1"}
    });
    graph.filterGroups.emplace_back(fliplib::FilterGroup{
        3,
        -1,
        std::string{"Macro 2"}
    });

    // pipes from filter1 to macro1
    graph.pipes.emplace_back(fliplib::Pipe{
        macro1.id,
        filter1.id,
        std::string{""},
        0,
        std::string{"Sender name"},
        Poco::UUIDGenerator::defaultGenerator().createRandom(),
        macro1.inConnectors.at(0).id,
        Poco::UUIDGenerator::defaultGenerator().createRandom(),
        std::string{""}
    });
    graph.pipes.emplace_back(fliplib::Pipe{
        macro1.id,
        filter1.id,
        std::string{""},
        0,
        std::string{"Sender name 2"},
        Poco::UUIDGenerator::defaultGenerator().createRandom(),
        macro1.inConnectors.at(1).id,
        Poco::UUIDGenerator::defaultGenerator().createRandom(),
        std::string{""}
    });

    // pipes from macro 1 to macro 2
    graph.pipes.emplace_back(fliplib::Pipe{
        macro2.id,
        macro1.id,
        std::string{""},
        0,
        macro1.outConnectors.at(0).name,
        Poco::UUIDGenerator::defaultGenerator().createRandom(),
        macro2.inConnectors.at(0).id,
        macro1.outConnectors.at(0).id,
        std::string{""}
    });
    graph.pipes.emplace_back(fliplib::Pipe{
        macro2.id,
        macro1.id,
        std::string{""},
        0,
        macro1.outConnectors.at(1).name,
        Poco::UUIDGenerator::defaultGenerator().createRandom(),
        macro2.inConnectors.at(1).id,
        macro1.outConnectors.at(1).id,
        std::string{""}
    });
    graph.pipes.emplace_back(fliplib::Pipe{
        macro2.id,
        macro1.id,
        std::string{""},
        0,
        macro1.outConnectors.at(2).name,
        Poco::UUIDGenerator::defaultGenerator().createRandom(),
        macro2.inConnectors.at(2).id,
        macro1.outConnectors.at(2).id,
        std::string{""}
    });

    // pipe from Macro 2 to filter 2
    graph.pipes.emplace_back(fliplib::Pipe{
        filter2.id,
        macro2.id,
        std::string{""},
        0,
        macro2.outConnectors.at(0).name,
        Poco::UUIDGenerator::defaultGenerator().createRandom(),
        Poco::UUIDGenerator::defaultGenerator().createRandom(),
        macro2.outConnectors.at(0).id,
        std::string{""}
    });

    precitec::storage::GraphExporter exporter{graph};
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    exporter.setFileName(dir.filePath(QStringLiteral("Graph_with_macro.xml")));
    exporter.exportToXml();

    auto graphBuilder = fliplib::GraphBuilderFactory().create();
    auto exportedGraph = graphBuilder->buildGraphDescription(dir.filePath(QStringLiteral("Graph_with_macro.xml")).toStdString());
    QCOMPARE(exportedGraph.id, graph.id);
    QCOMPARE(exportedGraph.comment, graph.comment);
    QCOMPARE(exportedGraph.name, graph.name);
    QCOMPARE(exportedGraph.group, std::string("This is a group"));

    QCOMPARE(exportedGraph.filterDescriptions.size(), 2u);
    QCOMPARE(exportedGraph.filterDescriptions.at(0).id, graph.filterDescriptions.at(0).id);
    QCOMPARE(exportedGraph.filterDescriptions.at(0).componentId, graph.filterDescriptions.at(0).componentId);
    QCOMPARE(exportedGraph.filterDescriptions.at(0).component, graph.filterDescriptions.at(0).component);
    QCOMPARE(exportedGraph.filterDescriptions.at(0).name, graph.filterDescriptions.at(0).name);
    QCOMPARE(exportedGraph.filterDescriptions.at(0).version, graph.filterDescriptions.at(0).version);
    QCOMPARE(exportedGraph.filterDescriptions.at(1).id, graph.filterDescriptions.at(1).id);
    QCOMPARE(exportedGraph.filterDescriptions.at(1).componentId, graph.filterDescriptions.at(1).componentId);
    QCOMPARE(exportedGraph.filterDescriptions.at(1).component, graph.filterDescriptions.at(1).component);
    QCOMPARE(exportedGraph.filterDescriptions.at(1).name, graph.filterDescriptions.at(1).name);
    QCOMPARE(exportedGraph.filterDescriptions.at(1).version, graph.filterDescriptions.at(1).version);

    QCOMPARE(exportedGraph.instanceFilters.size(), 2u);
    QCOMPARE(exportedGraph.instanceFilters.at(0).id, graph.instanceFilters.at(0).id);
    QCOMPARE(exportedGraph.instanceFilters.at(0).filterId, graph.instanceFilters.at(0).filterId);
    QCOMPARE(exportedGraph.instanceFilters.at(0).group, graph.instanceFilters.at(0).group);
    QVERIFY(exportedGraph.instanceFilters.at(0).group != -1);
    QCOMPARE(exportedGraph.instanceFilters.at(0).name, graph.instanceFilters.at(0).name);
    QCOMPARE(exportedGraph.instanceFilters.at(0).position.x, graph.instanceFilters.at(0).position.x);
    QCOMPARE(exportedGraph.instanceFilters.at(0).position.y, graph.instanceFilters.at(0).position.y);
    QCOMPARE(exportedGraph.instanceFilters.at(0).position.width, graph.instanceFilters.at(0).position.width);
    QCOMPARE(exportedGraph.instanceFilters.at(0).position.height, graph.instanceFilters.at(0).position.height);

    QCOMPARE(exportedGraph.instanceFilters.at(1).id, graph.instanceFilters.at(1).id);
    QCOMPARE(exportedGraph.instanceFilters.at(1).filterId, graph.instanceFilters.at(1).filterId);
    QCOMPARE(exportedGraph.instanceFilters.at(1).group, graph.instanceFilters.at(1).group);
    QVERIFY(exportedGraph.instanceFilters.at(1).group != -1);
    QCOMPARE(exportedGraph.instanceFilters.at(1).name, graph.instanceFilters.at(1).name);
    QCOMPARE(exportedGraph.instanceFilters.at(1).position.x, graph.instanceFilters.at(1).position.x);
    QCOMPARE(exportedGraph.instanceFilters.at(1).position.y, graph.instanceFilters.at(1).position.y);
    QCOMPARE(exportedGraph.instanceFilters.at(1).position.width, graph.instanceFilters.at(1).position.width);
    QCOMPARE(exportedGraph.instanceFilters.at(1).position.height, graph.instanceFilters.at(1).position.height);

    QCOMPARE(exportedGraph.macros.size(), 2u);
    QCOMPARE(exportedGraph.macros.at(0).macroId, graph.macros.at(0).macroId);
    QCOMPARE(exportedGraph.macros.at(0).id, graph.macros.at(0).id);
    QCOMPARE(exportedGraph.macros.at(0).group, graph.macros.at(0).group);
    QVERIFY(exportedGraph.macros.at(0).group != -1);
    QCOMPARE(exportedGraph.macros.at(0).position.x, graph.macros.at(0).position.x);
    QCOMPARE(exportedGraph.macros.at(0).position.y, graph.macros.at(0).position.y);
    QCOMPARE(exportedGraph.macros.at(0).position.width, graph.macros.at(0).position.width);
    QCOMPARE(exportedGraph.macros.at(0).position.height, graph.macros.at(0).position.height);
    QCOMPARE(exportedGraph.macros.at(0).inConnectors.size(), 2u);
    QCOMPARE(exportedGraph.macros.at(0).inConnectors.at(0).id, graph.macros.at(0).inConnectors.at(0).id);
    QCOMPARE(exportedGraph.macros.at(0).inConnectors.at(0).name, graph.macros.at(0).inConnectors.at(0).name);
    QCOMPARE(exportedGraph.macros.at(0).inConnectors.at(0).type, graph.macros.at(0).inConnectors.at(0).type);
    QCOMPARE(exportedGraph.macros.at(0).inConnectors.at(0).position.width, 0);
    QCOMPARE(exportedGraph.macros.at(0).inConnectors.at(0).position.height, 0);
    QCOMPARE(exportedGraph.macros.at(0).inConnectors.at(1).id, graph.macros.at(0).inConnectors.at(1).id);
    QCOMPARE(exportedGraph.macros.at(0).inConnectors.at(1).name, graph.macros.at(0).inConnectors.at(1).name);
    QCOMPARE(exportedGraph.macros.at(0).inConnectors.at(1).type, graph.macros.at(0).inConnectors.at(1).type);
    QCOMPARE(exportedGraph.macros.at(0).inConnectors.at(1).position.width, 0);
    QCOMPARE(exportedGraph.macros.at(0).inConnectors.at(1).position.height, 0);
    QCOMPARE(exportedGraph.macros.at(0).outConnectors.size(), 3u);
    QCOMPARE(exportedGraph.macros.at(0).outConnectors.at(0).id, graph.macros.at(0).outConnectors.at(0).id);
    QCOMPARE(exportedGraph.macros.at(0).outConnectors.at(0).name, graph.macros.at(0).outConnectors.at(0).name);
    QCOMPARE(exportedGraph.macros.at(0).outConnectors.at(0).type, graph.macros.at(0).outConnectors.at(0).type);
    QCOMPARE(exportedGraph.macros.at(0).outConnectors.at(0).position.width, 0);
    QCOMPARE(exportedGraph.macros.at(0).outConnectors.at(0).position.height, 0);
    QCOMPARE(exportedGraph.macros.at(0).outConnectors.at(1).id, graph.macros.at(0).outConnectors.at(1).id);
    QCOMPARE(exportedGraph.macros.at(0).outConnectors.at(1).name, graph.macros.at(0).outConnectors.at(1).name);
    QCOMPARE(exportedGraph.macros.at(0).outConnectors.at(1).type, graph.macros.at(0).outConnectors.at(1).type);
    QCOMPARE(exportedGraph.macros.at(0).outConnectors.at(1).position.width, 0);
    QCOMPARE(exportedGraph.macros.at(0).outConnectors.at(1).position.height, 0);
    QCOMPARE(exportedGraph.macros.at(0).outConnectors.at(2).id, graph.macros.at(0).outConnectors.at(2).id);
    QCOMPARE(exportedGraph.macros.at(0).outConnectors.at(2).name, graph.macros.at(0).outConnectors.at(2).name);
    QCOMPARE(exportedGraph.macros.at(0).outConnectors.at(2).type, graph.macros.at(0).outConnectors.at(2).type);
    QCOMPARE(exportedGraph.macros.at(0).outConnectors.at(2).position.width, 0);
    QCOMPARE(exportedGraph.macros.at(0).outConnectors.at(2).position.height, 0);

    QCOMPARE(exportedGraph.macros.at(1).macroId, graph.macros.at(1).macroId);
    QCOMPARE(exportedGraph.macros.at(1).id, graph.macros.at(1).id);
    QCOMPARE(exportedGraph.macros.at(1).group, graph.macros.at(1).group);
    QVERIFY(exportedGraph.macros.at(1).group != -1);
    QCOMPARE(exportedGraph.macros.at(1).position.x, graph.macros.at(1).position.x);
    QCOMPARE(exportedGraph.macros.at(1).position.y, graph.macros.at(1).position.y);
    QCOMPARE(exportedGraph.macros.at(1).position.width, graph.macros.at(1).position.width);
    QCOMPARE(exportedGraph.macros.at(1).position.height, graph.macros.at(1).position.height);
    QCOMPARE(exportedGraph.macros.at(1).inConnectors.size(), 3u);
    QCOMPARE(exportedGraph.macros.at(1).inConnectors.at(0).id, graph.macros.at(1).inConnectors.at(0).id);
    QCOMPARE(exportedGraph.macros.at(1).inConnectors.at(0).name, graph.macros.at(1).inConnectors.at(0).name);
    QCOMPARE(exportedGraph.macros.at(1).inConnectors.at(0).type, graph.macros.at(1).inConnectors.at(0).type);
    QCOMPARE(exportedGraph.macros.at(1).inConnectors.at(1).id, graph.macros.at(1).inConnectors.at(1).id);
    QCOMPARE(exportedGraph.macros.at(1).inConnectors.at(1).name, graph.macros.at(1).inConnectors.at(1).name);
    QCOMPARE(exportedGraph.macros.at(1).inConnectors.at(1).type, graph.macros.at(1).inConnectors.at(1).type);
    QCOMPARE(exportedGraph.macros.at(1).inConnectors.at(2).id, graph.macros.at(1).inConnectors.at(2).id);
    QCOMPARE(exportedGraph.macros.at(1).inConnectors.at(2).name, graph.macros.at(1).inConnectors.at(2).name);
    QCOMPARE(exportedGraph.macros.at(1).inConnectors.at(2).type, graph.macros.at(1).inConnectors.at(2).type);
    QCOMPARE(exportedGraph.macros.at(1).outConnectors.size(), 1u);
    QCOMPARE(exportedGraph.macros.at(1).outConnectors.at(0).id, graph.macros.at(1).outConnectors.at(0).id);
    QCOMPARE(exportedGraph.macros.at(1).outConnectors.at(0).name, graph.macros.at(1).outConnectors.at(0).name);
    QCOMPARE(exportedGraph.macros.at(1).outConnectors.at(0).type, graph.macros.at(1).outConnectors.at(0).type);

    QCOMPARE(exportedGraph.inConnectors.size(), 3u);
    QCOMPARE(exportedGraph.outConnectors.size(), 1u);

    QCOMPARE(exportedGraph.inConnectors.at(0).position.x, 1);
    QCOMPARE(exportedGraph.inConnectors.at(0).position.y, 2);
    QCOMPARE(exportedGraph.inConnectors.at(0).position.width, 3);
    QCOMPARE(exportedGraph.inConnectors.at(0).position.height, 4);
    QCOMPARE(exportedGraph.outConnectors.at(0).position.x, 5);
    QCOMPARE(exportedGraph.outConnectors.at(0).position.y, 6);
    QCOMPARE(exportedGraph.outConnectors.at(0).position.width, 7);
    QCOMPARE(exportedGraph.outConnectors.at(0).position.height, 8);

    QCOMPARE(exportedGraph.filterGroups.size(), 3u);
    QCOMPARE(exportedGraph.filterGroups.at(0).number, 1);
    QCOMPARE(exportedGraph.filterGroups.at(0).parent, -1);
    QCOMPARE(exportedGraph.filterGroups.at(0).name, std::string{"Filters"});
    QCOMPARE(exportedGraph.filterGroups.at(1).number, 2);
    QCOMPARE(exportedGraph.filterGroups.at(1).parent, -1);
    QCOMPARE(exportedGraph.filterGroups.at(1).name, std::string{"Macro 1"});
    QCOMPARE(exportedGraph.filterGroups.at(2).number, 3);
    QCOMPARE(exportedGraph.filterGroups.at(2).parent, -1);
    QCOMPARE(exportedGraph.filterGroups.at(2).name, std::string{"Macro 2"});

    QCOMPARE(exportedGraph.pipes.size(), 6u);
    QCOMPARE(exportedGraph.pipes.at(0).id, graph.pipes.at(0).id);
    QCOMPARE(exportedGraph.pipes.at(0).receiver, graph.pipes.at(0).receiver);
    QCOMPARE(exportedGraph.pipes.at(0).sender, graph.pipes.at(0).sender);
    QCOMPARE(exportedGraph.pipes.at(0).senderConnectorId, graph.pipes.at(0).senderConnectorId);
    QCOMPARE(exportedGraph.pipes.at(0).receiverConnectorId, graph.pipes.at(0).receiverConnectorId);
    QCOMPARE(exportedGraph.pipes.at(0).senderConnectorName, graph.pipes.at(0).senderConnectorName);

    QCOMPARE(exportedGraph.pipes.at(1).id, graph.pipes.at(1).id);
    QCOMPARE(exportedGraph.pipes.at(1).receiver, graph.pipes.at(1).receiver);
    QCOMPARE(exportedGraph.pipes.at(1).sender, graph.pipes.at(1).sender);
    QCOMPARE(exportedGraph.pipes.at(1).senderConnectorId, graph.pipes.at(1).senderConnectorId);
    QCOMPARE(exportedGraph.pipes.at(1).receiverConnectorId, graph.pipes.at(1).receiverConnectorId);
    QCOMPARE(exportedGraph.pipes.at(1).senderConnectorName, graph.pipes.at(1).senderConnectorName);

    QCOMPARE(exportedGraph.pipes.at(2).id, graph.pipes.at(2).id);
    QCOMPARE(exportedGraph.pipes.at(2).receiver, graph.pipes.at(2).receiver);
    QCOMPARE(exportedGraph.pipes.at(2).sender, graph.pipes.at(2).sender);
    QCOMPARE(exportedGraph.pipes.at(2).senderConnectorId, graph.pipes.at(2).senderConnectorId);
    QCOMPARE(exportedGraph.pipes.at(2).receiverConnectorId, graph.pipes.at(2).receiverConnectorId);
    QCOMPARE(exportedGraph.pipes.at(2).senderConnectorName, graph.pipes.at(2).senderConnectorName);

    QCOMPARE(exportedGraph.pipes.at(3).id, graph.pipes.at(3).id);
    QCOMPARE(exportedGraph.pipes.at(3).receiver, graph.pipes.at(3).receiver);
    QCOMPARE(exportedGraph.pipes.at(3).sender, graph.pipes.at(3).sender);
    QCOMPARE(exportedGraph.pipes.at(3).senderConnectorId, graph.pipes.at(3).senderConnectorId);
    QCOMPARE(exportedGraph.pipes.at(3).receiverConnectorId, graph.pipes.at(3).receiverConnectorId);
    QCOMPARE(exportedGraph.pipes.at(3).senderConnectorName, graph.pipes.at(3).senderConnectorName);

    QCOMPARE(exportedGraph.pipes.at(4).id, graph.pipes.at(4).id);
    QCOMPARE(exportedGraph.pipes.at(4).receiver, graph.pipes.at(4).receiver);
    QCOMPARE(exportedGraph.pipes.at(4).sender, graph.pipes.at(4).sender);
    QCOMPARE(exportedGraph.pipes.at(4).senderConnectorId, graph.pipes.at(4).senderConnectorId);
    QCOMPARE(exportedGraph.pipes.at(4).receiverConnectorId, graph.pipes.at(4).receiverConnectorId);
    QCOMPARE(exportedGraph.pipes.at(4).senderConnectorName, graph.pipes.at(4).senderConnectorName);

    QCOMPARE(exportedGraph.pipes.at(5).id, graph.pipes.at(5).id);
    QCOMPARE(exportedGraph.pipes.at(5).receiver, graph.pipes.at(5).receiver);
    QCOMPARE(exportedGraph.pipes.at(5).sender, graph.pipes.at(5).sender);
    QCOMPARE(exportedGraph.pipes.at(5).senderConnectorId, graph.pipes.at(5).senderConnectorId);
    QCOMPARE(exportedGraph.pipes.at(5).receiverConnectorId, graph.pipes.at(5).receiverConnectorId);
    QCOMPARE(exportedGraph.pipes.at(5).senderConnectorName, graph.pipes.at(5).senderConnectorName);
}

QTEST_GUILESS_MAIN(GraphExporterTest)
#include "testGraphExporter.moc"
