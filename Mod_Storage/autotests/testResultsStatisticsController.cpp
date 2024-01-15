#include <QTest>
#include <QSignalSpy>

#include "../src/resultsStatisticsController.h"
#include "../src/product.h"
#include "../src/seamSeries.h"
#include "../src/seam.h"
#include "event/resultType.h"

using precitec::storage::ResultsStatisticsController;
using precitec::storage::Product;
using precitec::interface::ResultType;

class TestResultsStatisticsController : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testCtor();
    void testCurrentProduct();
    void testResultsStoragePath();
    void testCalculate();
};

void TestResultsStatisticsController::testCtor()
{
    auto controller = new ResultsStatisticsController{this};

    QVERIFY(!controller->calculating());
    QVERIFY(!controller->currentProduct());
    QVERIFY(controller->empty());
    QCOMPARE(controller->io(), 0u);
    QCOMPARE(controller->nio(), 0u);
    QCOMPARE(controller->ioInPercent(), 0.0);
    QCOMPARE(controller->nioInPercent(), 0.0);
    QCOMPARE(controller->resultsStoragePath(), QLatin1String(""));
}

void TestResultsStatisticsController::testCurrentProduct()
{
    auto controller = new ResultsStatisticsController{this};
    QVERIFY(!controller->currentProduct());

    QSignalSpy currentProductChangedSpy(controller, &ResultsStatisticsController::currentProductChanged);
    QVERIFY(currentProductChangedSpy.isValid());

    QSignalSpy updateSpy(controller, &ResultsStatisticsController::update);
    QVERIFY(updateSpy.isValid());

    controller->setCurrentProduct(nullptr);
    QCOMPARE(currentProductChangedSpy.count(), 0);
    QCOMPARE(updateSpy.count(), 0);

    auto product = new Product{QUuid::createUuid(), this};
    controller->setCurrentProduct(product);
    QCOMPARE(controller->currentProduct(), product);
    QCOMPARE(currentProductChangedSpy.count(), 1);
    QCOMPARE(updateSpy.count(), 1);

    controller->setCurrentProduct(product);
    QCOMPARE(currentProductChangedSpy.count(), 1);
    QCOMPARE(updateSpy.count(), 1);
}

void TestResultsStatisticsController::testResultsStoragePath()
{
    auto controller = new ResultsStatisticsController{this};
    QCOMPARE(controller->resultsStoragePath(), QLatin1String(""));

    QSignalSpy resultsStoragePathChangedSpy(controller, &ResultsStatisticsController::resultsStoragePathChanged);
    QVERIFY(resultsStoragePathChangedSpy.isValid());

    controller->setResultsStoragePath(QLatin1String(""));
    QCOMPARE(resultsStoragePathChangedSpy.count(), 0);

    controller->setResultsStoragePath(QStringLiteral("PathToResults"));
    QCOMPARE(controller->resultsStoragePath(), QStringLiteral("PathToResults"));
    QCOMPARE(resultsStoragePathChangedSpy.count(), 1);

    controller->setResultsStoragePath(QStringLiteral("PathToResults"));
    QCOMPARE(resultsStoragePathChangedSpy.count(), 1);
}

void TestResultsStatisticsController::testCalculate()
{
    auto controller = new ResultsStatisticsController{this};
    QVERIFY(!controller->calculating());
    QVERIFY(controller->empty());

    QSignalSpy calculatingChangedSpy(controller, &ResultsStatisticsController::calculatingChanged);
    QVERIFY(calculatingChangedSpy.isValid());

    QSignalSpy updateSpy(controller, &ResultsStatisticsController::update);
    QVERIFY(updateSpy.isValid());

    controller->calculate(QDate{}, QDate{});
    QCOMPARE(calculatingChangedSpy.count(), 1);
    QTRY_COMPARE(calculatingChangedSpy.count(), 2);
    QCOMPARE(updateSpy.count(), 1);
    QVERIFY(controller->empty());

    const auto& productJson = QFINDTESTDATA("testdata/statistics/statistics_product.json");

    auto product = Product::fromJson(productJson, this);
    QVERIFY(product);

    controller->setCurrentProduct(product);
    QCOMPARE(updateSpy.count(), 2);

    controller->calculate(QDate{}, QDate{});
    QCOMPARE(calculatingChangedSpy.count(), 3);
    QTRY_COMPARE(calculatingChangedSpy.count(), 4);
    QCOMPARE(updateSpy.count(), 3);
    QVERIFY(controller->empty());

    controller->setResultsStoragePath(QFINDTESTDATA("testdata/statistics"));

    controller->calculate(QDate{}, QDate{});
    QCOMPARE(calculatingChangedSpy.count(), 5);
    QTRY_COMPARE(calculatingChangedSpy.count(), 6);
    QCOMPARE(updateSpy.count(), 4);
    QVERIFY(controller->empty());

    controller->calculate(QDate{2021, 5, 25}, QDate{2021, 6, 1});
    QCOMPARE(calculatingChangedSpy.count(), 7);
    QTRY_COMPARE(calculatingChangedSpy.count(), 8);
    QCOMPARE(updateSpy.count(), 5);
    QVERIFY(!controller->empty());

    const auto& productStats = controller->productStatistics();

    QVERIFY(!productStats.empty());
    QCOMPARE(productStats.instanceCount(), 8);
    QCOMPARE(productStats.ioCount(), 2);
    QCOMPARE(productStats.nioCount(), 6);
    QCOMPARE(productStats.ioInPercent(), 2.0 / 8.0);
    QCOMPARE(productStats.nioInPercent(), 6.0 / 8.0);
    QCOMPARE(productStats.nios().size(), 3);
    QCOMPARE(productStats.nios().at(ResultType::ValueOutOfLimits), 12);
    QCOMPARE(productStats.nios().at(ResultType::RankViolation), 4);
    QCOMPARE(productStats.nios().at(ResultType::NoResultsError), 4);
    QCOMPARE(productStats.seriesStatistics().size(), 2);

    const auto& series1Stats = productStats.seriesStatistics().at(0);

    QCOMPARE(series1Stats.uuid(), product->seamSeries().at(0)->uuid());
    QCOMPARE(series1Stats.name(), product->seamSeries().at(0)->name());
    QCOMPARE(series1Stats.visualNumber(), product->seamSeries().at(0)->visualNumber());
    QVERIFY(!series1Stats.empty());
    QCOMPARE(series1Stats.instanceCount(), 7);
    QCOMPARE(series1Stats.ioCount(), 1);
    QCOMPARE(series1Stats.nioCount(), 6);
    QCOMPARE(series1Stats.ioInPercent(), 1.0 / 7.0);
    QCOMPARE(series1Stats.nioInPercent(), 6.0 / 7.0);
    QCOMPARE(series1Stats.nios().size(), 3);
    QCOMPARE(series1Stats.nios().at(ResultType::ValueOutOfLimits), 10);
    QCOMPARE(series1Stats.nios().at(ResultType::RankViolation), 4);
    QCOMPARE(series1Stats.nios().at(ResultType::NoResultsError), 4);
    QCOMPARE(series1Stats.seamStatistics().size(), 4);

    const auto& seam1Stats = series1Stats.seamStatistics().at(0);
    QCOMPARE(seam1Stats.uuid(), product->seamSeries().at(0)->seams().at(0)->uuid());
    QCOMPARE(seam1Stats.name(), product->seamSeries().at(0)->seams().at(0)->name());
    QCOMPARE(seam1Stats.visualNumber(), product->seamSeries().at(0)->seams().at(0)->visualNumber());
    QVERIFY(seam1Stats.linkTo().isNull());
    QVERIFY(!seam1Stats.empty());
    QCOMPARE(seam1Stats.instanceCount(), 6);
    QCOMPARE(seam1Stats.ioCount(), 1);
    QCOMPARE(seam1Stats.nioCount(), 5);
    QCOMPARE(seam1Stats.ioInPercent(), 1.0 / 6.0);
    QCOMPARE(seam1Stats.nioInPercent(), 5.0 / 6.0);
    QVERIFY(seam1Stats.includesLinkedSeams());
    QCOMPARE(seam1Stats.linkedSeams().size(), 2);
    QCOMPARE(seam1Stats.ioCountIncludingLinkedSeams(), 3);
    QCOMPARE(seam1Stats.nioCountIncludingLinkedSeams(), 5);
    QCOMPARE(seam1Stats.ioCountIncludingLinkedSeamsInPercent(), 3.0 / 8.0);
    QCOMPARE(seam1Stats.nioCountIncludingLinkedSeamsInPercent(), 5.0 / 8.0);
    QCOMPARE(seam1Stats.nios().size(), 2);
    QCOMPARE(seam1Stats.nios().at(ResultType::ValueOutOfLimits), 6);
    QCOMPARE(seam1Stats.nios().at(ResultType::RankViolation), 1);
    QCOMPARE(seam1Stats.linkedNios().size(), 2);
    QCOMPARE(seam1Stats.linkedNios().at(ResultType::ValueOutOfLimits), 6);
    QCOMPARE(seam1Stats.linkedNios().at(ResultType::RankViolation), 1);

    const auto& linkedSeam1Stats = seam1Stats.linkedSeams().at(0);
    QCOMPARE(linkedSeam1Stats.uuid(), product->seamSeries().at(0)->seams().at(4)->uuid());
    QCOMPARE(linkedSeam1Stats.name(), product->seamSeries().at(0)->seams().at(4)->name());
    QCOMPARE(linkedSeam1Stats.visualNumber(), product->seamSeries().at(0)->seams().at(4)->visualNumber());
    QCOMPARE(linkedSeam1Stats.linkTo(), product->seamSeries().at(0)->seams().at(0)->uuid());
    QVERIFY(!linkedSeam1Stats.empty());
    QCOMPARE(linkedSeam1Stats.instanceCount(), 1);
    QCOMPARE(linkedSeam1Stats.ioCount(), 1);
    QCOMPARE(linkedSeam1Stats.nioCount(), 0);
    QCOMPARE(linkedSeam1Stats.ioInPercent(), 1.0 );
    QCOMPARE(linkedSeam1Stats.nioInPercent(), 0.0);
    QVERIFY(!linkedSeam1Stats.includesLinkedSeams());

    const auto& linkedSeam2Stats = seam1Stats.linkedSeams().at(1);
    QCOMPARE(linkedSeam2Stats.uuid(), product->seamSeries().at(0)->seams().at(5)->uuid());
    QCOMPARE(linkedSeam2Stats.name(), product->seamSeries().at(0)->seams().at(5)->name());
    QCOMPARE(linkedSeam2Stats.visualNumber(), product->seamSeries().at(0)->seams().at(5)->visualNumber());
    QCOMPARE(linkedSeam2Stats.linkTo(), product->seamSeries().at(0)->seams().at(0)->uuid());
    QVERIFY(!linkedSeam2Stats.empty());
    QCOMPARE(linkedSeam2Stats.instanceCount(), 1);
    QCOMPARE(linkedSeam2Stats.ioCount(), 1);
    QCOMPARE(linkedSeam2Stats.nioCount(), 0);
    QCOMPARE(linkedSeam2Stats.ioInPercent(), 1.0 );
    QCOMPARE(linkedSeam2Stats.nioInPercent(), 0.0);
    QVERIFY(!linkedSeam2Stats.includesLinkedSeams());

    const auto& seam2Stats = series1Stats.seamStatistics().at(1);
    QCOMPARE(seam2Stats.uuid(), product->seamSeries().at(0)->seams().at(1)->uuid());
    QCOMPARE(seam2Stats.name(), product->seamSeries().at(0)->seams().at(1)->name());
    QCOMPARE(seam2Stats.visualNumber(), product->seamSeries().at(0)->seams().at(1)->visualNumber());
    QVERIFY(seam2Stats.linkTo().isNull());
    QVERIFY(!seam2Stats.empty());
    QCOMPARE(seam2Stats.instanceCount(), 3);
    QCOMPARE(seam2Stats.ioCount(), 0);
    QCOMPARE(seam2Stats.nioCount(), 3);
    QCOMPARE(seam2Stats.ioInPercent(), 0.0);
    QCOMPARE(seam2Stats.nioInPercent(), 1.0);
    QVERIFY(!seam2Stats.includesLinkedSeams());
    QVERIFY(seam2Stats.linkedSeams().empty());
    QCOMPARE(seam2Stats.ioCountIncludingLinkedSeams(), 0);
    QCOMPARE(seam2Stats.nioCountIncludingLinkedSeams(), 3);
    QCOMPARE(seam2Stats.ioCountIncludingLinkedSeamsInPercent(), 0.0);
    QCOMPARE(seam2Stats.nioCountIncludingLinkedSeamsInPercent(), 1.0);
    QCOMPARE(seam2Stats.nios().size(), 1);
    QCOMPARE(seam2Stats.nios().at(ResultType::RankViolation), 3);
    // linked seam nio map remains empty, until a linked seam is added
    QCOMPARE(seam2Stats.linkedNios().size(), 0);

    const auto& seam3Stats = series1Stats.seamStatistics().at(2);
    QCOMPARE(seam3Stats.uuid(), product->seamSeries().at(0)->seams().at(2)->uuid());
    QCOMPARE(seam3Stats.name(), product->seamSeries().at(0)->seams().at(2)->name());
    QCOMPARE(seam3Stats.visualNumber(), product->seamSeries().at(0)->seams().at(2)->visualNumber());
    QVERIFY(seam3Stats.linkTo().isNull());
    QVERIFY(!seam3Stats.empty());
    QCOMPARE(seam3Stats.instanceCount(), 4);
    QCOMPARE(seam3Stats.ioCount(), 0);
    QCOMPARE(seam3Stats.nioCount(), 4);
    QCOMPARE(seam3Stats.ioInPercent(), 0.0);
    QCOMPARE(seam3Stats.nioInPercent(), 1.0);
    QVERIFY(!seam3Stats.includesLinkedSeams());
    QVERIFY(seam3Stats.linkedSeams().empty());
    QCOMPARE(seam3Stats.ioCountIncludingLinkedSeams(), 0);
    QCOMPARE(seam3Stats.nioCountIncludingLinkedSeams(), 4);
    QCOMPARE(seam3Stats.ioCountIncludingLinkedSeamsInPercent(), 0.0);
    QCOMPARE(seam3Stats.nioCountIncludingLinkedSeamsInPercent(), 1.0);
    QCOMPARE(seam3Stats.nios().size(), 1);
    QCOMPARE(seam3Stats.nios().at(ResultType::ValueOutOfLimits), 4);
    // linked seam nio map remains empty, until a linked seam is added
    QCOMPARE(seam3Stats.linkedNios().size(), 0);

    const auto& seam4Stats = series1Stats.seamStatistics().at(3);
    QCOMPARE(seam4Stats.uuid(), product->seamSeries().at(0)->seams().at(3)->uuid());
    QCOMPARE(seam4Stats.name(), product->seamSeries().at(0)->seams().at(3)->name());
    QCOMPARE(seam4Stats.visualNumber(), product->seamSeries().at(0)->seams().at(3)->visualNumber());
    QVERIFY(seam4Stats.linkTo().isNull());
    QVERIFY(!seam4Stats.empty());
    QCOMPARE(seam4Stats.instanceCount(), 4);
    QCOMPARE(seam4Stats.ioCount(), 0);
    QCOMPARE(seam4Stats.nioCount(), 4);
    QCOMPARE(seam4Stats.ioInPercent(), 0.0);
    QCOMPARE(seam4Stats.nioInPercent(), 1.0);
    QVERIFY(!seam4Stats.includesLinkedSeams());
    QVERIFY(seam4Stats.linkedSeams().empty());
    QCOMPARE(seam4Stats.ioCountIncludingLinkedSeams(), 0);
    QCOMPARE(seam4Stats.nioCountIncludingLinkedSeams(), 4);
    QCOMPARE(seam4Stats.ioCountIncludingLinkedSeamsInPercent(), 0.0);
    QCOMPARE(seam4Stats.nioCountIncludingLinkedSeamsInPercent(), 1.0);
    QCOMPARE(seam4Stats.nios().size(), 1);
    QCOMPARE(seam4Stats.nios().at(ResultType::NoResultsError), 4);
    // linked seam nio map remains empty, until a linked seam is added
    QCOMPARE(seam4Stats.linkedNios().size(), 0);

    QCOMPARE(series1Stats.nios().at(ResultType::ValueOutOfLimits), seam1Stats.nios().at(ResultType::ValueOutOfLimits) + seam3Stats.nios().at(ResultType::ValueOutOfLimits));

    QCOMPARE(series1Stats.nios().at(ResultType::RankViolation), seam1Stats.nios().at(ResultType::RankViolation) + seam2Stats.nios().at(ResultType::RankViolation));

    QCOMPARE(series1Stats.nios().at(ResultType::NoResultsError), seam4Stats.nios().at(ResultType::NoResultsError));

    const auto& series2Stats = productStats.seriesStatistics().at(1);

    QCOMPARE(series2Stats.uuid(), product->seamSeries().at(1)->uuid());
    QCOMPARE(series2Stats.name(), product->seamSeries().at(1)->name());
    QCOMPARE(series2Stats.visualNumber(), product->seamSeries().at(1)->visualNumber());
    QVERIFY(!series2Stats.empty());
    QCOMPARE(series2Stats.instanceCount(), 3);
    QCOMPARE(series2Stats.ioCount(), 1);
    QCOMPARE(series2Stats.nioCount(), 2);
    QCOMPARE(series2Stats.ioInPercent(), 1.0 / 3.0);
    QCOMPARE(series2Stats.nioInPercent(), 2.0 / 3.0);
    QCOMPARE(series2Stats.nios().size(), 1);
    QCOMPARE(series2Stats.nios().at(ResultType::ValueOutOfLimits), 2);
    QCOMPARE(series2Stats.seamStatistics().size(), 1);

    const auto& seam5Stats = series2Stats.seamStatistics().at(0);
    QCOMPARE(seam5Stats.uuid(), product->seamSeries().at(1)->seams().at(0)->uuid());
    QCOMPARE(seam5Stats.name(), product->seamSeries().at(1)->seams().at(0)->name());
    QCOMPARE(seam5Stats.visualNumber(), product->seamSeries().at(1)->seams().at(0)->visualNumber());
    QVERIFY(seam5Stats.linkTo().isNull());
    QVERIFY(!seam5Stats.empty());
    QCOMPARE(seam5Stats.instanceCount(), 3);
    QCOMPARE(seam5Stats.ioCount(), 1);
    QCOMPARE(seam5Stats.nioCount(), 2);
    QCOMPARE(seam5Stats.ioInPercent(), 1.0 / 3.0);
    QCOMPARE(seam5Stats.nioInPercent(), 2.0 / 3.0);
    QVERIFY(!seam5Stats.includesLinkedSeams());
    QVERIFY(seam5Stats.linkedSeams().empty());
    QCOMPARE(seam5Stats.ioCountIncludingLinkedSeams(), 1);
    QCOMPARE(seam5Stats.nioCountIncludingLinkedSeams(), 2);
    QCOMPARE(seam5Stats.ioCountIncludingLinkedSeamsInPercent(), 1.0 / 3.0);
    QCOMPARE(seam5Stats.nioCountIncludingLinkedSeamsInPercent(), 2.0 / 3.0);
    QCOMPARE(seam5Stats.nios().size(), 1);
    QCOMPARE(seam5Stats.nios().at(ResultType::ValueOutOfLimits), 2);
    // linked seam nio map remains empty, until a linked seam is added
    QCOMPARE(seam5Stats.linkedNios().size(), 0);

    QCOMPARE(series2Stats.nios().at(ResultType::ValueOutOfLimits), seam5Stats.nios().at(ResultType::ValueOutOfLimits));

    QCOMPARE(productStats.nios().at(ResultType::ValueOutOfLimits), series1Stats.nios().at(ResultType::ValueOutOfLimits) + series2Stats.nios().at(ResultType::ValueOutOfLimits));
    QCOMPARE(productStats.nios().at(ResultType::RankViolation), series1Stats.nios().at(ResultType::RankViolation));
    QCOMPARE(productStats.nios().at(ResultType::NoResultsError), series1Stats.nios().at(ResultType::NoResultsError));

    auto emptyProduct = new Product{QUuid::createUuid(), this};

    controller->setCurrentProduct(emptyProduct);
    QCOMPARE(updateSpy.count(), 6);
    QVERIFY(controller->empty());
}

QTEST_GUILESS_MAIN(TestResultsStatisticsController)
#include "testResultsStatisticsController.moc"
