#include <QTest>
#include <QSignalSpy>

#include "../src/groupController.h"
#include "../src/FilterConnector.h"
#include "../src/FilterGraph.h"
#include "../src/FilterGroup.h"
#include "../src/FilterNode.h"
#include "../src/plausibilityController.h"

#include "fliplib/graphContainer.h"

#include "graphModel.h"

#include <Poco/UUIDGenerator.h>

using namespace precitec::gui::components::grapheditor;

class GroupControllerTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void initTestCase();
    void cleanup();

    void testCtor();
    void testUpdateGroupLabel();
    void testAddToGroup();
    void testAddToGroupBySizeChange();
    void testAddGroupToGroup();
    void testSetPlausibilityController();
    void testCanExportToMacro();
    void testExportToMacro();

private:
    QQmlEngine *m_engine = nullptr;
    FilterGraph *m_filterGraph = nullptr;
    std::unique_ptr<QQmlComponent> m_connectorComponent;
};

void GroupControllerTest::initTestCase()
{
    m_engine = new QQmlEngine{this};
    QStringList paths = m_engine->importPathList();
    // append path to default library
    // we need to append it as with prepand it would be used for things like QtQuickControls and pick up system installed Qt
    paths << QStringLiteral("/usr/lib/x86_64-linux-gnu/qt5/qml");
    m_engine->setImportPathList(paths);

    qmlRegisterType<precitec::gui::components::grapheditor::FilterConnector>("grapheditor.components", 1, 0, "FilterPort");

    m_filterGraph = new FilterGraph{};
    m_engine->setContextForObject(m_filterGraph, m_engine->rootContext());
    QQuickItem filterGraphContainer;
    m_filterGraph->setContainerItem(new QQuickItem{});
    m_filterGraph->getContainerItem()->setX(0);
    m_filterGraph->getContainerItem()->setY(0);
    m_filterGraph->getContainerItem()->setWidth(10000);
    m_filterGraph->getContainerItem()->setHeight(10000);

    m_connectorComponent = std::make_unique<QQmlComponent>(m_engine);
    m_connectorComponent->setData(QByteArrayLiteral("import QtQuick 2.7; import grapheditor.components 1.0; FilterPort {}"), {});
    // using setProperty as the setter is protected
    m_filterGraph->setProperty("portDelegate", QVariant::fromValue(m_connectorComponent.get()));
}

void GroupControllerTest::cleanup()
{
    m_filterGraph->clear();
}

void GroupControllerTest::testCtor()
{
    GroupController controller{};
    QCOMPARE(controller.gridSize(), 1);
    QCOMPARE(controller.useGridSizeAutomatically(), false);
    QVERIFY(!controller.filterGraph());
    QVERIFY(!controller.plausibilityController());
}

void GroupControllerTest::testUpdateGroupLabel()
{
    fliplib::GraphContainer graph;
    graph.filterGroups.emplace_back(fliplib::FilterGroup{1, -1, "Test", Poco::UUID{"15a6c1b4-a981-49fc-860f-222a4f154f86"}});
    graph.filterGroups.emplace_back(fliplib::FilterGroup{2, -1, "Other", Poco::UUID{"15a6c1b4-a981-49fc-860f-222a4f154f86"}});
    QCOMPARE(graph.filterGroups.at(0).name, std::string{"Test"});
    QCOMPARE(graph.filterGroups.at(1).name, std::string{"Other"});

    FilterGroup group;
    group.setID(1);
    group.setLabel(QStringLiteral("Test"));

    GroupController controller{};
    QSignalSpy changedSpy{&controller, &GroupController::changed};
    controller.setActualGraph(&graph);

    QCOMPARE(changedSpy.count(), 0);
    controller.updateGroupLabel(&group, QStringLiteral("New Name"));
    QCOMPARE(changedSpy.count(), 1);

    QCOMPARE(graph.filterGroups.at(0).name, std::string{"New Name"});
    QCOMPARE(graph.filterGroups.at(1).name, std::string{"Other"});
    QCOMPARE(group.getLabel(), QStringLiteral("New Name"));
}

void GroupControllerTest::testAddToGroup()
{
    QVERIFY(m_engine);
    QVERIFY(m_filterGraph);

    fliplib::GraphContainer graph;

    fliplib::InstanceFilter filter;
    filter.group = -1;
    filter.filterId = Poco::UUID{"1bab2c68-09b9-410a-a144-a138f4f8dd86"};
    filter.id = Poco::UUID{"1c98aac2-dc68-4611-aef4-971e4ab7d2ce"};
    filter.name = std::string{"An instance filter"};
    filter.position.x = 10;
    filter.position.y = 10;
    graph.instanceFilters.push_back(filter);
    filter.group = -1;
    filter.filterId = Poco::UUID{"1bab2c68-09b9-410a-a144-a138f4f8dd86"};
    filter.id = Poco::UUID{"1a2c865c-479c-495b-8145-bdfe998e3f7f"};
    filter.name = std::string{"Another instance filter"};
    filter.position.x = 20;
    filter.position.y = 20;
    graph.instanceFilters.push_back(filter);

    FilterNode *node = dynamic_cast<FilterNode*>(m_filterGraph->insertNode<FilterNode>(qan::Node::delegate(*m_engine)));
    QVERIFY(node);
    node->setID(QUuid::fromString(QStringLiteral("1c98aac2-dc68-4611-aef4-971e4ab7d2ce")));
    node->setGroupID(-1);
    node->setItemGeometry(10, 10, 80, 80);

    GroupController controller{};
    QSignalSpy changedSpy{&controller, &GroupController::changed};
    controller.setActualGraph(&graph);
    controller.setFilterGraph(m_filterGraph);

    QVERIFY(!controller.find(1));
    auto group = controller.insertNewGroup(QPointF(-10, -10));
    QVERIFY(group);
    QCOMPARE(controller.find(1), group);
    QVERIFY(!controller.find(2));
    QCOMPARE(controller.idFromName(group->getLabel()), 1);
    qan::GroupItem groupItem;
    QQuickItem container;
    groupItem.setContainer(&container);
    container.setX(-10);
    container.setY(-10);
    container.setWidth(200);
    container.setHeight(200);
    m_engine->setContextForObject(&groupItem, m_engine->rootContext());
    QCOMPARE(group->getGraph(), m_filterGraph);
    groupItem.setGroup(group);
    groupItem.setGraph(m_filterGraph);
    group->setItem(&groupItem);
    controller.initGroupItem(&groupItem, QPointF{-10 + 25, -10 + 25});
    QCOMPARE(groupItem.getDroppable(), false);

    QCOMPARE(group->ID(), 1);
    QCOMPARE(group->getLabel(), QStringLiteral("G01 Group name"));
    QCOMPARE(changedSpy.count(), 1);

    QCOMPARE(graph.filterGroups.size(), 1u);
    QCOMPARE(graph.filterGroups.at(0).number, 1);
    QCOMPARE(graph.filterGroups.at(0).parent, -1);
    QCOMPARE(graph.filterGroups.at(0).name, "G01 Group name");

    controller.addToGroup(node, group);
    QCOMPARE(changedSpy.count(), 2);

    QCOMPARE(node->getItem()->parentItem(), &container);
    QCOMPARE(graph.instanceFilters.at(0).group, 1);
    QCOMPARE(graph.instanceFilters.at(0).position.x, 10);
    QCOMPARE(graph.instanceFilters.at(0).position.y, 10);
    QCOMPARE(node->groupID(), 1);
    QCOMPARE(node->getGroup(), group);
    QCOMPARE(node->getItemGeometry(), QRectF(20, 20, 80, 80));

    QCOMPARE(graph.instanceFilters.at(1).group, -1);
    QCOMPARE(graph.instanceFilters.at(1).position.x, 20);
    QCOMPARE(graph.instanceFilters.at(1).position.y, 20);

    // now remove again
    controller.ungroup(node);
    QCOMPARE(changedSpy.count(), 3);
    QCOMPARE(graph.instanceFilters.at(0).group, -1);
    QCOMPARE(graph.instanceFilters.at(0).position.x, 10);
    QCOMPARE(graph.instanceFilters.at(0).position.y, 10);
    QCOMPARE(node->groupID(), -1);
    QVERIFY(!node->getGroup());
    QCOMPARE(node->getItem()->parentItem(), m_filterGraph->getContainerItem());
    QCOMPARE(node->getItemGeometry(), QRectF(10, 10, 80, 80));
}

void GroupControllerTest::testAddToGroupBySizeChange()
{
    QVERIFY(m_engine);
    QVERIFY(m_filterGraph);

    fliplib::GraphContainer graph;

    fliplib::InstanceFilter filter;
    filter.group = -1;
    filter.filterId = Poco::UUID{"1bab2c68-09b9-410a-a144-a138f4f8dd86"};
    filter.id = Poco::UUID{"1c98aac2-dc68-4611-aef4-971e4ab7d2ce"};
    filter.name = std::string{"An instance filter"};
    filter.position.x = 1000;
    filter.position.y = 1000;
    graph.instanceFilters.push_back(filter);
    filter.group = -1;
    filter.filterId = Poco::UUID{"1bab2c68-09b9-410a-a144-a138f4f8dd86"};
    filter.id = Poco::UUID{"1a2c865c-479c-495b-8145-bdfe998e3f7f"};
    filter.name = std::string{"Another instance filter"};
    filter.position.x = 2000;
    filter.position.y = 2000;
    graph.instanceFilters.push_back(filter);

    FilterNode *node = dynamic_cast<FilterNode*>(m_filterGraph->insertNode<FilterNode>(qan::Node::delegate(*m_engine)));
    QVERIFY(node);
    node->setID(QUuid::fromString(QStringLiteral("1c98aac2-dc68-4611-aef4-971e4ab7d2ce")));
    node->setGroupID(-1);
    node->setItemGeometry(1000, 1000, 80, 80);

    FilterNode *node2 = dynamic_cast<FilterNode*>(m_filterGraph->insertNode<FilterNode>(qan::Node::delegate(*m_engine)));
    QVERIFY(node2);
    node2->setID(QUuid::fromString(QStringLiteral("1a2c865c-479c-495b-8145-bdfe998e3f7f")));
    node2->setGroupID(-1);
    node2->setItemGeometry(2000, 2000, 80, 80);

    GroupController controller{};
    QSignalSpy changedSpy{&controller, &GroupController::changed};
    controller.setActualGraph(&graph);
    controller.setFilterGraph(m_filterGraph);

    auto group = controller.insertNewGroup(QPointF(-10, -10));
    QVERIFY(group);
    qan::GroupItem groupItem;
    QQuickItem container;
    groupItem.setContainer(&container);
    container.setX(-10);
    container.setY(-10);
    container.setWidth(200);
    container.setHeight(200);
    m_engine->setContextForObject(&groupItem, m_engine->rootContext());
    QCOMPARE(group->getGraph(), m_filterGraph);
    groupItem.setGroup(group);
    groupItem.setGraph(m_filterGraph);
    group->setItem(&groupItem);
    controller.initGroupItem(&groupItem, QPointF{-10 + 25, -10 + 25});

    QCOMPARE(group->ID(), 1);
    QCOMPARE(group->getLabel(), QStringLiteral("G01 Group name"));
    QCOMPARE(changedSpy.count(), 1);

    controller.updateGroupSize(group, QSizeF{500, 500});
    QCOMPARE(changedSpy.count(), 2);
    QCOMPARE(graph.instanceFilters.at(0).group, -1);
    QCOMPARE(graph.instanceFilters.at(0).position.x, 1000);
    QCOMPARE(graph.instanceFilters.at(0).position.y, 1000);
    QCOMPARE(graph.instanceFilters.at(1).group, -1);
    QCOMPARE(graph.instanceFilters.at(1).position.x, 2000);
    QCOMPARE(graph.instanceFilters.at(1).position.y, 2000);

    controller.updateGroupSize(group, QSizeF{1500, 1500});
    // just changing size emits one changed and one for adding to the group
    QCOMPARE(changedSpy.count(), 4);

    QCOMPARE(node->getItem()->parentItem(), &container);
    QCOMPARE(graph.instanceFilters.at(0).group, 1);
    QCOMPARE(graph.instanceFilters.at(0).position.x, 1000);
    QCOMPARE(graph.instanceFilters.at(0).position.y, 1000);
    QCOMPARE(graph.instanceFilters.at(1).group, -1);
    QCOMPARE(graph.instanceFilters.at(1).position.x, 2000);
    QCOMPARE(graph.instanceFilters.at(1).position.y, 2000);
    QCOMPARE(node->groupID(), 1);
    QCOMPARE(node->getGroup(), group);
    QCOMPARE(node->getItem()->parentItem(), &container);
    QCOMPARE(node->getItemGeometry(), QRectF(1010, 1010, 80, 80));
    QCOMPARE(node2->getItemGeometry(), QRectF(2000, 2000, 80, 80));

    // let's move the group
    controller.updateGroupPosition(group, QPointF{0, 0});
    QCOMPARE(changedSpy.count(), 5);
    QCOMPARE(graph.instanceFilters.at(0).group, 1);
    QCOMPARE(graph.instanceFilters.at(0).position.x, 1010);
    QCOMPARE(graph.instanceFilters.at(0).position.y, 1010);
    QCOMPARE(graph.instanceFilters.at(1).group, -1);
    QCOMPARE(graph.instanceFilters.at(1).position.x, 2000);
    QCOMPARE(graph.instanceFilters.at(1).position.y, 2000);
    QCOMPARE(node->getItemGeometry(), QRectF(1010, 1010, 80, 80));
    QCOMPARE(node2->getItemGeometry(), QRectF(2000, 2000, 80, 80));
}

void GroupControllerTest::testAddGroupToGroup()
{
    QVERIFY(m_engine);
    QVERIFY(m_filterGraph);

    fliplib::GraphContainer graph;

    GroupController controller{};
    QSignalSpy changedSpy{&controller, &GroupController::changed};
    controller.setActualGraph(&graph);
    controller.setFilterGraph(m_filterGraph);

    auto group = controller.insertNewGroup(QPointF(-10, -10));
    QVERIFY(group);
    qan::GroupItem groupItem;
    QQuickItem container;
    groupItem.setContainer(&container);
    container.setX(-10);
    container.setY(-10);
    container.setWidth(200);
    container.setHeight(200);
    m_engine->setContextForObject(&groupItem, m_engine->rootContext());
    QCOMPARE(group->getGraph(), m_filterGraph);
    groupItem.setGroup(group);
    groupItem.setGraph(m_filterGraph);
    group->setItem(&groupItem);
    controller.initGroupItem(&groupItem, QPointF{-10 + 25, -10 + 25});
    QCOMPARE(graph.filterGroups.size(), 1u);
    QCOMPARE(graph.filterGroups.at(0).number, 1);
    QCOMPARE(graph.filterGroups.at(0).parent, -1);

    QCOMPARE(group->ID(), 1);
    QCOMPARE(group->getLabel(), QStringLiteral("G01 Group name"));
    QCOMPARE(changedSpy.count(), 1);

    auto group2 = controller.insertNewGroup(QPointF(-10, -10));
    QVERIFY(group2);
    qan::GroupItem groupItem2;
    QQuickItem container2;
    groupItem.setContainer(&container2);
    container2.setX(1000);
    container2.setY(1000);
    container2.setWidth(200);
    container2.setHeight(200);
    m_engine->setContextForObject(&groupItem2, m_engine->rootContext());
    QCOMPARE(group2->getGraph(), m_filterGraph);
    groupItem2.setGroup(group2);
    groupItem2.setGraph(m_filterGraph);
    group2->setItem(&groupItem2);
    controller.initGroupItem(&groupItem2, QPointF{-10 + 25, -10 + 25});
    QCOMPARE(graph.filterGroups.size(), 2u);
    QCOMPARE(graph.filterGroups.at(1).number, 2);
    QCOMPARE(graph.filterGroups.at(1).parent, -1);

    QCOMPARE(group2->ID(), 2);
    QCOMPARE(group2->getLabel(), QStringLiteral("G02 Group name"));
    QVERIFY(!group2->getGroup());
    QCOMPARE(changedSpy.count(), 2);

    controller.addToGroup(group2, group);
    QCOMPARE(changedSpy.count(), 2);
    QVERIFY(!group2->getGroup());
    QVERIFY(!group->getGroup());
    QCOMPARE(graph.filterGroups.size(), 2u);
    QCOMPARE(graph.filterGroups.at(0).number, 1);
    QCOMPARE(graph.filterGroups.at(0).parent, -1);
    QCOMPARE(graph.filterGroups.at(1).number, 2);
    QCOMPARE(graph.filterGroups.at(1).parent, -1);
}

void GroupControllerTest::testSetPlausibilityController()
{
    GroupController controller{};
    QSignalSpy plausibilityControllerChangedSpy{&controller, &GroupController::plausibilityControllerChanged};
    QVERIFY(plausibilityControllerChangedSpy.isValid());

    QVERIFY(!controller.plausibilityController());
    auto plausibilityController = std::make_unique<PlausibilityController>();

    controller.setPlausibilityController(plausibilityController.get());
    QVERIFY(controller.plausibilityController());
    QCOMPARE(controller.plausibilityController(), plausibilityController.get());
    QCOMPARE(plausibilityControllerChangedSpy.count(), 1);

    // setting same should not change
    controller.setPlausibilityController(plausibilityController.get());
    QCOMPARE(plausibilityControllerChangedSpy.count(), 1);

    // deleting should reset
    plausibilityController.reset();
    QVERIFY(!controller.plausibilityController());
    QCOMPARE(plausibilityControllerChangedSpy.count(), 2);
}

void GroupControllerTest::testCanExportToMacro()
{
    QVERIFY(m_engine);
    QVERIFY(m_filterGraph);

    GroupController controller{};
    PlausibilityController plausibilityController{};
    QSignalSpy changedSpy{&controller, &GroupController::canBeExportedToMacroChanged};
    QVERIFY(changedSpy.isValid());
    QVERIFY(!controller.canBeExportedToMacro());

    controller.setPlausibilityController(&plausibilityController);
    QVERIFY(!controller.canBeExportedToMacro());

    fliplib::GraphContainer graph;
    fliplib::InstanceFilter filter;
    filter.group = -1;
    filter.filterId = Poco::UUID{"1bab2c68-09b9-410a-a144-a138f4f8dd86"};
    filter.id = Poco::UUID{"1c98aac2-dc68-4611-aef4-971e4ab7d2ce"};
    filter.name = std::string{"An instance filter"};
    filter.position.x = 1000;
    filter.position.y = 1000;
    graph.instanceFilters.push_back(filter);

    FilterNode *node = dynamic_cast<FilterNode*>(m_filterGraph->insertNode<FilterNode>(qan::Node::delegate(*m_engine)));
    QVERIFY(node);
    node->setID(QUuid::fromString(QStringLiteral("1c98aac2-dc68-4611-aef4-971e4ab7d2ce")));
    node->setGroupID(-1);
    node->setItemGeometry(1000, 1000, 80, 80);

    controller.setActualGraph(&graph);
    controller.setFilterGraph(m_filterGraph);
    plausibilityController.setActualGraph(&graph);
    plausibilityController.setFilterGraph(m_filterGraph);

    // add a group
    auto group = controller.insertNewGroup(QPointF(-10, -10));
    QVERIFY(group);
    qan::GroupItem groupItem;
    QQuickItem container;
    groupItem.setContainer(&container);
    container.setX(-10);
    container.setY(-10);
    container.setWidth(200);
    container.setHeight(200);
    m_engine->setContextForObject(&groupItem, m_engine->rootContext());
    QCOMPARE(group->getGraph(), m_filterGraph);
    groupItem.setGroup(group);
    groupItem.setGraph(m_filterGraph);
    group->setItem(&groupItem);
    controller.initGroupItem(&groupItem, QPointF{-10 + 25, -10 + 25});
    QCOMPARE(graph.filterGroups.size(), 1u);
    QCOMPARE(graph.filterGroups.at(0).number, 1);
    QCOMPARE(graph.filterGroups.at(0).parent, -1);

    QCOMPARE(group->ID(), 1);
    QCOMPARE(group->getLabel(), QStringLiteral("G01 Group name"));

    // an empty group should not be exportable
    QVERIFY(!controller.canBeExportedToMacro());
    QVERIFY(!plausibilityController.checkGroup(group));
    m_filterGraph->clearSelection();
    m_filterGraph->addToSelection(*group);
    QVERIFY(!controller.canBeExportedToMacro());

    // add a filter to the group
    controller.addToGroup(node, group);
    m_filterGraph->clearSelection();
    m_filterGraph->addToSelection(*group);
    QCOMPARE(changedSpy.count(), 1);
    QVERIFY(controller.canBeExportedToMacro());
    m_filterGraph->clearSelection();
    QVERIFY(!controller.canBeExportedToMacro());
    QCOMPARE(changedSpy.count(), 2);

    // let's add an input connector to the filter
    auto newInputConnector = dynamic_cast<FilterConnector*> (m_filterGraph->insertFilterConnector(node, qan::NodeItem::Dock::Left, qan::PortItem::Type::In, QStringLiteral("foo")));
    newInputConnector->setID(QUuid::createUuid());
    newInputConnector->setTag(QStringLiteral("bar"));
    newInputConnector->setGroup(1);
    newInputConnector->setConnectorType(0);
    newInputConnector->setColorValue(newInputConnector->connectorType());
    newInputConnector->setMultiplicity(qan::PortItem::Multiplicity::Single);
    newInputConnector->setConnectionType(fliplib::PipeConnector::ConnectionType::Mandatory);
    newInputConnector->setGeometryInputConnector(node->getItemGeometry(), 0, 1);

    // now it should not be exportable
    m_filterGraph->addToSelection(*group);
    QVERIFY(!controller.canBeExportedToMacro());
    m_filterGraph->clearSelection();

    filter.group = -1;
    filter.filterId = Poco::UUID{"ca8ef5fd-a4d5-456b-bc4d-2540eb95bc22"};
    filter.id = Poco::UUID{"1a2c865c-479c-495b-8145-bdfe998e3f7f"};
    filter.name = std::string{"Another instance filter"};
    filter.position.x = 2000;
    filter.position.y = 2000;
    graph.instanceFilters.push_back(filter);
    FilterNode *node2 = dynamic_cast<FilterNode*>(m_filterGraph->insertNode<FilterNode>(qan::Node::delegate(*m_engine)));
    QVERIFY(node2);
    node2->setID(QUuid::fromString(QStringLiteral("1a2c865c-479c-495b-8145-bdfe998e3f7f")));
    node2->setGroupID(-1);
    node2->setItemGeometry(2000, 2000, 80, 80);
    // add matching output connector
    auto newOutputConnector = dynamic_cast<FilterConnector*> (m_filterGraph->insertFilterConnector(node2, qan::NodeItem::Dock::Right, qan::PortItem::Type::Out, QStringLiteral("foo")));
    newOutputConnector->setID(QUuid::createUuid());
    newOutputConnector->setTag(QStringLiteral("bar"));
    newOutputConnector->setGroup(1);
    newOutputConnector->setConnectorType(0);
    newOutputConnector->setColorValue(newOutputConnector->connectorType());
    newOutputConnector->setMultiplicity(qan::PortItem::Multiplicity::Multiple);
    newOutputConnector->setConnectionType(fliplib::PipeConnector::ConnectionType::Optional);
    newOutputConnector->setGeometryOutputConnector(node->getItemGeometry(), 0, 1);

    auto edge = m_filterGraph->insertEdge(node2, node);
    edge->setLabel(QUuid::createUuid().toString(QUuid::WithoutBraces));
    m_filterGraph->bindEdge(edge, newOutputConnector, newInputConnector);

    // node2 is not yet in the group, so it should not be exportable
    m_filterGraph->addToSelection(*group);
    QVERIFY(!controller.canBeExportedToMacro());
    m_filterGraph->clearSelection();

    // move node2 into the group
    controller.addToGroup(node2, group);
    // now it should work
    m_filterGraph->addToSelection(*group);
    QVERIFY(controller.canBeExportedToMacro());
    QCOMPARE(changedSpy.count(), 3);
}

void GroupControllerTest::testExportToMacro()
{
    QVERIFY(m_engine);
    QVERIFY(m_filterGraph);

    GroupController controller{};
    PlausibilityController plausibilityController{};
    controller.setPlausibilityController(&plausibilityController);

    fliplib::GraphContainer graph;
    fliplib::FilterDescription description;
    description.id = Poco::UUID{"1bab2c68-09b9-410a-a144-a138f4f8dd86"};
    description.name = "Filter 1";
    description.component = "Component";
    description.componentId = Poco::UUID{"b5e853d8-e0b1-4e24-a321-86da994b3dfa"};
    graph.filterDescriptions.push_back(description);
    description.id = Poco::UUID{"ca8ef5fd-a4d5-456b-bc4d-2540eb95bc22"};
    description.name = "Filter 2";
    graph.filterDescriptions.push_back(description);
    description.id = Poco::UUID{"1862c04b-f923-45f4-9ec8-ecbeea61273e"};
    description.name = "Filter 3";
    graph.filterDescriptions.push_back(description);

    fliplib::InstanceFilter filter;
    filter.group = -1;
    filter.filterId = Poco::UUID{"1bab2c68-09b9-410a-a144-a138f4f8dd86"};
    filter.id = Poco::UUID{"1c98aac2-dc68-4611-aef4-971e4ab7d2ce"};
    filter.name = std::string{"An instance filter"};
    filter.position.x = 1000;
    filter.position.y = 1000;
    graph.instanceFilters.push_back(filter);
    filter.group = -1;
    filter.filterId = Poco::UUID{"ca8ef5fd-a4d5-456b-bc4d-2540eb95bc22"};
    filter.id = Poco::UUID{"1a2c865c-479c-495b-8145-bdfe998e3f7f"};
    filter.name = std::string{"Another instance filter"};
    filter.position.x = 2000;
    filter.position.y = 2000;
    graph.instanceFilters.push_back(filter);
    filter.group = -1;
    filter.filterId = Poco::UUID{"1862c04b-f923-45f4-9ec8-ecbeea61273e"};
    filter.id = Poco::UUID{"473a9ff4-138d-47fc-a93a-f749e8ffe9c6"};
    filter.name = std::string{"Another instance filter"};
    filter.position.x = 3000;
    filter.position.y = 3000;
    graph.instanceFilters.push_back(filter);
    filter.group = -1;
    filter.filterId = Poco::UUID{"1bab2c68-09b9-410a-a144-a138f4f8dd86"};
    filter.id = Poco::UUID{"56f1b26b-3416-4bd1-95b1-700f79ac9f2c"};
    filter.name = std::string{"An instance filter 4"};
    filter.position.x = 4000;
    filter.position.y = 4000;
    graph.instanceFilters.push_back(filter);

    controller.setActualGraph(&graph);
    controller.setFilterGraph(m_filterGraph);
    plausibilityController.setActualGraph(&graph);
    plausibilityController.setFilterGraph(m_filterGraph);

    FilterNode *node = dynamic_cast<FilterNode*>(m_filterGraph->insertNode<FilterNode>(qan::Node::delegate(*m_engine)));
    QVERIFY(node);
    node->setID(QUuid::fromString(QStringLiteral("1c98aac2-dc68-4611-aef4-971e4ab7d2ce")));
    node->setGroupID(-1);
    node->setItemGeometry(1000, 1000, 80, 80);
    // let's add an input connector to the filter
    auto newInputConnector = dynamic_cast<FilterConnector*> (m_filterGraph->insertFilterConnector(node, qan::NodeItem::Dock::Left, qan::PortItem::Type::In, QStringLiteral("foo")));
    newInputConnector->setID(QUuid::createUuid());
    newInputConnector->setTag(QStringLiteral("bar"));
    newInputConnector->setGroup(1);
    newInputConnector->setConnectorType(0);
    newInputConnector->setColorValue(newInputConnector->connectorType());
    newInputConnector->setMultiplicity(qan::PortItem::Multiplicity::Single);
    newInputConnector->setConnectionType(fliplib::PipeConnector::ConnectionType::Mandatory);
    newInputConnector->setGeometryInputConnector(node->getItemGeometry(), 0, 1);
    FilterNode *node2 = dynamic_cast<FilterNode*>(m_filterGraph->insertNode<FilterNode>(qan::Node::delegate(*m_engine)));
    QVERIFY(node2);
    node2->setID(QUuid::fromString(QStringLiteral("1a2c865c-479c-495b-8145-bdfe998e3f7f")));
    node2->setGroupID(-1);
    node2->setItemGeometry(2000, 2000, 80, 80);
    FilterNode *node3 = dynamic_cast<FilterNode*>(m_filterGraph->insertNode<FilterNode>(qan::Node::delegate(*m_engine)));
    QVERIFY(node3);
    node3->setID(QUuid::fromString(QStringLiteral("473a9ff4-138d-47fc-a93a-f749e8ffe9c6")));
    node3->setGroupID(-1);
    node3->setItemGeometry(3000, 3000, 80, 80);
    FilterNode *node4 = dynamic_cast<FilterNode*>(m_filterGraph->insertNode<FilterNode>(qan::Node::delegate(*m_engine)));
    QVERIFY(node4);
    node4->setID(QUuid::fromString(QStringLiteral("56f1b26b-3416-4bd1-95b1-700f79ac9f2c")));
    node4->setGroupID(-1);
    node4->setItemGeometry(4000, 4000, 80, 80);
    // add matching output connector
    auto newOutputConnector = dynamic_cast<FilterConnector*> (m_filterGraph->insertFilterConnector(node2, qan::NodeItem::Dock::Right, qan::PortItem::Type::Out, QStringLiteral("foo")));
    newOutputConnector->setID(QUuid::createUuid());
    newOutputConnector->setTag(QStringLiteral("bar"));
    newOutputConnector->setGroup(1);
    newOutputConnector->setConnectorType(0);
    newOutputConnector->setColorValue(newOutputConnector->connectorType());
    newOutputConnector->setMultiplicity(qan::PortItem::Multiplicity::Multiple);
    newOutputConnector->setConnectionType(fliplib::PipeConnector::ConnectionType::Optional);
    newOutputConnector->setGeometryOutputConnector(node->getItemGeometry(), 0, 1);

    auto edge = m_filterGraph->insertEdge(node2, node);
    edge->setLabel(QUuid::createUuid().toString(QUuid::WithoutBraces));
    m_filterGraph->bindEdge(edge, newOutputConnector, newInputConnector);

    // add pipe to graph
    fliplib::Pipe pipe;
    pipe.id = Poco::UUID{edge->getLabel().toStdString()};
    pipe.receiver = Poco::UUID{node->ID().toString(QUuid::WithoutBraces).toStdString()};
    pipe.sender = Poco::UUID{node2->ID().toString(QUuid::WithoutBraces).toStdString()};
    pipe.receiverConnectorId = Poco::UUID{newInputConnector->ID().toString(QUuid::WithoutBraces).toStdString()};
    pipe.senderConnectorId = Poco::UUID{newOutputConnector->ID().toString(QUuid::WithoutBraces).toStdString()};
    graph.pipes.push_back(pipe);
    fliplib::Pipe pipe2;
    pipe2.id = Poco::UUIDGenerator::defaultGenerator().createOne();
    pipe2.receiver = Poco::UUID{node3->ID().toString(QUuid::WithoutBraces).toStdString()};
    pipe2.sender = Poco::UUID{node4->ID().toString(QUuid::WithoutBraces).toStdString()};
    pipe2.receiverConnectorId = Poco::UUIDGenerator::defaultGenerator().createOne();
    pipe2.senderConnectorId = Poco::UUIDGenerator::defaultGenerator().createOne();
    graph.pipes.push_back(pipe2);

    // add a group
    auto group = controller.insertNewGroup(QPointF(-10, -10));
    QVERIFY(group);
    qan::GroupItem groupItem;
    QQuickItem container;
    groupItem.setContainer(&container);
    container.setX(-10);
    container.setY(-10);
    container.setWidth(200);
    container.setHeight(200);
    m_engine->setContextForObject(&groupItem, m_engine->rootContext());
    QCOMPARE(group->getGraph(), m_filterGraph);
    groupItem.setGroup(group);
    groupItem.setGraph(m_filterGraph);
    group->setItem(&groupItem);
    controller.initGroupItem(&groupItem, QPointF{-10 + 25, -10 + 25});
    QCOMPARE(graph.filterGroups.size(), 1u);
    QCOMPARE(graph.filterGroups.at(0).number, 1);
    QCOMPARE(graph.filterGroups.at(0).parent, -1);

    controller.addToGroup(node, group);
    controller.addToGroup(node2, group);

    QCOMPARE(graph.instanceFilters.at(0).group, 1);
    QCOMPARE(graph.instanceFilters.at(1).group, 1);
    QCOMPARE(graph.instanceFilters.at(2).group, -1);

    m_filterGraph->addToSelection(*group);
    QVERIFY(controller.canBeExportedToMacro());

    QTemporaryDir dir{};
    QVERIFY(dir.isValid());

    controller.exportSelectedToMacro(dir.path(), QStringLiteral("TestGraph"), QStringLiteral("This is a comment"));
    QVERIFY(QFileInfo{dir.filePath(QStringLiteral("TestGraph.xml"))}.exists());

    precitec::storage::GraphModel graphModel;
    graphModel.loadGraphs(dir.path());
    QTRY_COMPARE(graphModel.rowCount(), 1);

    const auto exportedGraph = graphModel.graph(graphModel.index(0, 0));
    QCOMPARE(exportedGraph.name, std::string{"TestGraph"});
    QCOMPARE(exportedGraph.fileName, std::string{"TestGraph.xml"});
    QCOMPARE(exportedGraph.comment, std::string{"This is a comment"});

    QCOMPARE(exportedGraph.instanceFilters.size(), 2);
    QCOMPARE(exportedGraph.instanceFilters.at(0).id, Poco::UUID{"1c98aac2-dc68-4611-aef4-971e4ab7d2ce"});
    QCOMPARE(exportedGraph.instanceFilters.at(0).group, -1);
    QCOMPARE(exportedGraph.instanceFilters.at(1).id, Poco::UUID{"1a2c865c-479c-495b-8145-bdfe998e3f7f"});
    QCOMPARE(exportedGraph.instanceFilters.at(1).group, -1);

    QCOMPARE(exportedGraph.filterDescriptions.size(), 2);
    QCOMPARE(exportedGraph.filterDescriptions.at(0).id, Poco::UUID{"1bab2c68-09b9-410a-a144-a138f4f8dd86"});
    QCOMPARE(exportedGraph.filterDescriptions.at(1).id, Poco::UUID{"ca8ef5fd-a4d5-456b-bc4d-2540eb95bc22"});

    QCOMPARE(exportedGraph.pipes.size(), 1);
    QCOMPARE(exportedGraph.pipes.at(0).id, pipe.id);
}


QTEST_MAIN(GroupControllerTest)
#include "testGroupController.moc"
