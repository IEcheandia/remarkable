#include <QTest>

#include "../lineModelToLaserline.h"
#include <fliplib/NullSourceFilter.h>

#include <fliplib/BaseFilter.h>
#include <overlay/overlayCanvas.h>

using precitec::interface::GeoVecAnnotatedDPointarray;

Q_DECLARE_METATYPE(precitec::geo2d::DPoint);

class LineModelToLaserlineTest : public QObject
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
    fliplib::SynchronePipe<precitec::interface::GeoLineModelarray> m_pipeIn_line;
    fliplib::SynchronePipe<precitec::interface::GeoDoublearray> m_pipeIn_start;
    fliplib::SynchronePipe<precitec::interface::GeoDoublearray> m_pipeIn_end;

    precitec::interface::GeoLineModelarray m_line;
    precitec::interface::GeoDoublearray m_start;
    precitec::interface::GeoDoublearray m_end;

    DummyInput()
    : m_pipeIn_line(&m_sourceFilter, "Line")
    , m_pipeIn_start(&m_sourceFilter, "Start")
    , m_pipeIn_end(&m_sourceFilter, "End")
    {
    }

    void connectToFilter(fliplib::BaseFilter* pFilter)
    {
        int group = 1;
        m_pipeIn_line.setTag("line");
        m_pipeIn_start.setTag("start");
        m_pipeIn_end.setTag("end");
        QVERIFY(pFilter->connectPipe(&(m_pipeIn_line), group));
        QVERIFY(pFilter->connectPipe(&(m_pipeIn_start), group));
        QVERIFY(pFilter->connectPipe(&(m_pipeIn_end), group));
    }

    void fillData(std::vector<double> coefficientA, std::vector<double> coefficientB, std::vector<double> coefficientC, std::vector<double> start, std::vector<double> end, precitec::geo2d::Point trafoOffset)
    {
        precitec::interface::SmpTrafo oTrafo{new precitec::interface::LinearTrafo(trafoOffset)};
        precitec::interface::ImageContext context (precitec::interface::ImageContext{}, oTrafo);
        context.setImageNumber(0);

        // convert in-pipe lines into appropriate data structure
        precitec::geo2d::LineModelarray lineArray;
        for (auto i = 0u; i < coefficientA.size(); i++)
        {
            precitec::geo2d::LineModel line(0, 0, coefficientA.at(i), coefficientB.at(i), coefficientC.at(i));
            lineArray.getData().push_back(line);
            lineArray.getRank().push_back(precitec::filter::eRankMax);
        }

        m_line = precitec::interface::GeoLineModelarray(context, lineArray, precitec::interface::AnalysisOK, precitec::interface::Perfect);

        precitec::geo2d::Doublearray startarray;
        for (auto i = 0u; i < start.size(); i++)
        {
            startarray.getData().push_back(start.at(i));
            startarray.getRank().push_back(precitec::filter::eRankMax);
        }
        m_start = precitec::interface::GeoDoublearray(context, startarray, precitec::interface::AnalysisOK, precitec::interface::Perfect);

        precitec::geo2d::Doublearray endarray;
        for (auto i = 0u; i < end.size(); i++)
        {
            endarray.getData().push_back(end.at(i));
            endarray.getRank().push_back(precitec::filter::eRankMax);
        }
        m_end = precitec::interface::GeoDoublearray(context, endarray, precitec::interface::AnalysisOK, precitec::interface::Perfect);
    }

    void signal()
    {
        m_pipeIn_line.signal(m_line);
        m_pipeIn_start.signal(m_start);
        m_pipeIn_end.signal(m_end);
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

void LineModelToLaserlineTest::testCtor()
{
    precitec::filter::LineModelToLaserline filter;

    QCOMPARE(filter.name(), std::string("LineModelToLaserline"));
    QVERIFY(filter.findPipe("LineOut") != nullptr);
}

void LineModelToLaserlineTest::testProceed_data()
{

    QTest::addColumn<std::vector<double>> ("coefficientA");
    QTest::addColumn<std::vector<double>> ("coefficientB");
    QTest::addColumn<std::vector<double>> ("coefficientC");
    QTest::addColumn<std::vector<double>> ("start");
    QTest::addColumn<std::vector<double>> ("end");
    QTest::addColumn<int> ("strategy");
    QTest::addColumn< std::vector<std::vector<double>> > ("expectedLine");  // expected result line

    QTest::newRow("First index")   // Displayed test name
            << std::vector<double> {1, 2, 3}
            << std::vector<double> {1, -1, 1}
            << std::vector<double> {0, 0, 0}
            << std::vector<double> {0, 3, 1}
            << std::vector<double> {2, 4, 4}
            << 0
            << std::vector<std::vector<double>> {{0, -1, -2}, {0, 2, 4}, {0, -3, -6}};
    QTest::newRow("shortest line")   // Displayed test name
            << std::vector<double> {1, 2, 3}
            << std::vector<double> {1, -1, 1}
            << std::vector<double> {0, 0, 0}
            << std::vector<double> {0, 3, 1}
            << std::vector<double> {2, 4, 4}
            << 1
            << std::vector<std::vector<double>> {};
    QTest::newRow("longest line")   // Displayed test name
            << std::vector<double> {1, 2, 3}
            << std::vector<double> {1, -1, 1}
            << std::vector<double> {0, 0, 0}
            << std::vector<double> {0, 3, 1}
            << std::vector<double> {2, 4, 4}
            << 2
            << std::vector<std::vector<double>> {{0, -1, -2, -3, -4}, {0, 2, 4, 6, 8}, {0, -3, -6, -9, -12}};
    QTest::newRow("longest line, only 1 start value")   // Displayed test name
            << std::vector<double> {1, 2, 3}
            << std::vector<double> {1, -1, 0}
            << std::vector<double> {0, 0, 0}
            << std::vector<double> {1}
            << std::vector<double> {2, 4, 4}
            << 2
            << std::vector<std::vector<double>> {{-1, -2, -3, -4}, {2, 4, 6, 8}, {}};
}

void LineModelToLaserlineTest::testProceed()
{
    precitec::filter::LineModelToLaserline filter;

    std::unique_ptr<precitec::image::OverlayCanvas> pCanvas {new precitec::image::OverlayCanvas};
    filter.setCanvas(pCanvas.get());

    // connect in-pipes
    DummyInput dummyInput;
    dummyInput.connectToFilter(&filter);

    // create out-pipe
    auto outPipe = dynamic_cast<fliplib::SynchronePipe<precitec::interface::GeoVecDoublearray>*>(filter.findPipe("LineOut"));
    QVERIFY(outPipe);
    DummyFilter filterOutData;
    QVERIFY(filterOutData.connectPipe(outPipe, 0));

    // update parameters
    QFETCH(int, strategy);
    filter.getParameters().update(std::string("Strategy"), fliplib::Parameter::TYPE_int, strategy);
    filter.setParameter();

    // dummy data
    int imageNumber  =   0;
    int position     = 300;
    precitec::geo2d::Point trafoOffset {510,100};

    precitec::interface::ImageContext context;
    context.setImageNumber(imageNumber);
    context.setPosition(position);

    // parse test data
    QFETCH(std::vector<double>, coefficientA);
    QFETCH(std::vector<double>, coefficientB);
    QFETCH(std::vector<double>, coefficientC);
    QFETCH(std::vector<double>, start);
    QFETCH(std::vector<double>, end);
    dummyInput.fillData(coefficientA, coefficientB, coefficientC, start, end, trafoOffset);

    // verify that the filter has run and signal pipes
    QCOMPARE(filterOutData.isProceedCalled(), false);
    dummyInput.signal();
    QCOMPARE(filterOutData.isProceedCalled(), true);
    filterOutData.resetProceedCalled();

    // compare signaled data
    QFETCH(std::vector<std::vector<double>>, expectedLine);
    const auto result = outPipe->read(imageNumber);

    QCOMPARE(result.ref().size(), expectedLine.size());
    for (uint i = 0; i < expectedLine.size(); ++i)
    {
        for (uint j = 0; j < expectedLine[i].size(); ++j)
        {
            QCOMPARE(result.ref()[i].getData()[j], expectedLine[i][j]);
        }
    }
}

QTEST_GUILESS_MAIN(LineModelToLaserlineTest)

#include "lineModelToLaserlineTest.moc"

