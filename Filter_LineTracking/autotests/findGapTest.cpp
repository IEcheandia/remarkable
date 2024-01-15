#include <QTest>

#include "../findGap.h"
#include <fliplib/NullSourceFilter.h>

#include <fliplib/BaseFilter.h>
#include <overlay/overlayCanvas.h>


class FindGapTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testCtor();
    void testProceed();
    void testProceed_data();
};

struct DummyInput
{
    fliplib::NullSourceFilter m_sourceFilter;
    fliplib::SynchronePipe<precitec::interface::GeoVecDoublearray> m_pipeIn_line;

    precitec::interface::GeoVecDoublearray m_line;

    DummyInput()
    : m_pipeIn_line(&m_sourceFilter, "LineIn")
    {
    }

    void connectToFilter(fliplib::BaseFilter* pFilter)
    {
        int group = 0;
        m_pipeIn_line.setTag("line");
        QVERIFY(pFilter->connectPipe(&(m_pipeIn_line), group));
    }

    void fillData(const int imageNumber, const std::vector<std::vector<double>>& lineIn, const std::vector<std::vector<double>>& rank, const precitec::geo2d::Point& trafoOffset, int badRankBorder=0)
    {
        precitec::interface::SmpTrafo oTrafo{new precitec::interface::LinearTrafo(trafoOffset)};
        precitec::interface::ImageContext context (precitec::interface::ImageContext{}, oTrafo);
        context.setImageNumber(imageNumber);

        // convert in-pipe lines into appropriate data structure
        precitec::geo2d::VecDoublearray line(lineIn.size());
        for (uint i = 0; i < lineIn.size(); ++i)
        {
            int lineLength = lineIn[i].size();
            line[i].getData().insert(line[i].getData().begin(), lineIn[i].begin(), lineIn[i].end());
            line[i].getRank().insert(line[i].getRank().begin(), rank[i].begin(), rank[i].end());
            if (2*badRankBorder < lineLength)
            {
                for (int x = 0; x < badRankBorder; x++)
                {
                    line[i].getRank()[x] = 0;
                }
                for (int x = lineLength-1; x >= lineLength - badRankBorder; x--)
                {
                    line[i].getRank()[x] = 0;
                }
            }
        }

        m_line = precitec::interface::GeoVecDoublearray(context, line, precitec::interface::AnalysisOK, precitec::interface::Perfect);
    }

    void signal()
    {
        m_pipeIn_line.signal(m_line);
    }
};

class DummyFilter : public fliplib::BaseFilter
{
public:
    DummyFilter() : fliplib::BaseFilter("dummy") {}
    void proceed(const void *sender, fliplib::PipeEventArgs &e) override
    {
        Q_UNUSED(sender)
        Q_UNUSED(e)
        preSignalAction();
        m_proceedCalled = true;
    }

    void proceedGroup ( const void * sender, fliplib::PipeGroupEventArgs & e ) override
    {
        Q_UNUSED(sender)
        Q_UNUSED(e)
        preSignalAction();
        m_proceedCalled = true;
    }

    int getFilterType() const override
    {
        return BaseFilterInterface::SINK;
    }

    bool isProceedCalled() const
    {
        return m_proceedCalled;
    }

    void resetProceedCalled()
    {
        m_proceedCalled = false;
    }


private:
    bool m_proceedCalled = false;
};

void FindGapTest::testCtor()
{
    precitec::filter::FindGap filter;

    QCOMPARE(filter.name(), std::string("FindGap"));
    QVERIFY(filter.findPipe("posX"));
    QVERIFY(filter.findPipe("posY"));
}

void FindGapTest::testProceed_data()
{
    // in-pipe line
    QTest::addColumn<std::vector<std::vector<double>>>("lineIn");
    QTest::addColumn<std::vector<std::vector<double>>>("rank");
    QTest::addColumn<int>("badRankBorder");
    // parameters
    QTest::addColumn<int>("searchDirection");
    QTest::addColumn<int>("maxJumpDownY");
    QTest::addColumn<int>("maxJumpUpY");
    QTest::addColumn<int>("maxJumpX");
    QTest::addColumn<int>("gapWidth");
    QTest::addColumn<int>("numberValuesForAveraging");
    // expected pos
    QTest::addColumn<std::vector<double>>("expectedX");
    QTest::addColumn<std::vector<double>>("expectedY");

    int size = 20;
    std::vector<double> continuousLine;
    std::vector<double> goodRank;
    std::vector<double> badRank;
    for (int i = 0; i < size; ++i)
    {
        continuousLine.push_back(20 - i / 2);
        goodRank.push_back(precitec::filter::eRankMax);
        badRank.push_back(precitec::filter::eRankMin);
    }
    std::vector<double> oneSmallGap = continuousLine;
    oneSmallGap[10] = 25;
    std::vector<double> oneGap = oneSmallGap;
    oneGap[11] = 26;
    oneGap[12] = 28;
    std::vector<double> longLineWithOneGap;
    std::vector<double> longGoodRank;
    size = 30;
    for (int i = 0; i < size; ++i)
    {
        longLineWithOneGap.push_back(20 - i / 2);
        longGoodRank.push_back(precitec::filter::eRankMax);
    }
    longLineWithOneGap[25] = 18;
    longLineWithOneGap[26] = 19;
    longLineWithOneGap[27] = 20;
    std::vector<double> longRankMixed = longGoodRank;
    longRankMixed.at(0) = precitec::filter::eRankMin;
    for (int i = 4; i < 8; ++i)
    {
        longRankMixed.at(i) = precitec::filter::eRankMin;
    }
    longRankMixed.at(26) = precitec::filter::eRankMin;
    longRankMixed.at(29) = precitec::filter::eRankMin;


    QTest::newRow("No line") // Displayed test name
            << std::vector<std::vector<double>>  {}
            << std::vector<std::vector<double>>  {}
            << 0
            << 0 << 10 << 20 << 10 << 5 << 3
            << std::vector<double>               {}
            << std::vector<double>               {};
    QTest::newRow("Empty line") // Displayed test name
            << std::vector<std::vector<double>>  {{}}
            << std::vector<std::vector<double>>  {{}}
            << 0
            << 0 << 10 << 20 << 10 << 5 << 3
            << std::vector<double>               {0}
            << std::vector<double>               {0};
    QTest::newRow("One continuous line") // Displayed test name
            << std::vector<std::vector<double>>  {continuousLine}
            << std::vector<std::vector<double>>  {goodRank}
            << 0
            << 0 << 10 << 5 << 10 << 2 << 3
            << std::vector<double>               {19}
            << std::vector<double>               {11};
    QTest::newRow("One continuous line with bad rank padding") // Displayed test name
            << std::vector<std::vector<double>>  {continuousLine}
            << std::vector<std::vector<double>>  {goodRank}
            << 5
            << 0 << 10 << 5 << 10 << 2 << 3
            << std::vector<double>               {14}
            << std::vector<double>               {13};
    QTest::newRow("One continuous line with single bad rank from left") // Displayed test name
            << std::vector<std::vector<double>>  {continuousLine}
            << std::vector<std::vector<double>>  {goodRank}
            << 1
            << 0 << 10 << 5 << 10 << 2 << 3
            << std::vector<double>               {18}
            << std::vector<double>               {11};
    QTest::newRow("One continuous line with single bad rank from right") // Displayed test name
            << std::vector<std::vector<double>>  {continuousLine}
            << std::vector<std::vector<double>>  {goodRank}
            << 1
            << 1 << 10 << 5 << 10 << 2 << 3
            << std::vector<double>               {1}
            << std::vector<double>               {20};
    QTest::newRow("Small gap, empty line, continuous line (from right)") // Displayed test name
            << std::vector<std::vector<double>>  {oneSmallGap, {}, continuousLine}
            << std::vector<std::vector<double>>  {goodRank, {}, goodRank}
            << 0
            << 1 << 5 << 20 << 10 << 2 << 3
            << std::vector<double>               { 0, 0,  0}
            << std::vector<double>               {20, 0, 20};
    QTest::newRow("Empty line, gap, no gap, too small gap") // Displayed test name
            << std::vector<std::vector<double>>  {{}, oneGap, continuousLine, oneSmallGap}
            << std::vector<std::vector<double>>  {{}, goodRank, goodRank, goodRank}
            << 0
            << 0 << 5 << 20 << 1 << 2 << 3
            << std::vector<double>               {0, 19, 19, 19}
            << std::vector<double>               {0, 11, 11, 11};
    QTest::newRow("Empty line, gap, no gap, gap") // Displayed test name
            << std::vector<std::vector<double>>  {{}, oneGap, continuousLine, oneSmallGap}
            << std::vector<std::vector<double>>  {{}, goodRank, goodRank, goodRank}
            << 0
            << 0 << 5 << 20 << 10 << 2 << 1
            << std::vector<double>               {0,  9, 19, 19}
            << std::vector<double>               {0, 16, 11, 11};
    QTest::newRow("Empty line, gap, no gap, gap with bad rank padding") // Displayed test name
            << std::vector<std::vector<double>>  {{}, oneGap, continuousLine, oneSmallGap}
            << std::vector<std::vector<double>>  {{}, goodRank, goodRank, goodRank}
            << 5
            << 0 << 5 << 20 << 10 << 2 << 1
            << std::vector<double>               {0,  9, 14, 14}
            << std::vector<double>               {0, 16, 13, 13};
    QTest::newRow("Long Line with one gap") // Displayed test name
            << std::vector<std::vector<double>>  {longLineWithOneGap}
            << std::vector<std::vector<double>>  {longGoodRank}
            << 0
            << 0 << 5 << 20 << 10 << 2 << 1
            << std::vector<double>               {24}
            << std::vector<double>               {8};
    QTest::newRow("Gap hidden by bad rank padding") // Displayed test name
            << std::vector<std::vector<double>>  {longLineWithOneGap}
            << std::vector<std::vector<double>>  {longGoodRank}
            << 9
            << 0 << 5 << 20 << 10 << 2 << 1
            << std::vector<double>               {30-9-1}
            << std::vector<double>               {10};
    QTest::newRow("Long Line with one gap, mixed rank") // Displayed test name
            << std::vector<std::vector<double>>  {longLineWithOneGap}
            << std::vector<std::vector<double>>  {longRankMixed}
            << 0
            << 0 << 5 << 20 << 10 << 2 << 1
            << std::vector<double>               {3}
            << std::vector<double>               {19};
    QTest::newRow("Long Line with one gap, shorter continuousLine") // Displayed test name
            << std::vector<std::vector<double>>  {longLineWithOneGap, continuousLine}
            << std::vector<std::vector<double>>  {longGoodRank, goodRank}
            << 0
            << 0 << 5 << 20 << 10 << 2 << 1
            << std::vector<double>               {24, 19}
            << std::vector<double>               {8, 11};
    QTest::newRow("two lines with bad rank, one empty") // Displayed test name
            << std::vector<std::vector<double>>  {oneGap, continuousLine, {}}
            << std::vector<std::vector<double>>  {badRank, badRank, {}}
            // badRankBorder, searchDirection, maxJumpDownY, maxJumpUpY, maxJumpX, gapWidth, numberValuesForAveraging
            << 0                << 0            << 5            << 20   << 10       << 2    << 1
            << std::vector<double>               {19, 19, 0}
            << std::vector<double>               {11, 11, 0};
}

void FindGapTest::testProceed()
{
    precitec::filter::FindGap filter;

    std::unique_ptr<precitec::image::OverlayCanvas> pCanvas {new precitec::image::OverlayCanvas};
    filter.setCanvas(pCanvas.get());

    // connect in-pipes
    DummyInput dummyInput;
    dummyInput.connectToFilter(&filter);

    // create out-pipes
    auto outPipeX = dynamic_cast<fliplib::SynchronePipe<precitec::interface::GeoDoublearray>*>(filter.findPipe("posX"));
    QVERIFY(outPipeX);
    DummyFilter filterOutData;
    QVERIFY(filterOutData.connectPipe(outPipeX, 1));

    auto outPipeY = dynamic_cast<fliplib::SynchronePipe<precitec::interface::GeoDoublearray>*>(filter.findPipe("posY"));
    QVERIFY(outPipeY);
    QVERIFY(filterOutData.connectPipe(outPipeY, 1));

    // update parameters
    QFETCH(int, searchDirection);
    QFETCH(int, maxJumpDownY);
    QFETCH(int, maxJumpUpY);
    QFETCH(int, gapWidth);
    QFETCH(int, numberValuesForAveraging);
    filter.getParameters().update(std::string("SearchDirection"), fliplib::Parameter::TYPE_int, searchDirection );
    filter.getParameters().update(std::string("MaxJumpDownY"), fliplib::Parameter::TYPE_int, maxJumpDownY );
    filter.getParameters().update(std::string("MaxJumpUpY"), fliplib::Parameter::TYPE_int, maxJumpUpY );
    filter.getParameters().update(std::string("GapWidth"), fliplib::Parameter::TYPE_int, gapWidth );
    filter.getParameters().update(std::string("NumberValuesForAveraging"), fliplib::Parameter::TYPE_int, numberValuesForAveraging );
    filter.setParameter();

    // dummy data
    int imageNumber  =   0;
    int position     = 300;
    precitec::geo2d::Point trafoOffset {510,100};

    precitec::interface::ImageContext context;
    context.setImageNumber(imageNumber);
    context.setPosition(position);

    // parse test data
    QFETCH(std::vector<std::vector<double>>, lineIn);
    QFETCH(std::vector<std::vector<double>>, rank);
    QFETCH(int, badRankBorder);
    dummyInput.fillData(imageNumber, lineIn, rank, trafoOffset, badRankBorder);

    // verify that the filter has run and signal pipes
    QCOMPARE(filterOutData.isProceedCalled(), false);
    dummyInput.signal();
    QCOMPARE(filterOutData.isProceedCalled(), true);
    filterOutData.resetProceedCalled();

    // compare signaled data
    QFETCH(std::vector<double>, expectedX);
    QFETCH(std::vector<double>, expectedY);
    const auto resultX = outPipeX->read(imageNumber);
    const auto resultY = outPipeY->read(imageNumber);
    QCOMPARE(resultX.ref().size(), expectedX.size());
    QCOMPARE(resultY.ref().size(), expectedY.size());
    for (auto i = 0u; i < expectedX.size(); ++i)
    {
        QCOMPARE(resultX.ref().getData().at(i), expectedX.at(i));
        QCOMPARE(resultY.ref().getData().at(i), expectedY.at(i));
    }
}

QTEST_GUILESS_MAIN(FindGapTest)

#include "findGapTest.moc"


