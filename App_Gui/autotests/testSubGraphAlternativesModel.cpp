#include <QTest>
#include <QSignalSpy>

#include "subGraphModel.h"
#include "../src/subGraphAlternativesModel.h"

using precitec::gui::SubGraphAlternativesModel;
using precitec::storage::SubGraphModel;

class TestSubGraphAlternativesModel : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testCtor();
    void testAlternatives();
};

void TestSubGraphAlternativesModel::testCtor()
{
    SubGraphAlternativesModel model;
    QVERIFY(model.subGraphModel() == nullptr);
    QCOMPARE(model.selectedIndex(), QModelIndex{});

    std::unique_ptr<SubGraphModel> subGraphModel = std::make_unique<SubGraphModel>(new SubGraphModel());

    QSignalSpy subGraphModelChangedSpy{&model, &SubGraphAlternativesModel::subGraphModelChanged};
    QVERIFY(subGraphModelChangedSpy.isValid());

    model.setSubGraphModel(subGraphModel.get());
    QCOMPARE(model.subGraphModel(), subGraphModel.get());
    QCOMPARE(subGraphModelChangedSpy.count(), 1);

    subGraphModel.reset();
    QCOMPARE(subGraphModelChangedSpy.count(), 2);
    QVERIFY(model.subGraphModel() == nullptr);
}

void TestSubGraphAlternativesModel::testAlternatives()
{
    SubGraphModel graphModel;
    QSignalSpy loadingChangedSpy{&graphModel, &SubGraphModel::loadingChanged};
    graphModel.loadSubGraphs(QFINDTESTDATA("../../Mod_Storage/autotests/testdata/subgraphs/"), QFINDTESTDATA("testdata/subGraphAlternativesModel"));
    QCOMPARE(loadingChangedSpy.count(), 1);
    QCOMPARE(graphModel.isLoading(), true);
    QVERIFY(loadingChangedSpy.wait());
    QCOMPARE(loadingChangedSpy.count(), 2);
    QCOMPARE(graphModel.isLoading(), false);

    QCOMPARE(graphModel.rowCount(), 5);

    QCOMPARE(graphModel.checkedGraphs().empty(), true);

    const QModelIndex inspectionLineFinding = graphModel.indexFor({QByteArrayLiteral("0940cf62-a092-4d9f-b59d-2edd97e038d5")});
    const QModelIndex lineFinding= graphModel.indexFor({QByteArrayLiteral("7772eaab-acf4-47cd-86dc-02affc8c68c0")});

    // laser source, line finding, line profile
    graphModel.check({{QByteArrayLiteral("6932b184-42b5-4a42-8417-7ff81d066cab"), QByteArrayLiteral("ca23f1a1-874d-4383-95fb-f22b65c513e9"), QByteArrayLiteral("7772eaab-acf4-47cd-86dc-02affc8c68c0")}});

    SubGraphAlternativesModel model;
    model.setSubGraphModel(&graphModel);
    model.setSelectedIndex(lineFinding);
    QCOMPARE(model.selectedIndex(), lineFinding);

    QCOMPARE(model.rowCount(), 1);
    QCOMPARE(model.mapToSource(model.index(0, 0)), inspectionLineFinding);
}

QTEST_GUILESS_MAIN(TestSubGraphAlternativesModel)
#include "testSubGraphAlternativesModel.moc"
