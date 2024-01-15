#include <QtTest/QtTest>

#include "../adjustContrast.h"
#include <fliplib/NullSourceFilter.h>

#include <fliplib/BaseFilter.h>
#include <overlay/overlayCanvas.h>

class TestAdjustContrast : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testCtor();
    void testProceed();
    void testProceed_data();
};

using namespace precitec::interface;
struct DummyInput
{       

    static const int squareWidth = 3;
    static const int numSquaresPerRow = 10;
    static const int numSquaresPerColumn = 10;
    static const int m_minIntensity = 8;
    static const int m_maxIntensity = 240;

    fliplib::NullSourceFilter m_sourceFilter;
    fliplib::SynchronePipe<ImageFrame> m_pipeImage;
    fliplib::SynchronePipe<GeoDoublearray> m_pipeThreshold1;
    fliplib::SynchronePipe<GeoDoublearray> m_pipeThreshold2;
    ImageFrame m_imageFrame;
    GeoDoublearray m_threshold1;
    GeoDoublearray m_threshold2;

    DummyInput()
    : m_pipeImage{ &m_sourceFilter, "framepipe"},
    m_pipeThreshold1{ & m_sourceFilter, "lut1"},
    m_pipeThreshold2{ & m_sourceFilter, "lut2"}
    {
        m_pipeImage.setTag("image");
        m_pipeThreshold1.setTag("threshold1");
        m_pipeThreshold2.setTag("threshold2");
    }
    
    bool connectToFilter(fliplib::BaseFilter * pFilter)
    {
        int group = 1;
        //connect  pipes
        bool ok = pFilter->connectPipe(&( m_pipeImage ), group);
        ok &= pFilter->connectPipe(&( m_pipeThreshold1 ), group);
        ok &= pFilter->connectPipe(&( m_pipeThreshold2 ), group);
        return ok;
    }
    
    
    void fillData( int imageNumber, double threshold1, double threshold2, precitec::geo2d::Point trafoOffset)
    {
        using namespace precitec::interface;
        using namespace precitec::geo2d;
        
        SmpTrafo oTrafo{new LinearTrafo(trafoOffset)};
        ImageContext context (ImageContext{}, oTrafo);
        context.setImageNumber(imageNumber);

        m_imageFrame = ImageFrame{0, context,
            precitec::image::genSquaresImagePattern( squareWidth, squareWidth, numSquaresPerRow, numSquaresPerColumn, m_minIntensity, m_maxIntensity ),
            AnalysisOK};
        m_threshold1 = GeoDoublearray{m_imageFrame.context(),
                                    precitec::geo2d::Doublearray{1, threshold1, precitec::filter::eRankMax},
                                    AnalysisOK, 1.0 };
        m_threshold2 = GeoDoublearray{m_imageFrame.context(),
                                    precitec::geo2d::Doublearray{1, threshold2, precitec::filter::eRankMax},
                                    AnalysisOK, 1.0 };
        
    }
    void signal()
    {
        m_pipeImage.signal(m_imageFrame);
        m_pipeThreshold1.signal(m_threshold1);
        m_pipeThreshold2.signal(m_threshold2);
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

void TestAdjustContrast::testCtor()
{
    precitec::filter::AdjustContrast filter;
    QCOMPARE(filter.name(), std::string("AdjustContrast"));
    QVERIFY(filter.findPipe("ImageFrame") != nullptr);
    QVERIFY(filter.findPipe("NotAValidPipe") == nullptr);

    QVERIFY(filter.getParameters().exists("Operation"));
    QCOMPARE(filter.getParameters().findParameter("Operation").getType(), fliplib::Parameter::TYPE_int);
    QCOMPARE(filter.getParameters().findParameter("Operation").getValue().convert<int>(), 0);

}

void TestAdjustContrast::testProceed_data()
{
    QTest::addColumn<int>("parameterOperation");
    QTest::addColumn<double>("threshold1");
    QTest::addColumn<double>("threshold2");
    QTest::addColumn<byte>("expectedValueFirstPixel");
    QTest::addColumn<byte>("expectedValueLastPixel");

    auto expectedSquaresValues = precitec::image::genSquaresImagePattern( DummyInput::squareWidth, DummyInput::squareWidth, DummyInput::numSquaresPerRow, DummyInput::numSquaresPerColumn, DummyInput::m_minIntensity, DummyInput::m_maxIntensity);
    const auto inputFirstPixel  = expectedSquaresValues.getValue(0,0);
    const auto inputLastPixel  = expectedSquaresValues.getValue(expectedSquaresValues.width() - 1, expectedSquaresValues.height() -1);

    QTest::newRow("KeepLUT") << 0 << -1.0 << 256.0 << inputFirstPixel << inputLastPixel;
    QTest::newRow("Invert") << 0 << 255.0 << 0.0 << byte(255 - inputFirstPixel) << byte(255 -inputLastPixel);


    QVERIFY(inputFirstPixel == DummyInput::m_minIntensity);
    QVERIFY(inputLastPixel == DummyInput::m_maxIntensity);
    QTest::newRow("Rescale") << 0 << double(DummyInput::m_minIntensity) << double(DummyInput::m_maxIntensity) << byte(0) << byte(255);
    QTest::newRow("RescaleInvert") << 0 << double(DummyInput::m_maxIntensity) << double(DummyInput::m_minIntensity) << byte(255) << byte();

    QVERIFY(inputFirstPixel < 100);
    QVERIFY(inputLastPixel > 100);
    QTest::newRow("SaturateOver100") << 0 << 0.0 << 100.0 << byte(inputFirstPixel*255/100.0) << byte(255);
    QTest::newRow("BinBelow100") << 1 << 0.0 << 100.0 << byte(255.0) << byte(0.0);
    QTest::newRow("BinOver100") << 2 << 0.0 << 100.0 << byte(0.0) << byte(255.0);
    QTest::newRow("MaskOver100") << 3 << 0.0 << 100.0 << byte(inputFirstPixel*255/100.0) << byte(0);



}
void TestAdjustContrast::testProceed()
{
    precitec::filter::AdjustContrast filter;
    std::unique_ptr<precitec::image::OverlayCanvas> pcanvas {new precitec::image::OverlayCanvas};
    filter.setCanvas(pcanvas.get());
    
    DummyInput dummyInput;
    QVERIFY(dummyInput.connectToFilter(&filter));
            
    auto outPipeFeatureImage = dynamic_cast<fliplib::SynchronePipe<precitec::interface::ImageFrame>*>(filter.findPipe("ImageFrame"));
    QVERIFY( outPipeFeatureImage );

    DummyFilter dummyFilter;
    QVERIFY(dummyFilter.connectPipe( outPipeFeatureImage, 0));

    QFETCH(int, parameterOperation);
    QFETCH(double, threshold1);
    QFETCH(double, threshold2);
    
    filter.getParameters().update(std::string("Operation"), fliplib::Parameter::TYPE_int, parameterOperation);
    filter.setParameter();
    
    int imageNumber = 0;
    precitec::geo2d::Point trafoOffset {510,100};
    dummyInput.fillData(imageNumber,  threshold1, threshold2, trafoOffset);
    QCOMPARE(dummyFilter.isProceedCalled(), false);
    dummyInput.signal();
    QCOMPARE(dummyFilter.isProceedCalled(), true);
    dummyFilter.resetProceedCalled(); 
    
    const auto resultFeatureImage = outPipeFeatureImage->read(imageNumber).data();
    
    QCOMPARE(resultFeatureImage.isValid(), true);
    QTEST(*resultFeatureImage.begin(), "expectedValueFirstPixel");
    QCOMPARE(resultFeatureImage.size(), dummyInput.m_imageFrame.data().size());
    QTEST(*(resultFeatureImage.end() -1), "expectedValueLastPixel");

    

}



QTEST_GUILESS_MAIN(TestAdjustContrast)
#include "testAdjustContrast.moc"
