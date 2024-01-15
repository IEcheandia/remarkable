#include <QtTest/QtTest>

#include "../tileFeature.h"
#include <fliplib/NullSourceFilter.h>

#include <fliplib/BaseFilter.h>
#include <overlay/overlayCanvas.h>

class TestTileFeature : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testCtor();
    void testProceed();
    void testProceed_data();
};

struct DummyInput
{       
    static const int squareWidth = 3;
    static const int numSquaresPerRow = 10;
    static const int numSquaresPerColumn = 10;

    fliplib::NullSourceFilter m_sourceFilter;
    fliplib::SynchronePipe<precitec::interface::ImageFrame> m_pipeImage;
    precitec::interface::ImageFrame m_imageFrame;

    DummyInput()
    : m_pipeImage{ &m_sourceFilter, "framepipe"}
    {
    }
    
    bool connectToFilter(fliplib::BaseFilter * pFilter)
    {
        int group = 0;
        //connect  pipes
        bool ok = pFilter->connectPipe(&( m_pipeImage ), group);
        return ok;
    }
    
    
    void fillData( int imageNumber, precitec::geo2d::Point trafoOffset)
    {
        using namespace precitec::interface;
        using namespace precitec::geo2d;
        
        SmpTrafo oTrafo{new LinearTrafo(trafoOffset)};
        ImageContext context (ImageContext{}, oTrafo);
        context.setImageNumber(imageNumber);

        m_imageFrame = ImageFrame{0, context,
            precitec::image::genSquaresImagePattern( squareWidth, squareWidth, numSquaresPerRow, numSquaresPerColumn ),
            AnalysisOK};
        
    }
    void signal()
    {
        m_pipeImage.signal(m_imageFrame);
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

void TestTileFeature::testCtor()
{
    precitec::filter::TileFeature filter;
    QCOMPARE(filter.name(), std::string("TileFeature"));
    QVERIFY(filter.findPipe("FeatureImageOut") != nullptr);
    QVERIFY(filter.findPipe("NotAValidPipe") == nullptr);

    // check parameters of type int
    for (auto entry : std::vector<std::pair<std::string, int>> {
                {"TileSize", 100},
                {"JumpingDistance", 100},
                {"DrawThreshold", 10}})
    {
        auto parameter = entry.first;
        auto value = entry.second;
        QVERIFY(filter.getParameters().exists(parameter));
        QCOMPARE(filter.getParameters().findParameter(parameter).getType(), fliplib::Parameter::TYPE_UInt32);
        QCOMPARE(filter.getParameters().findParameter(parameter).getValue().convert<int>(), value);   
    }
    {
        std::string parameter = "AlgoTexture";
        int value = 0;
        QVERIFY(filter.getParameters().exists(parameter));
        QCOMPARE(filter.getParameters().findParameter(parameter).getType(), fliplib::Parameter::TYPE_int);
        QCOMPARE(filter.getParameters().findParameter(parameter).getValue().convert<int>(), value);
    }
    
}

void TestTileFeature::testProceed_data()
{
    QTest::addColumn<int>("parameterAlgoTexture");
    QTest::addColumn<int>("parameterTileSize");
    QTest::addColumn<int>("parameterJumpingDistance");
    QTest::addColumn<int>("expectedOutputWidth");
    QTest::addColumn<int>("expectedOutputHeight");
    QTest::addColumn<byte>("expectedValueFirstPixel");
    QTest::addColumn<byte>("expectedValueLastPixel");

    auto expectedSquaresValues = precitec::image::genSquaresImagePattern( 1,1, DummyInput::numSquaresPerRow, DummyInput::numSquaresPerColumn);
    QCOMPARE(expectedSquaresValues.width(), int(DummyInput::numSquaresPerRow));
    QCOMPARE(expectedSquaresValues.height(), int(DummyInput::numSquaresPerColumn));


    QTest::newRow("Variance") << 0 << int(DummyInput::squareWidth) << int(DummyInput::squareWidth)
                                    << int(DummyInput::numSquaresPerColumn) << int (DummyInput::numSquaresPerColumn)
                                    << byte(0) << byte(0);
    QTest::newRow("MeanIntensity") << 4 << int(DummyInput::squareWidth) << int(DummyInput::squareWidth)
                                    << int(DummyInput::numSquaresPerColumn) << int (DummyInput::numSquaresPerColumn)
                                    << expectedSquaresValues[0][0] << expectedSquaresValues[DummyInput::numSquaresPerColumn-1][DummyInput::numSquaresPerRow-1];
    QTest::newRow("Directionality") << 7 << int(DummyInput::squareWidth)  << int(DummyInput::squareWidth)
                                    << int(DummyInput::numSquaresPerColumn) << int (DummyInput::numSquaresPerColumn)
                                    << byte(255) << byte(255);


}
void TestTileFeature::testProceed()
{
    precitec::filter::TileFeature filter;
    std::unique_ptr<precitec::image::OverlayCanvas> pcanvas {new precitec::image::OverlayCanvas};
    filter.setCanvas(pcanvas.get());
    
    DummyInput dummyInput;
    QVERIFY(dummyInput.connectToFilter(&filter));
            
    auto outPipeFeatureImage = dynamic_cast<fliplib::SynchronePipe<precitec::interface::ImageFrame>*>(filter.findPipe("FeatureImageOut"));
    QVERIFY( outPipeFeatureImage );

    DummyFilter dummyFilter;
    QVERIFY(dummyFilter.connectPipe( outPipeFeatureImage, 0));

    QFETCH(int, parameterAlgoTexture);
    QFETCH(int, parameterTileSize);
    QFETCH(int, parameterJumpingDistance);
    
    filter.getParameters().update(std::string("AlgoTexture"), fliplib::Parameter::TYPE_int, parameterAlgoTexture);
    filter.getParameters().update(std::string("TileSize"), fliplib::Parameter::TYPE_UInt32, parameterTileSize);
    filter.getParameters().update(std::string("JumpingDistance"), fliplib::Parameter::TYPE_UInt32, parameterJumpingDistance);
    filter.setParameter();
    
    int imageNumber = 0;
    precitec::geo2d::Point trafoOffset {510,100};
    dummyInput.fillData(imageNumber, trafoOffset);
    QCOMPARE(dummyFilter.isProceedCalled(), false);
    dummyInput.signal();
    QCOMPARE(dummyFilter.isProceedCalled(), true);
    dummyFilter.resetProceedCalled(); 
    
    const auto resultFeatureImage = outPipeFeatureImage->read(imageNumber).data();
    
    QCOMPARE(resultFeatureImage.isValid(), true);
    QTEST(resultFeatureImage.getValue(0,0), "expectedValueFirstPixel");
    QFETCH(int, expectedOutputWidth);
    QFETCH(int, expectedOutputHeight);
    QCOMPARE(resultFeatureImage.width(), expectedOutputWidth);
    QCOMPARE(resultFeatureImage.height(), expectedOutputHeight);
    QTEST(resultFeatureImage.getValue(expectedOutputWidth-1,expectedOutputHeight-1), "expectedValueLastPixel");

    

}



QTEST_GUILESS_MAIN(TestTileFeature)
#include "testTileFeature.moc"
