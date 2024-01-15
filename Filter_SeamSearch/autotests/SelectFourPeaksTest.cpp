
#include <QtTest/QtTest>

#include "../selectFourPeaks.h"
#include <geo/blob.h>
#include <fliplib/NullSourceFilter.h>
#include <overlay/overlayCanvas.h>

#include <fliplib/BaseFilter.h>

Q_DECLARE_METATYPE(precitec::interface::GeoDoublearray)

// If some printed output is required
#define  TEXT_OUT  1

class SelectFourPeaksTest : public QObject
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
    fliplib::SynchronePipe<precitec::interface::GeoVecDoublearray> m_pipeIn_gradientLeft;
    fliplib::SynchronePipe<precitec::interface::GeoVecDoublearray> m_pipeIn_gradientRight;
    fliplib::SynchronePipe<precitec::interface::GeoDoublearray> m_pipeIn_maxFilterLength;
    fliplib::SynchronePipe<precitec::interface::GeoDoublearray> m_pipeIn_imageSize;

    precitec::interface::GeoVecDoublearray m_gradientLeft;
    precitec::interface::GeoVecDoublearray m_gradientRight;
    precitec::interface::GeoDoublearray    m_maxFilterLength;
    precitec::interface::GeoDoublearray    m_imageSize;

    DummyInput()
    : m_pipeIn_gradientLeft(&m_sourceFilter, "gradientLeft")
    , m_pipeIn_gradientRight(&m_sourceFilter, "gradientRight")
    , m_pipeIn_maxFilterLength(&m_sourceFilter, "maxFilterLength")
    , m_pipeIn_imageSize(&m_sourceFilter, "imageSize")
    {
    }

    void connectToFilter(fliplib::BaseFilter* filter )
    {
        int group = 1;

        m_pipeIn_gradientLeft.setTag("gradientLeft");
        QVERIFY(filter->connectPipe(&m_pipeIn_gradientLeft, group));

        m_pipeIn_gradientRight.setTag("gradientRight");
        QVERIFY(filter->connectPipe(&m_pipeIn_gradientRight, group));

        m_pipeIn_maxFilterLength.setTag("maxFilterLength");
        QVERIFY(filter->connectPipe(&m_pipeIn_maxFilterLength, group));

        m_pipeIn_imageSize.setTag("imageSize");
        QVERIFY(filter->connectPipe(&m_pipeIn_imageSize, group));
    }

    void fillData(const int imageNumber, const std::vector<std::vector<double>>& gradientLeftIn, const std::vector<std::vector<double>>& gradientRightIn,
                  const std::vector<double>& maxFilterLengthIn, const std::vector<double>& imageSizeIn, const precitec::geo2d::Point& trafoOffset)
    {
        precitec::interface::SmpTrafo oTrafo{new precitec::interface::LinearTrafo(trafoOffset)};
        precitec::interface::ImageContext context (precitec::interface::ImageContext{}, oTrafo);
        context.setImageNumber(imageNumber);

        // convert in-pipe lines into appropriate data structure
        precitec::geo2d::VecDoublearray gradientLeft (gradientLeftIn.size());
        for (uint i = 0; i < gradientLeftIn.size(); ++i)
        {
            gradientLeft[i].getData().insert(gradientLeft[i].getData().begin(), gradientLeftIn[i].begin(), gradientLeftIn[i].end());
            gradientLeft[i].getRank().assign(gradientLeft[i].getData().size(), precitec::filter::eRankMax);
        }
        m_gradientLeft = precitec::interface::GeoVecDoublearray(context, gradientLeft, precitec::interface::AnalysisOK, precitec::interface::Perfect);

        precitec::geo2d::VecDoublearray gradientRight (gradientRightIn.size());
        for (uint i = 0; i < gradientRightIn.size(); ++i)
        {
            gradientRight[i].getData().insert(gradientRight[i].getData().begin(), gradientRightIn[i].begin(), gradientRightIn[i].end());
            gradientRight[i].getRank().assign(gradientRight[i].getData().size(), precitec::filter::eRankMax);
        }
        m_gradientRight = precitec::interface::GeoVecDoublearray(context, gradientRight, precitec::interface::AnalysisOK, precitec::interface::Perfect);

        precitec::geo2d::Doublearray maxFilterLength (maxFilterLengthIn.size());
        maxFilterLength.getData().insert(maxFilterLength.getData().begin(), maxFilterLengthIn.begin(), maxFilterLengthIn.end());
        maxFilterLength.getRank().assign(maxFilterLength.getData().size(), precitec::filter::eRankMax);
        m_maxFilterLength = precitec::interface::GeoDoublearray(context, maxFilterLength, precitec::interface::AnalysisOK, precitec::interface::Perfect);

        precitec::geo2d::Doublearray imageSize (imageSizeIn.size());
        imageSize.getData().insert(imageSize.getData().begin(), imageSizeIn.begin(), imageSizeIn.end());
        imageSize.getRank().assign(imageSize.getData().size(), precitec::filter::eRankMax);
        m_imageSize = precitec::interface::GeoDoublearray(context, imageSize, precitec::interface::AnalysisOK, precitec::interface::Perfect);
    }

    void signal()
    {
        m_pipeIn_gradientLeft.signal(m_gradientLeft);
        m_pipeIn_gradientRight.signal(m_gradientRight);
        m_pipeIn_maxFilterLength.signal(m_maxFilterLength);
        m_pipeIn_imageSize.signal(m_imageSize);
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

void SelectFourPeaksTest::testCtor()
{
    precitec::filter::SelectFourPeaks filter;

    QCOMPARE(filter.name(), std::string("SelectFourPeaks"));
    QVERIFY(filter.findPipe("FirstPeak") != nullptr);
    QVERIFY(filter.findPipe("SecondPeak") != nullptr);
    QVERIFY(filter.findPipe("ThirdPeak") != nullptr);
    QVERIFY(filter.findPipe("FourthPeak") != nullptr);
}


void SelectFourPeaksTest::testProceed_data()
{
    QTest::addColumn< std::vector<std::vector<double>> > ("gradientLeft");
    QTest::addColumn< std::vector<std::vector<double>> > ("gradientRight");
    QTest::addColumn< std::vector<double> > ("maxFilterLength");
    QTest::addColumn< std::vector<double> > ("imageSize");
    QTest::addColumn< std::vector<double> > ("expectedFirstPeak");
    QTest::addColumn< std::vector<double> > ("expectedSecondPeak");
    QTest::addColumn< std::vector<double> > ("expectedThirdPeak");
    QTest::addColumn< std::vector<double> > ("expectedFourthPeak");

    QTest::newRow("Peaks found")
        << std::vector<std::vector<double>>{ {1, 2, 3,   4, 2, 0, -6, 1, 2, 2, 3, 4,  2, 0, -16, 3, 2} }                   // left gradient
        << std::vector<std::vector<double>>{ {3, 1, 2, -12, 4, 2,  6, 3, 3, 1, 2, 0, -4, 2,   6, 3, 1} }                   // right gradient
        << std::vector<double>{ 2 }                                                                                        // max filter length
        << std::vector<double>{ 1, 17 }                                                                                    // image size
        << std::vector<double>{ 3 } << std::vector<double>{ 6 } << std::vector<double>{ 11 } << std::vector<double>{ 14 }; // expected results

    QTest::newRow("No Peaks found")
        << std::vector<std::vector<double>>{ {1, 21, 3,   4, 2, 0, -6,  1, 2, 2, 3, 4,  22,  0, -16, 3, 2}, {1, 21, 3, 44,  22, 0, -16, 3, 2}, {} } // left gradient
        << std::vector<std::vector<double>>{ {3,  1, 2, -12, 4, 2,  6, 13, 3, 1, 2, 0,  -4, 32,  -6, 3, 1}, {3,  1, -6, 3, 1}, {} }                 // right gradient
        << std::vector<double>{ 2 }                                                                                                             // max filter length
        << std::vector<double>{ 2, 17 }                                                                                                         // image size
        << std::vector<double>{ 0, 0, 0 } << std::vector<double>{ 0, 0, 0 } << std::vector<double>{ 0, 0, 0 } << std::vector<double>{ 0, 0, 0 };            // expected results
}

void SelectFourPeaksTest::testProceed()
{
    precitec::filter::SelectFourPeaks filter;

    std::unique_ptr<precitec::image::OverlayCanvas> canvas {new precitec::image::OverlayCanvas};
    filter.setCanvas(canvas.get());

    // In-Pipe
    DummyInput dummyInput;
    dummyInput.connectToFilter(&filter);

    // connect out pipes
    auto outPipeFirstPeak = dynamic_cast<fliplib::SynchronePipe<precitec::interface::GeoDoublearray>*>(filter.findPipe("FirstPeak"));
    QVERIFY(outPipeFirstPeak);
    auto outPipeSecondPeak = dynamic_cast<fliplib::SynchronePipe<precitec::interface::GeoDoublearray>*>(filter.findPipe("SecondPeak"));
    QVERIFY(outPipeSecondPeak);
    auto outPipeThirdPeak = dynamic_cast<fliplib::SynchronePipe<precitec::interface::GeoDoublearray>*>(filter.findPipe("ThirdPeak"));
    QVERIFY(outPipeThirdPeak);
    auto outPipeFourthPeak = dynamic_cast<fliplib::SynchronePipe<precitec::interface::GeoDoublearray>*>(filter.findPipe("FourthPeak"));
    QVERIFY(outPipeFourthPeak);


    DummyFilter dummyFilter;
    QVERIFY(dummyFilter.connectPipe(outPipeFirstPeak, 0));
    QVERIFY(dummyFilter.connectPipe(outPipeSecondPeak, 0));
    QVERIFY(dummyFilter.connectPipe(outPipeThirdPeak, 0));
    QVERIFY(dummyFilter.connectPipe(outPipeFourthPeak, 0));

    // dummy data
    int imageNumber  =   0;
    int position     = 300;
    precitec::geo2d::Point trafoOffset {510,100};
    precitec::interface::SmpTrafo oTrafo{new precitec::interface::LinearTrafo(trafoOffset)};
    precitec::interface::ImageContext context (precitec::interface::ImageContext{}, oTrafo);
    context.setImageNumber(imageNumber);
    context.setPosition(position);

    QFETCH(std::vector<std::vector<double>>, gradientLeft);
    QFETCH(std::vector<std::vector<double>>, gradientRight);
    QFETCH(std::vector<double>, maxFilterLength);
    QFETCH(std::vector<double>, imageSize);
    dummyInput.fillData(imageNumber, gradientLeft, gradientRight, maxFilterLength, imageSize, trafoOffset);

    // update parameters
    filter.getParameters().update(std::string("HeatAffectedZoneLeftWidth"),     fliplib::Parameter::TYPE_int, 3);
    filter.getParameters().update(std::string("SeamWidth"),                     fliplib::Parameter::TYPE_int, 5);
    filter.getParameters().update(std::string("HeatAffectedZoneRightWidth"),    fliplib::Parameter::TYPE_int, 3);
    filter.getParameters().update(std::string("HeatAffectedZoneLeftVariance"),  fliplib::Parameter::TYPE_int, 2);
    filter.getParameters().update(std::string("SeamVariance"),                  fliplib::Parameter::TYPE_int, 2);
    filter.getParameters().update(std::string("HeatAffectedZoneRightVariance"), fliplib::Parameter::TYPE_int, 2);
    filter.getParameters().update(std::string("PeakWidth"),                     fliplib::Parameter::TYPE_int, 3);
    filter.setParameter();

    QCOMPARE(dummyFilter.isProceedCalled(), false);
    dummyInput.signal();
    QCOMPARE(dummyFilter.isProceedCalled(), true);
    dummyFilter.resetProceedCalled();

    QFETCH(std::vector<double>, expectedFirstPeak);
    QFETCH(std::vector<double>, expectedSecondPeak);
    QFETCH(std::vector<double>, expectedThirdPeak);
    QFETCH(std::vector<double>, expectedFourthPeak);

    const std::vector<double>& resultFirstPeak  = outPipeFirstPeak->read(context.imageNumber()).ref().getData();
    const std::vector<double>& resultSecondPeak = outPipeSecondPeak->read(context.imageNumber()).ref().getData();
    const std::vector<double>& resultThirdPeak  = outPipeThirdPeak->read(context.imageNumber()).ref().getData();
    const std::vector<double>& resultFourthPeak = outPipeFourthPeak->read(context.imageNumber()).ref().getData();
    for (size_t i= 0; i < resultFirstPeak.size(); i++)
    {
        QCOMPARE(resultFirstPeak[i], expectedFirstPeak[i]);
        QCOMPARE(resultSecondPeak[i], expectedSecondPeak[i]);
        QCOMPARE(resultThirdPeak[i], expectedThirdPeak[i]);
        QCOMPARE(resultFourthPeak[i], expectedFourthPeak[i]);
    }
}

QTEST_GUILESS_MAIN(SelectFourPeaksTest)

#include "SelectFourPeaksTest.moc"

