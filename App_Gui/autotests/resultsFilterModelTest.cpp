#include <QTest>

#include "../src/resultsFilterModel.h"
#include "../src/resultsModel.h"

using precitec::gui::ResultsFilterModel;
using precitec::gui::ResultsModel;

class ResultsFilterModelTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testCtor();
    void testSeamSeriesAvailable_data();
    void testSeamSeriesAvailable();
};

void ResultsFilterModelTest::testCtor()
{
    ResultsFilterModel filterModel;
    ResultsModel resultsModel;
    filterModel.setSourceModel(&resultsModel);
    QCOMPARE(filterModel.seamSeriesAvailable(), false);
    QCOMPARE(filterModel.rowCount(), 4);
}

void ResultsFilterModelTest::testSeamSeriesAvailable_data()
{
    QTest::addColumn<bool>("enabled");
    QTest::addColumn<int>("count");
    QTest::addColumn<QVector<ResultsModel::ResultsComponent>>("components");

    QTest::newRow("enabled") << true << 5 << QVector<ResultsModel::ResultsComponent>{
        ResultsModel::ResultsComponent::Product,
        ResultsModel::ResultsComponent::Instance,
        ResultsModel::ResultsComponent::Series,
        ResultsModel::ResultsComponent::Seam,
        ResultsModel::ResultsComponent::Results

    };
    QTest::newRow("disabled") << false << 4 << QVector<ResultsModel::ResultsComponent>{
        ResultsModel::ResultsComponent::Product,
        ResultsModel::ResultsComponent::Instance,
        ResultsModel::ResultsComponent::Seam,
        ResultsModel::ResultsComponent::Results
    };
}

void ResultsFilterModelTest::testSeamSeriesAvailable()
{
    ResultsFilterModel filterModel;
    ResultsModel resultsModel;
    filterModel.setSourceModel(&resultsModel);

    QFETCH(bool, enabled);
    filterModel.setSeamSeriesAvailable(enabled);
    QCOMPARE(filterModel.seamSeriesAvailable(), enabled);

    QTEST(filterModel.rowCount(), "count");

    QFETCH(QVector<ResultsModel::ResultsComponent>, components);
    QCOMPARE(components.count(), filterModel.rowCount());
    for (int i = 0; i < filterModel.rowCount(); i++)
    {
        QCOMPARE(filterModel.index(i, 0).data(Qt::UserRole + 1).value<ResultsModel::ResultsComponent>(), components.at(i));
    }
}

QTEST_GUILESS_MAIN(ResultsFilterModelTest)
#include "resultsFilterModelTest.moc"

