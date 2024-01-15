#include <QTest>

#include "../lineSelectLocalExtremum.h"
#include <fliplib/NullSourceFilter.h>

#include <fliplib/BaseFilter.h>
#include <overlay/overlayCanvas.h>


class LineSelectLocalExtremumTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testCtor();
    void testPeak();
    void testPeak_data();
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
        int group = 1;
        m_pipeIn_line.setTag("Line");
        QVERIFY(pFilter->connectPipe(&(m_pipeIn_line), group));
    }

    void fillData(const int imageNumber, const std::vector<std::vector<double>>& lineIn, const precitec::geo2d::Point& trafoOffset)
    {
        precitec::interface::SmpTrafo oTrafo{new precitec::interface::LinearTrafo(trafoOffset)};
        precitec::interface::ImageContext context (precitec::interface::ImageContext{}, oTrafo);
        context.setImageNumber(imageNumber);

        // convert in-pipe lines into appropriate data structure
        precitec::geo2d::VecDoublearray line(lineIn.size());
        for (uint i = 0; i < lineIn.size(); ++i)
        {
            line[i].getData().insert(line[i].getData().begin(), lineIn[i].begin(), lineIn[i].end());
            line[i].getRank().assign(line[i].getData().size(), precitec::filter::eRankMax);
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


void LineSelectLocalExtremumTest::testCtor()
{
    precitec::filter::LineSelectLocalExtremum filter;

    QCOMPARE(filter.name(), std::string("LineSelectLocalExtremum"));
    QVERIFY(filter.findPipe("PositionX"));
    QVERIFY(filter.findPipe("PositionY"));
    QCOMPARE(filter.outPipeConnectors()[2].name() , "StartPeakX");
    QCOMPARE(filter.outPipeConnectors()[3].name() , "EndPeakX");

}

using precitec::geo2d::DPoint;

Q_DECLARE_METATYPE(DPoint);

static const std::vector<double> monotonicallyNondecreasing = []()
    {
        std::vector<double> monotonicallyNondecreasing;
        for (int i = 0; i < 30; ++i)
        {
            monotonicallyNondecreasing.push_back(30-i);
        }
        return monotonicallyNondecreasing;
    }();


static const DPoint peak1 {7.0, monotonicallyNondecreasing[7] + 5};
std::vector<double> oneMaximum = []()
    {
        std::vector<double> oneMaximum = monotonicallyNondecreasing;
        for (int x = peak1.x - 1; x <= peak1.x +1; x++)
        {
            oneMaximum[x] = peak1.y - 1.0;
        }
        oneMaximum[peak1.x] = peak1.y;
        return oneMaximum;
    }();

static const std::vector<double> oneOutlier = []()
    {
        std::vector<double> oneOutlier = monotonicallyNondecreasing;
        oneOutlier[10] = 15.0;
        return oneOutlier;
    }();

static const DPoint peak2 {15.0, 50.0};
static const std::vector<double> twoMaxima = []()
    {
        std::vector<double> twoMaxima = oneMaximum;
        for (int x = peak2.x - 2; x <= peak2.x +2; x++)
        {
            twoMaxima[x] = peak2.y - 1.0;
        }
        //3 neighboring points with the same value
        for (int x = peak2.x - 1; x <= peak2.x +1; x++)
        {
            twoMaxima[x] = peak2.y;
        }
        return twoMaxima;
    }();

static const std::vector<double> monotonicallyDecreasing = []()
    {
        std::vector<double> monotonicallyDecreasing;
        for (int i = 0; i < 30; ++i)
        {
            monotonicallyDecreasing.push_back(i);
        }
        return monotonicallyDecreasing;
    }();


void LineSelectLocalExtremumTest::testPeak_data()
{
    QTest::addColumn< std::vector<std::vector<double>> > ("lineIn");
    QTest::addColumn< int > ("extremumNumber");        // in-pipe Line data
    QTest::addColumn< std::vector<DPoint> > ("expectedOutputPoint");
    QTest::addColumn< std::vector<int> > ("expectedOutputRank");


    QTest::newRow("monotonicallyNondecreasing_1")   // Displayed test name
            << std::vector<std::vector<double>>  { monotonicallyNondecreasing }
            << 1
            << std::vector<DPoint>               { {0,0} }
            << std::vector<int>                  { 0 };
    QTest::newRow("monotonicallyNondecreasing_2")   // Displayed test name
            << std::vector<std::vector<double>>  { monotonicallyNondecreasing }
            << 2
            << std::vector<DPoint>               { {0,0} }
            << std::vector<int>                  { 0 };

    QTest::newRow("MonotonicallyDecreasing_1")   // Displayed test name
            << std::vector<std::vector<double>>  { monotonicallyDecreasing }
            << 1
            << std::vector<DPoint>               { {0,0} }
            << std::vector<int>                  { 0 };

    QTest::newRow("MonotonicallyDecreasing_2")   // Displayed test name
            << std::vector<std::vector<double>>  { monotonicallyDecreasing }
            << 2
            << std::vector<DPoint>               { {0,0} }
            << std::vector<int>                  { 0 };

    QTest::newRow("oneMaximum_1")   // Displayed test name
            << std::vector<std::vector<double>>  { oneMaximum }
            << 1
            << std::vector<DPoint>               { peak1 }
            << std::vector<int>                  { 255 };

    QTest::newRow("oneMaximum_2")   // Displayed test name
            << std::vector<std::vector<double>>  { oneMaximum }
            << 2
            << std::vector<DPoint>               { {0,0} }
            << std::vector<int>                  { 0 };

    QTest::newRow("oneMaximum_20")   // Displayed test name
            << std::vector<std::vector<double>>  { oneMaximum }
            << 20
            << std::vector<DPoint>               { {0,0} }
            << std::vector<int>                  { 0 };

    QTest::newRow("twoMaxima_1")   // Displayed test name
            << std::vector<std::vector<double>>  { twoMaxima }
            << 1
            << std::vector<DPoint>               { peak2}
            << std::vector<int>                  { 255 };

    QTest::newRow("twoMaxima_2")   // Displayed test name
            << std::vector<std::vector<double>>  { twoMaxima }
            << 2
            << std::vector<DPoint>               { peak1}
            << std::vector<int>                  { 255 };

    QTest::newRow("twoMaxima_20")   // Displayed test name
            << std::vector<std::vector<double>>  { twoMaxima }
            << 20
            << std::vector<DPoint>               { {0,0} }
            << std::vector<int>                  { 0 };


    QTest::newRow("oneOutlier")   // Displayed test name
            << std::vector<std::vector<double>>  { oneOutlier }
            << 1
            << std::vector<DPoint>               { {0,0}}
            << std::vector<int>                  { 0 };
}

void LineSelectLocalExtremumTest::testPeak()
{
    precitec::filter::LineSelectLocalExtremum filter;

    std::unique_ptr<precitec::image::OverlayCanvas> pCanvas {new precitec::image::OverlayCanvas};
    filter.setCanvas(pCanvas.get());

    // connect in-pipes
    DummyInput dummyInput;
    dummyInput.connectToFilter(&filter);

    // create out-pipes
    auto outPipeX = dynamic_cast<fliplib::SynchronePipe<precitec::interface::GeoDoublearray>*>(filter.findPipe("PositionX"));
    QVERIFY(outPipeX);
    DummyFilter filterOutData;
    QVERIFY(filterOutData.connectPipe(outPipeX, 1));

    auto outPipeY = dynamic_cast<fliplib::SynchronePipe<precitec::interface::GeoDoublearray>*>(filter.findPipe("PositionY"));
    QVERIFY(outPipeY);
    QVERIFY(filterOutData.connectPipe(outPipeY, 1));

    auto outPipeStart = dynamic_cast<fliplib::SynchronePipe<precitec::interface::GeoDoublearray>*>(filter.findPipe("StartPeakX"));
    QVERIFY(outPipeStart);
    QVERIFY(filterOutData.connectPipe(outPipeStart, 1));


    auto outPipeEnd = dynamic_cast<fliplib::SynchronePipe<precitec::interface::GeoDoublearray>*>(filter.findPipe("EndPeakX"));
    QVERIFY(outPipeEnd);
    QVERIFY(filterOutData.connectPipe(outPipeEnd, 1));


    // update parameters
    QFETCH(int, extremumNumber);
    int extremumType = precitec::filter::ExtremumType::eMaximum;
    int extremumDistance = 1;
    double extremumDifference = 0.;
    filter.getParameters().update(std::string("ExtremumType"), fliplib::Parameter::TYPE_int, extremumType );
    filter.getParameters().update(std::string("ExtremumNumber"), fliplib::Parameter::TYPE_int, extremumNumber );
    filter.getParameters().update(std::string("ExtremumDistance"), fliplib::Parameter::TYPE_int, extremumDistance );
    filter.getParameters().update(std::string("ExtremumDifference"), fliplib::Parameter::TYPE_double, extremumDifference );

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
    dummyInput.fillData(imageNumber, lineIn, trafoOffset);

    // expected maximum value
    QFETCH(std::vector<DPoint>, expectedOutputPoint);
    QFETCH(std::vector<int>, expectedOutputRank);
    QVERIFY2(expectedOutputPoint.size() == expectedOutputRank.size(), "Test Data inconsistent");



    // verify that the filter has run and signal pipes
    QCOMPARE(filterOutData.isProceedCalled(), false);
    dummyInput.signal();
    QCOMPARE(filterOutData.isProceedCalled(), true);
    filterOutData.resetProceedCalled();

    // compare signaled data
    {
        const auto resultX = outPipeX->read(imageNumber);
        const auto resultY = outPipeY->read(imageNumber);
        const auto resultStart = outPipeStart->read(imageNumber);
        const auto resultEnd = outPipeEnd->read(imageNumber);
        QCOMPARE(resultX.ref().size(), expectedOutputPoint.size());
        QCOMPARE(resultY.ref().size(), expectedOutputPoint.size());
        QCOMPARE(resultStart.ref().size(), expectedOutputPoint.size());
        QCOMPARE(resultEnd.ref().size(), expectedOutputPoint.size());


        for (uint i = 0; i < expectedOutputPoint.size(); ++i)
        {
            auto expectedRank = expectedOutputRank[i];
            QCOMPARE(resultX.ref().getData()[i], expectedOutputPoint[i].x);
            QCOMPARE(resultY.ref().getData()[i], expectedOutputPoint[i].y);
            QCOMPARE(resultX.ref().getRank()[i], expectedRank);
            QCOMPARE(resultY.ref().getRank()[i], expectedRank);
            if (expectedRank > 0)
            {
                QVERIFY(resultStart.ref().getData()[i] <= expectedOutputPoint[i].x);
                QVERIFY(resultEnd.ref().getData()[i] >= expectedOutputPoint[i].x);
            }
        }
    }




    // Flip the signal to test the minimum value

    filter.getParameters().update(std::string("ExtremumType"), fliplib::Parameter::TYPE_int, (int)(precitec::filter::ExtremumType::eMinimum) );
    filter.setParameter();

    std::vector<std::vector<double>> lineInFlipped;
    for (auto & line : lineIn)
    {
        lineInFlipped.push_back({});
        std::transform(line.begin(), line.end(), std::back_inserter(lineInFlipped.back()), [](auto v){return -v;});
    }
    std::vector<DPoint> expectedOutputPointMinimum;
    std::transform(expectedOutputPoint.begin(), expectedOutputPoint.end(), std::back_inserter(expectedOutputPointMinimum),
                   [](auto point){return DPoint{point.x, -point.y};});

    dummyInput.fillData(imageNumber, lineInFlipped, trafoOffset);

    // verify that the filter has run and signal pipes
    QCOMPARE(filterOutData.isProceedCalled(), false);
    dummyInput.signal();
    QCOMPARE(filterOutData.isProceedCalled(), true);
    filterOutData.resetProceedCalled();

    // compare signaled data
    {
        const auto resultX = outPipeX->read(imageNumber);
        const auto resultY = outPipeY->read(imageNumber);
        const auto resultStart = outPipeStart->read(imageNumber);
        const auto resultEnd = outPipeEnd->read(imageNumber);
        QCOMPARE(resultX.ref().size(), expectedOutputPoint.size());
        QCOMPARE(resultY.ref().size(), expectedOutputPoint.size());
        QCOMPARE(resultStart.ref().size(), expectedOutputPoint.size());
        QCOMPARE(resultEnd.ref().size(), expectedOutputPoint.size());

        for (uint i = 0; i < expectedOutputPointMinimum.size(); ++i)
        {
            auto expectedRank = expectedOutputRank[i];
            QCOMPARE(resultX.ref().getData()[i], expectedOutputPointMinimum[i].x);
            QCOMPARE(resultY.ref().getData()[i], expectedOutputPointMinimum[i].y);
            QCOMPARE(resultX.ref().getRank()[i], expectedRank);
            QCOMPARE(resultY.ref().getRank()[i], expectedRank);
            if (expectedRank > 0)
            {
                QVERIFY(resultStart.ref().getData()[i] <= expectedOutputPoint[i].x);
                QVERIFY(resultEnd.ref().getData()[i] >= expectedOutputPoint[i].x);
            }
        }
    }
}

void LineSelectLocalExtremumTest::testProceed_data()
{
    QTest::addColumn< std::vector<std::vector<double>> > ("lineIn");        // in-pipe Line data
    QTest::addColumn< std::vector<DPoint> > ("expectedOutputPoint");
    QTest::addColumn< std::vector<int> > ("expectedOutputRank");


    QTest::newRow("No line")   // Displayed test name
            << std::vector<std::vector<double>>  {  }
            << std::vector<DPoint>               {}
            << std::vector<int>                  {};
    QTest::newRow("Empty line")   // Displayed test name
            << std::vector<std::vector<double>>  { {} }
            << std::vector<DPoint>               { {0.0,0.0} }
            << std::vector<int>                  { 0 };
    QTest::newRow("OneLine")   // Displayed test name
            << std::vector<std::vector<double>>  { monotonicallyNondecreasing }
            << std::vector<DPoint>               { {0,0} }
            << std::vector<int>                  { 0 };
    QTest::newRow("Input size differ, first big")   // Displayed test name
            << std::vector<std::vector<double>>  { oneMaximum, {}, twoMaxima}
            << std::vector<DPoint>               { peak1, {0.0, 0.0}, peak2 }
            << std::vector<int>                  { 255, 0, 255};
    QTest::newRow("Input size differ, first zero")   // Displayed test name
            << std::vector<std::vector<double>>  { {}, monotonicallyDecreasing, twoMaxima, oneMaximum}
            << std::vector<DPoint>               { {0,0}, {0,0}, peak2, peak1 }
            << std::vector<int>                  { 0, 0,  255, 255 };
}


void LineSelectLocalExtremumTest::testProceed()
{
    precitec::filter::LineSelectLocalExtremum filter;

    std::unique_ptr<precitec::image::OverlayCanvas> pCanvas {new precitec::image::OverlayCanvas};
    filter.setCanvas(pCanvas.get());

    // connect in-pipes
    DummyInput dummyInput;
    dummyInput.connectToFilter(&filter);

    // create out-pipes
    auto outPipeX = dynamic_cast<fliplib::SynchronePipe<precitec::interface::GeoDoublearray>*>(filter.findPipe("PositionX"));
    QVERIFY(outPipeX);
    DummyFilter filterOutData;
    QVERIFY(filterOutData.connectPipe(outPipeX, 1));

    auto outPipeY = dynamic_cast<fliplib::SynchronePipe<precitec::interface::GeoDoublearray>*>(filter.findPipe("PositionY"));
    QVERIFY(outPipeY);
    QVERIFY(filterOutData.connectPipe(outPipeY, 1));

    auto outPipeStart = dynamic_cast<fliplib::SynchronePipe<precitec::interface::GeoDoublearray>*>(filter.findPipe("StartPeakX"));
    QVERIFY(outPipeStart);
    QVERIFY(filterOutData.connectPipe(outPipeStart, 1));


    auto outPipeEnd = dynamic_cast<fliplib::SynchronePipe<precitec::interface::GeoDoublearray>*>(filter.findPipe("EndPeakX"));
    QVERIFY(outPipeEnd);
    QVERIFY(filterOutData.connectPipe(outPipeEnd, 1));

    // update parameters
    int extremumType = precitec::filter::ExtremumType::eMaximum;
    int extremumNumber = 1;
    int extremumDistance = 1;
    double extremumDifference = 0.;
    filter.getParameters().update(std::string("ExtremumType"), fliplib::Parameter::TYPE_int, extremumType );
    filter.getParameters().update(std::string("ExtremumNumber"), fliplib::Parameter::TYPE_int, extremumNumber );
    filter.getParameters().update(std::string("ExtremumDistance"), fliplib::Parameter::TYPE_int, extremumDistance );
    filter.getParameters().update(std::string("ExtremumDifference"), fliplib::Parameter::TYPE_double, extremumDifference );

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
    dummyInput.fillData(imageNumber, lineIn, trafoOffset);

    // verify that the filter has run and signal pipes
    QCOMPARE(filterOutData.isProceedCalled(), false);
    dummyInput.signal();
    QCOMPARE(filterOutData.isProceedCalled(), true);
    filterOutData.resetProceedCalled();

    // compare signaled data
    QFETCH(std::vector<DPoint>, expectedOutputPoint);
    QFETCH(std::vector<int>, expectedOutputRank);
    QVERIFY2(expectedOutputPoint.size() == expectedOutputRank.size(), "Test Data inconsistent");

    const auto resultX = outPipeX->read(imageNumber);
    const auto resultY = outPipeY->read(imageNumber);
    const auto resultStart = outPipeStart->read(imageNumber);
    const auto resultEnd = outPipeEnd->read(imageNumber);
    QCOMPARE(resultX.ref().size(), expectedOutputPoint.size());
    QCOMPARE(resultY.ref().size(), expectedOutputPoint.size());
    QCOMPARE(resultStart.ref().size(), expectedOutputPoint.size());
    QCOMPARE(resultEnd.ref().size(), expectedOutputPoint.size());

    for (uint i = 0; i < expectedOutputPoint.size(); ++i)
    {
        auto expectedRank = expectedOutputRank[i];
        QCOMPARE(resultX.ref().getData()[i], expectedOutputPoint[i].x);
        QCOMPARE(resultY.ref().getData()[i], expectedOutputPoint[i].y);
        QCOMPARE(resultX.ref().getRank()[i], expectedRank);
        QCOMPARE(resultY.ref().getRank()[i], expectedRank);
        if (expectedRank > 0)
        {
            QVERIFY(resultStart.ref().getData()[i] <= expectedOutputPoint[i].x);
            QVERIFY(resultEnd.ref().getData()[i] >= expectedOutputPoint[i].x);
        }
    }
}


QTEST_GUILESS_MAIN(LineSelectLocalExtremumTest)

#include "lineSelectLocalExtremumTest.moc"

