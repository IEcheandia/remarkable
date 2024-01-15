#include <QTest>

#include "../dynamicRoiSimple.h"
#include <fliplib/NullSourceFilter.h>

#include <fliplib/BaseFilter.h>
#include <overlay/overlayCanvas.h>

class TestDynamicRoiSimple : public QObject
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
    fliplib::SynchronePipe<precitec::interface::GeoDoublearray> m_pipeInRoiX;
    fliplib::SynchronePipe<precitec::interface::GeoDoublearray> m_pipeInRoiDX;

    precitec::interface::ImageFrame m_image;
    precitec::interface::GeoDoublearray m_roiX;
    precitec::interface::GeoDoublearray m_roiDX;

    DummyInput()
    : m_pipeInImage{&m_sourceFilter, "image"}
    , m_pipeInRoiX{&m_sourceFilter, "roiX"}
    , m_pipeInRoiDX{&m_sourceFilter, "roiDx"}
    {
    }

    void connectToFilter(fliplib::BaseFilter * filter )
    {
        int group = 1;
        //connect  pipes
        m_pipeInImage.setTag("image");
        filter->connectPipe(&m_pipeInImage, group);
        m_pipeInRoiX.setTag("roiX");
        filter->connectPipe(&m_pipeInRoiX, group);
        m_pipeInRoiDX.setTag("roiDx");
        filter->connectPipe(&m_pipeInRoiDX, group);
    }


    void fillData(int imageNumber, int roiX, int roiDX)
    {
        using namespace precitec::interface;
        using namespace precitec::geo2d;

        precitec::interface::SmpTrafo oTrafo{new precitec::interface::LinearTrafo(precitec::geo2d::Point(2, 1))};
        precitec::interface::ImageContext context{precitec::interface::ImageContext{}, oTrafo};
        context.setImageNumber(imageNumber);
        precitec::image::BImage image(precitec::geo2d::Size{4,4});
        m_image = precitec::interface::ImageFrame{context, image, precitec::interface::ResultType::AnalysisOK};

        m_roiX = precitec::interface::GeoDoublearray{precitec::interface::ImageContext{}, precitec::geo2d::Doublearray{1, (double)roiX, precitec::filter::eRankMax}, precitec::interface::ResultType::AnalysisOK};
        m_roiDX = precitec::interface::GeoDoublearray{precitec::interface::ImageContext{}, precitec::geo2d::Doublearray{1, (double)roiDX, precitec::filter::eRankMax}, precitec::interface::ResultType::AnalysisOK};
    }

    void signal()
    {
        m_pipeInImage.signal(m_image);
        m_pipeInRoiX.signal(m_roiX);
        m_pipeInRoiDX.signal(m_roiDX);
    }
};

class DummyFilter : public fliplib::BaseFilter
{
public:
    DummyFilter() : fliplib::BaseFilter("dummy") {}
    void proceed (const void * sender, fliplib::PipeEventArgs & e) override
    {
        Q_UNUSED(sender)
        Q_UNUSED(e)
        preSignalAction();
        m_proceedCalled = true;
    }
    void proceedGroup (const void * sender, fliplib::PipeGroupEventArgs & e) override
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

void TestDynamicRoiSimple::testCtor()
{
    precitec::filter::DynamicRoiSimple filter;

    QCOMPARE(filter.name(), std::string("DynamicRoiSimple"));
    QVERIFY(filter.findPipe("SubFrame") != nullptr);
}

void TestDynamicRoiSimple::testProceed_data()
{
    QTest::addColumn<int>("roiX");
    QTest::addColumn<int>("roiY");
    QTest::addColumn<int>("roiDX");
    QTest::addColumn<int>("roiDY");
    QTest::addColumn<int>("expectedX");
    QTest::addColumn<int>("expectedY");
    QTest::addColumn<int>("expectedDX");
    QTest::addColumn<int>("expectedDY");

    QTest::newRow("on image") << 1 << 1 << 2 << 2 << 3 << 2 << 2 << 2;
    QTest::newRow("half on image") << 1 << 3 << 2 << 2 << 3 << 3 << 2 << 2;
    QTest::newRow("outside") << 5 << 1 << 4 << 1 << 2 << 1 << 0 << 0;
    QTest::newRow("on image border") << 2 << 4 << 4 << 3  << 2 << 1 << 0 << 0;
    QTest::newRow("inside but empty") << 1 << 2 << 0 << 1 << 2 << 1 << 0 << 0;
}

void TestDynamicRoiSimple::testProceed()
{
    precitec::filter::DynamicRoiSimple filter;

    std::unique_ptr<precitec::image::OverlayCanvas> overlayCanvas{new precitec::image::OverlayCanvas};
    filter.setCanvas(overlayCanvas.get());

    DummyInput dummyInput;
    dummyInput.connectToFilter(&filter);

    auto outPipe = dynamic_cast<fliplib::SynchronePipe<precitec::interface::ImageFrame>*>(filter.findPipe("SubFrame"));
    QVERIFY(outPipe);

    DummyFilter dummyFilter;
    QVERIFY(dummyFilter.connectPipe(outPipe, 1));

    QFETCH(int, roiX);
    QFETCH(int, roiY);
    QFETCH(int, roiDX);
    QFETCH(int, roiDY);

    int imageNumber = 0;
    dummyInput.fillData(imageNumber, roiX, roiDX);

    filter.getParameters().update(std::string("roiY"), fliplib::Parameter::TYPE_int, roiY);
    filter.getParameters().update(std::string("roiDy"), fliplib::Parameter::TYPE_int, roiDY);
    filter.getParameters().update(std::string("xIsStart"), fliplib::Parameter::TYPE_bool, true);
    filter.setParameter();

    QCOMPARE(dummyFilter.isProceedCalled(), false);
    dummyInput.signal();
    QCOMPARE(dummyFilter.isProceedCalled(), true);
    dummyFilter.resetProceedCalled();

    QFETCH(int, expectedX);
    QFETCH(int, expectedY);
    QFETCH(int, expectedDX);
    QFETCH(int, expectedDY);

    const auto result = outPipe->read(imageNumber);

    QCOMPARE(result.context().getTrafoX(), expectedX);
    QCOMPARE(result.context().getTrafoY(), expectedY);
    QCOMPARE(result.data().width(), expectedDX);
    QCOMPARE(result.data().height(), expectedDY);
}



QTEST_GUILESS_MAIN(TestDynamicRoiSimple)
#include "testDynamicRoiSimple.moc"


