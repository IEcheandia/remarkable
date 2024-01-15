#include <QTest>

#include "../src/graphReference.h"

using precitec::storage::GraphReference;
using precitec::storage::SubGraphReference;
using precitec::storage::hasSubGraphs;
using precitec::storage::hasLinkedGraph;
using precitec::storage::LinkedGraphReference;
using precitec::storage::SingleGraphReference;

class GraphReferenceTest : public QObject
{
    Q_OBJECT
    QUuid graphUuid = QUuid::createUuid();
    QUuid seamUuid = QUuid::createUuid();
    SingleGraphReference singleRef{graphUuid};
    std::vector<QUuid> uuidVec = {QUuid::createUuid(), QUuid::createUuid(), QUuid::createUuid()};
    SubGraphReference subGraphRef{uuidVec};
    LinkedGraphReference linkedRef{seamUuid};

private Q_SLOTS:
    void testHasGraph_data();
    void testHasGraph();
    void testToStringForChanges_data();
    void testToStringForChanges();
};

void GraphReferenceTest::testHasGraph_data()
{
    QTest::addColumn<GraphReference>("input");
    QTest::addColumn<bool>("expectHasGraph");
    QTest::addColumn<bool>("expectHasSubGraphs");
    QTest::addColumn<bool>("expectHasLinkedGraph");

    // To be consistent with previous behaviour, default-constructed GraphReferences have an empty subGraph,
    // and empty subGraphs are considered to be a subGraph, contrary to empty graphs, which are not a graph,
    // see also ProductTest::testSeamSubGraph.
    QTest::addRow("Default constructed graph reference has subGraph") << GraphReference{} << false << true << false;
    QTest::addRow("Single graph reference with value") << GraphReference{singleRef} << true << false << false;
    QTest::addRow("Empty single graph reference") << GraphReference{SingleGraphReference{}} << false << false << false;
    QTest::addRow("SubGraph reference with value") << GraphReference{subGraphRef} << false << true << false;
    QTest::addRow("Empty SubGraph reference") << GraphReference{SubGraphReference{}} << false << true << false;
    QTest::addRow("Linked graph reference with value") << GraphReference{linkedRef} << false << false << true;
    QTest::addRow("Empty linked graph reference") << GraphReference{LinkedGraphReference{}} << false << false << true;
}

void GraphReferenceTest::testHasGraph()
{
    QFETCH(GraphReference, input);
    QTEST(hasGraph(input), "expectHasGraph");
    QTEST(hasSubGraphs(input), "expectHasSubGraphs");
    QTEST(hasLinkedGraph(input), "expectHasLinkedGraph");
}

void GraphReferenceTest::testToStringForChanges_data()
{
    QTest::addColumn<GraphReference>("input");
    QTest::addColumn<QString>("expected");

    QTest::addRow("SingleGraphReference") << GraphReference{singleRef} << graphUuid.toString();
    QString subGraphUuidString = uuidVec[0].toString().append(uuidVec[1].toString()).append(uuidVec[2].toString());
    QTest::addRow("SubGraphReference") << GraphReference{subGraphRef} << subGraphUuidString;
    QTest::addRow("LinkedGraphReference") << GraphReference{linkedRef} << QString{"Linked to Seam "} + seamUuid.toString();
}

void GraphReferenceTest::testToStringForChanges()
{
    QFETCH(GraphReference, input);
    QString actual = std::visit(precitec::storage::toStringForChangeTracking, input);
    QTEST(actual, "expected");
}

QTEST_GUILESS_MAIN(GraphReferenceTest)
#include "testGraphReference.moc"