#include <QTest>

#include "../binarizeDynamicOnOff.h"
#include "image/image.h"

#include <fliplib/NullSourceFilter.h>
#include <fliplib/BaseFilter.h>

class TestBinarizeDynamicOnOff : public QObject
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
    fliplib::SynchronePipe<precitec::interface::ImageFrame> m_pipeInImage;
    fliplib::SynchronePipe<precitec::interface::GeoDoublearray> m_pipeInThreshold;

    precitec::interface::ImageFrame m_image;
    precitec::interface::GeoDoublearray m_threshold;

    DummyInput()
    : m_pipeInImage{&m_sourceFilter, "image"}
    , m_pipeInThreshold{&m_sourceFilter, "threshold"}
    {
    }

    void connectToFilter(fliplib::BaseFilter * pFilter)
    {
        const auto group = 1;
        //connect  pipes
        m_pipeInImage.setTag("image");
        QVERIFY(pFilter->connectPipe(&m_pipeInImage, group));
        m_pipeInThreshold.setTag("threshold");
        QVERIFY(pFilter->connectPipe(&m_pipeInThreshold, group));
    }

    void fillData(int imageNumber, precitec::geo2d::Point trafoOffset, double threshold)
    {
        const auto height = 2u;
        const auto width = 2u;
        precitec::image::BImage image;
        image.resizeFill(precitec::geo2d::Size(width, height), 255);

        precitec::interface::SmpTrafo oTrafo{new precitec::interface::LinearTrafo(trafoOffset)};
        precitec::interface::ImageContext context (precitec::interface::ImageContext{}, oTrafo);
        context.setImageNumber(imageNumber);

        // fill image
        for (auto i = 0u; i < height; i++)
        {
            for (auto j = 0u; j < width; j++)
            {
                if (i % 2 == 0)
                {
                    image[i][j] = 22;
                } else
                {
                    image[i][j] = 222;
                }
            }
        }

        m_image.data().swap(image);
        m_image.context() = context;
        m_threshold = precitec::interface::GeoDoublearray(context, precitec::geo2d::Doublearray(1, threshold, precitec::filter::eRankMax), precitec::interface::ResultType::AnalysisOK, 1.0);
    }
    void signal()
    {
        m_pipeInImage.signal(m_image);
        m_pipeInThreshold.signal(m_threshold);
    }
};

class DummyFilter : public fliplib::BaseFilter
{
public:
    DummyFilter() : fliplib::BaseFilter("dummy") {}
    void proceedGroup(const void * sender, fliplib::PipeGroupEventArgs & e) override
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

void TestBinarizeDynamicOnOff::testCtor()
{
    precitec::filter::BinarizeDynamicOnOff filter;
    QCOMPARE(filter.name(), std::string("BinarizeDynamicOnOff"));
    QVERIFY(filter.findPipe("ImageFrame") != nullptr);
    QVERIFY(filter.findPipe("threshold") == nullptr);
    QVERIFY(filter.findPipe("image") == nullptr);
    QVERIFY(filter.findPipe("NotAValidPipe") == nullptr);

    // check parameters
    for (auto entry : std::vector<std::pair<std::string, int>> {{"ComparisonType",0}, {"BinarizeType", 0}})
    {
        auto parameter = entry.first;
        auto value = entry.second;
        QVERIFY(filter.getParameters().exists(parameter));
        QCOMPARE(filter.getParameters().findParameter(parameter).getType(), fliplib::Parameter::TYPE_int);
        QCOMPARE(filter.getParameters().findParameter(parameter).getValue().convert<int>(), value);
    }
    auto parameter = "OnOffSwitch";
    QVERIFY(filter.getParameters().exists(parameter));
    QCOMPARE(filter.getParameters().findParameter(parameter).getType(), fliplib::Parameter::TYPE_bool);
    QCOMPARE(filter.getParameters().findParameter(parameter).getValue(), true);

}

void TestBinarizeDynamicOnOff::testProceed_data()
{
    QTest::addColumn<double>("threshold");
    QTest::addColumn<bool>("onOff");
    QTest::addColumn<std::vector<std::vector<int>>>("image");

//                                          threshold    on/off         result
     QTest::newRow("Threshold 100 / off") << 100.0    <<  false    << std::vector<std::vector<int>>{{22, 22}, {222, 222}};
     QTest::newRow("Threshold 100 / on")  << 100.0    <<  true     << std::vector<std::vector<int>>{{255, 255}, {0, 0}};
}

void TestBinarizeDynamicOnOff::testProceed()
{
    precitec::filter::BinarizeDynamicOnOff filter;

    // In-Pipe
    DummyInput dummyInput;
    dummyInput.connectToFilter(&filter);

    //connect  pipes
    auto outPipe = dynamic_cast<fliplib::SynchronePipe<precitec::interface::ImageFrame>*>(filter.findPipe("ImageFrame"));
    QVERIFY(outPipe);

    DummyFilter dummyFilter;
    QVERIFY(dummyFilter.connectPipe(outPipe, 1));

    // update parameters
    // The purpose of this test is to test the new parameter "onOffSwitch"
    QFETCH(bool, onOff);
    filter.getParameters().update(std::string("ComparisonType"), fliplib::Parameter::TYPE_int, 0);
    filter.getParameters().update(std::string("BinarizeType"), fliplib::Parameter::TYPE_int, 2);
    filter.getParameters().update(std::string("OnOffSwitch"), fliplib::Parameter::TYPE_bool, onOff);
    filter.setParameter();

    int imageNumber = 0;
    precitec::geo2d::Point trafoOffset {510,100};
    QFETCH(double, threshold);
    precitec::interface::SmpTrafo oTrafo{new precitec::interface::LinearTrafo(trafoOffset)};
    precitec::interface::ImageContext context (precitec::interface::ImageContext{}, oTrafo);
    context.setImageNumber(imageNumber);
    dummyInput.fillData(imageNumber, trafoOffset, threshold);

    QCOMPARE(dummyFilter.isProceedCalled(), false);
    dummyInput.signal();
    QCOMPARE(dummyFilter.isProceedCalled(), true);
    dummyFilter.resetProceedCalled();

    const auto result = outPipe->read(imageNumber);

    QFETCH(std::vector<std::vector<int>>, image);
    for (auto i = 0; i < result.data().width(); i++)
    {
        for (auto j = 0; j < result.data().height(); j++)
        {
            QCOMPARE(int(result.data()[i][j]), image[i][j]);
        }
    }

    precitec::geo2d::Point ROI_O{0,0};
    QCOMPARE(result.context().trafo()->apply(ROI_O), trafoOffset);
}

QTEST_GUILESS_MAIN(TestBinarizeDynamicOnOff)
#include "testBinarizeDynamicOnOff.moc"
