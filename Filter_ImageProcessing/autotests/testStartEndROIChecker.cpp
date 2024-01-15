#include <QTest>

#include "../startEndROIChecker.h"
#include <fliplib/NullSourceFilter.h>

#include <fliplib/BaseFilter.h>
#include <overlay/overlayCanvas.h>

class TestStartEndROIChecker : public QObject
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
    fliplib::SynchronePipe<precitec::interface::GeoStartEndInfoarray> m_pipeInStartEndInfo;
    fliplib::SynchronePipe<precitec::interface::ImageFrame> m_pipeInImage;

    precitec::interface::GeoStartEndInfoarray m_startEndInfo;
    precitec::interface::ImageFrame m_image;

    DummyInput()
    : m_pipeInStartEndInfo{&m_sourceFilter, "StartEndInfoIn"}
    , m_pipeInImage{&m_sourceFilter, "ImageIn"}
    {
    }

    void connectToFilter(fliplib::BaseFilter * filter )
    {
        int group = 1;
        //connect  pipes
        m_pipeInStartEndInfo.setTag("StartEndInfoIn");
        filter->connectPipe(&( m_pipeInStartEndInfo ), group);

        m_pipeInImage.setTag("ImageIn");
        filter->connectPipe(&( m_pipeInImage ), group);
    }


    void fillData( int imageNumber, precitec::geo2d::Point trafoOffset, int imageState, double mLeft, double qLeft, double mRight, double qRight, bool isTopDark, bool isBottomDark)
    {
        using namespace precitec::interface;
        using namespace precitec::geo2d;

        precitec::interface::SmpTrafo oTrafo{new precitec::interface::LinearTrafo(trafoOffset)};
        precitec::interface::ImageContext context{precitec::interface::ImageContext{}, oTrafo};
        context.setImageNumber(imageNumber);
        precitec::image::BImage image(precitec::geo2d::Size{2,2});
        m_image = precitec::interface::ImageFrame{context, image, precitec::interface::ResultType::AnalysisOK};

        precitec::geo2d::StartEndInfo startEndInfo;
        if (imageState == 1)
        {
            startEndInfo.m_oImageState = precitec::geo2d::StartEndInfo::ImageState::OnlyMaterial;
        }
        else if (imageState == 2)
        {
            startEndInfo.m_oImageState = precitec::geo2d::StartEndInfo::ImageState::OnlyLeftEdgeVisible;
        }
        else if (imageState == 3)
        {
            startEndInfo.m_oImageState = precitec::geo2d::StartEndInfo::ImageState::OnlyRightEdgeVisible;
        }
        else if (imageState == 4)
        {
            startEndInfo.m_oImageState = precitec::geo2d::StartEndInfo::ImageState::FullEdgeVisible;
        }
        startEndInfo.isTopDark = isTopDark;
        startEndInfo.isBottomDark = isBottomDark;
        startEndInfo.leftEdge = precitec::geo2d::StartEndInfo::FittedLine{mLeft, qLeft};
        startEndInfo.rightEdge = precitec::geo2d::StartEndInfo::FittedLine{mRight, qRight};
        startEndInfo.imageWidth = 8;

        precitec::geo2d::StartEndInfoarray startEndInfoarray{1, startEndInfo};
        m_startEndInfo = precitec::interface::GeoStartEndInfoarray{precitec::interface::ImageContext{}, startEndInfoarray};
    }

    void signal()
    {
        m_pipeInStartEndInfo.signal(m_startEndInfo);
        m_pipeInImage.signal(m_image);
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

void TestStartEndROIChecker::testCtor()
{
    precitec::filter::StartEndROIChecker filter;

    QCOMPARE(filter.name(), std::string("StartEndROIChecker"));
    QVERIFY(filter.findPipe("OnBlank") != nullptr);
}

void TestStartEndROIChecker::testProceed_data()
{
    QTest::addColumn<int>("trafoOffsetX");
    QTest::addColumn<int>("trafoOffsetY");
    QTest::addColumn<int>("imageState"); // 1: onlyMaterial, 2: leftEdgeVisible, 3: rightEdgeVisible, 4: fullEdgeVisible
    QTest::addColumn<double>("mLeft");
    QTest::addColumn<double>("qLeft");
    QTest::addColumn<double>("mRight");
    QTest::addColumn<double>("qRight");
    QTest::addColumn<bool>("isTopDark");
    QTest::addColumn<bool>("isBottomDark");
    QTest::addColumn<int>("expectedResult");

    QTest::newRow("Upper left corner background") << 5 << 2 << 2 << -2. << 0. << -2. << 3. << true << false << 1;
    QTest::newRow("Upper left corner blank") << 5 << 2 << 2 << -2. << 0. << -2. << 3. << true << true << 0;
    QTest::newRow("m = 0, partly on background") << 3 << 2 << 4 << 0. << 3. << 0. << 1.5 << false << true << 0;
    QTest::newRow("m = 0, on blank") << 3 << 2 << 4 << 0. << 1. << 0. << 1.5 << true << false << 1;
}

void TestStartEndROIChecker::testProceed()
{
    precitec::filter::StartEndROIChecker filter;

    std::unique_ptr<precitec::image::OverlayCanvas> overlayCanvas{new precitec::image::OverlayCanvas};
    filter.setCanvas(overlayCanvas.get());

    DummyInput dummyInput;
    dummyInput.connectToFilter(&filter);

    auto outPipe = dynamic_cast<fliplib::SynchronePipe<precitec::interface::GeoDoublearray>*>(filter.findPipe("OnBlank"));
    QVERIFY(outPipe);

    DummyFilter dummyFilter;
    QVERIFY(dummyFilter.connectPipe(outPipe, 1));

    QFETCH(int, trafoOffsetX);
    QFETCH(int, trafoOffsetY);
    QFETCH(int, imageState);
    QFETCH(double, mLeft);
    QFETCH(double, qLeft);
    QFETCH(double, mRight);
    QFETCH(double, qRight);
    QFETCH(bool, isTopDark);
    QFETCH(bool, isBottomDark);

    precitec::geo2d::Point trafoOffset{trafoOffsetX, trafoOffsetY};

    int imageNumber = 0;
    dummyInput.fillData(imageNumber, trafoOffset, imageState, mLeft, qLeft, mRight, qRight, isTopDark, isBottomDark);

    filter.setParameter();

    QCOMPARE(dummyFilter.isProceedCalled(), false);
    dummyInput.signal();
    QCOMPARE(dummyFilter.isProceedCalled(), true);
    dummyFilter.resetProceedCalled();

    const auto result = outPipe->read(imageNumber);
    QFETCH(int, expectedResult);

    QCOMPARE(result.ref().size(), 1ul);
    QCOMPARE(result.ref().getData()[0], expectedResult);
}



QTEST_GUILESS_MAIN(TestStartEndROIChecker)
#include "testStartEndROIChecker.moc"

