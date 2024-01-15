#include <QTest>
#include "filter/algoImage.h"
#include "filter/samplingInformation.h"
#include <random> 

using namespace precitec::image;
using precitec::geo2d::Point;
using precitec::geo2d::DPoint;
using precitec::filter::resampleFrame;
using precitec::filter::SamplingInformation;
using precitec::filter::upsampleImage;
using precitec::filter::downsampleImage;
using precitec::interface::ImageContext;
using precitec::interface::LinearTrafo;
using precitec::interface::SmpTrafo;


class TestResampleImage : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void testDownsampleImage_data();
    void testDownsampleImage();
    void testUpsampleImage_data();
    void testUpsampleImage();
    void testResampleUpsampledImage_data();
    void testResampleUpsampledImage();
    void testResampleDownsampledImage_data();
    void testResampleDownsampledImage();
};



Point applyTrafo(precitec::interface::Trafo & rTrafo, const DPoint & point)
{
    return rTrafo(Point(std::round(point.x), std::round(point.y)));
};


void TestResampleImage::testDownsampleImage_data()
{
    QTest::addColumn<int>("tileWidth");
    QTest::addColumn<int>("tileHeight");
    QTest::addColumn<int>("numTilesPerRow");
    QTest::addColumn<int>("numTilesPerColumn");
    QTest::addRow("6x5") << 10 << 10 << 6 << 5;
    QTest::addRow("sampling1") << 1 << 1 << 5 << 5;
    QTest::addRow("1x1") << 10 << 10 << 1 << 1;

}


void TestResampleImage::testDownsampleImage()
{
    QFETCH(int, tileWidth);
    QFETCH(int, tileHeight);
    QFETCH(int, numTilesPerRow);
    QFETCH(int, numTilesPerColumn);
    
    Size2d inputSize{tileWidth*numTilesPerRow, tileHeight*numTilesPerColumn};
    
    BImage expectedOutImage = genSquaresImagePattern(1, 1, numTilesPerRow, numTilesPerColumn);
    //srcImage represents an image upsampled by tileWidth * tileHeight
    BImage srcImage = genSquaresImagePattern(tileWidth, tileHeight, numTilesPerRow, numTilesPerColumn);
    
    ImageContext srcContext{ ImageContext{}, 
                             SmpTrafo{new LinearTrafo{0,0}}};
    srcContext.SamplingX_ = inputSize.width / double(expectedOutImage.width());
    srcContext.SamplingY_ = inputSize.height / double(expectedOutImage.height());
    
    
    BImage outImage;
    ImageContext outContext;
    resampleFrame(outImage, outContext, srcImage, srcContext);
    QCOMPARE(outContext.SamplingX_, 1.0);
    QCOMPARE(outContext.SamplingY_, 1.0);
    auto outTrafo = outContext.trafo();
    QCOMPARE(outTrafo->dx(),0);
    QCOMPARE(outTrafo->dy(),0);
    
    QCOMPARE(outImage.width(), expectedOutImage.width());
    QCOMPARE(outImage.height(), expectedOutImage.height());
    
    for (int y = 0, yMax = expectedOutImage.height(); y < yMax; y++)
    {
        for (int x = 0, xMax = expectedOutImage.width(); x < xMax; x++)
        {
            QCOMPARE(outImage.getValue(x,y), expectedOutImage.getValue(x,y)); 
        }            
    }
    
}

void TestResampleImage::testUpsampleImage_data()
{
    QTest::addColumn<int>("tileWidth");
    QTest::addColumn<int>("tileHeight");
    QTest::addColumn<int>("numTilesPerRow");
    QTest::addColumn<int>("numTilesPerColumn");
    QTest::addRow("6x5") << 10 << 10 << 6 << 5;
    QTest::addRow("sampling1") << 1 << 1 << 5 << 5;
    QTest::addRow("1x1") << 10 << 10 << 1 << 1;

}


void TestResampleImage::testUpsampleImage()
{
    QFETCH(int, tileWidth);
    QFETCH(int, tileHeight);
    QFETCH(int, numTilesPerRow);
    QFETCH(int, numTilesPerColumn);
    
    Size2d inputSize{numTilesPerRow, numTilesPerColumn};
    
    BImage expectedOutImage = genSquaresImagePattern(tileWidth, tileHeight, numTilesPerRow, numTilesPerColumn);
    //srcImage represents an image downsampled by tileWidth * tileHeight
    BImage srcImage = genSquaresImagePattern(1, 1, numTilesPerRow, numTilesPerColumn);
    
    ImageContext srcContext{ ImageContext{}, 
                             SmpTrafo{new LinearTrafo{0,0}}};
    srcContext.SamplingX_ = inputSize.width / double(expectedOutImage.width()); 
    srcContext.SamplingY_ = inputSize.height / double(expectedOutImage.height());
    
    
    BImage outImage;
    ImageContext outContext;
    resampleFrame(outImage, outContext, srcImage, srcContext);
    QCOMPARE(outContext.SamplingX_, 1.0);
    QCOMPARE(outContext.SamplingY_, 1.0);
    auto outTrafo = outContext.trafo();
    QCOMPARE(outTrafo->dx(),0);
    QCOMPARE(outTrafo->dy(),0);
    
    QCOMPARE(outImage.width(), expectedOutImage.width());
    QCOMPARE(outImage.height(), expectedOutImage.height());
    
    for (int y = 0, yMax = expectedOutImage.height(); y < yMax; y++)
    {
        for (int x = 0, xMax = expectedOutImage.width(); x < xMax; x++)
        {
            QCOMPARE(outImage.getValue(x,y), expectedOutImage.getValue(x,y)); 
        }            
    }
    
}
void TestResampleImage::testResampleUpsampledImage_data()
{
    QTest::addColumn<int>("referenceWidth");
    QTest::addColumn<int>("referenceHeight");   
    QTest::addColumn<int>("upsampleFactorX");
    QTest::addColumn<int>("upsampleFactorY"); 
    QTest::addColumn<int>("trafoX");
    QTest::addColumn<int>("trafoY");
    QTest::addRow("noTrafo") << 10 << 15 << 5 << 3 << 0 << 0;
    QTest::addRow("withTrafo") << 10 << 15 << 5 << 3 << 10 << 20 ;

}

void TestResampleImage::testResampleUpsampledImage()
{
    QFETCH(int, referenceWidth );
    QFETCH(int, referenceHeight );
    QFETCH(int, trafoX);
    QFETCH(int, trafoY);
    QFETCH(int, upsampleFactorX );
    QFETCH(int, upsampleFactorY );
    
    
    Size2d referenceSize {referenceWidth, referenceHeight};
    const int seed =1;
    BImage referenceImage = precitec::filter::genNoiseImage(referenceSize, seed);
    
    ImageContext contextReference{ ImageContext{}, 
                             precitec::interface::SmpTrafo{new precitec::interface::LinearTrafo{trafoX, trafoY}}};
    auto & trafoReference = *contextReference.trafo();
                             
                            
    BImage upsampledImage{Size2d{referenceWidth*upsampleFactorX, referenceHeight*upsampleFactorY}};
    upsampleImage(upsampledImage,referenceImage, upsampleFactorX, upsampleFactorY );
    Point expectedOffsetToReferenceImage(0,0);
    
    ImageContext contextUpsampled{ ImageContext{}, 
        SmpTrafo{new precitec::interface::LinearTrafo{trafoReference(expectedOffsetToReferenceImage)}}};
    contextUpsampled.SamplingX_ = upsampledImage.width() / double( referenceSize.width);
    contextUpsampled.SamplingY_ = upsampledImage.height() / double( referenceSize.height);
    
    BImage resampledImage;
    ImageContext contextResampled;
    resampleFrame(resampledImage, contextResampled, upsampledImage, contextUpsampled );
    QCOMPARE( contextResampled.SamplingX_, 1.0);
    QCOMPARE( contextResampled.SamplingY_, 1.0);
    auto & trafoResampled = *contextResampled.trafo();
    
    
    SamplingInformation upsampledImageInfo{ upsampledImage.size(), contextUpsampled.SamplingX_, contextUpsampled.SamplingY_ };
    
    auto referenceSizeWithoutLostBorder = upsampledImageInfo.getReferenceImageSize(SamplingInformation::BorderMode::excluded);
    auto referenceSizeIncludingLostBorder = upsampledImageInfo.getReferenceImageSize(SamplingInformation::BorderMode::included);
    
    //in the upsampling case no border is lost
    auto offsetToReferenceImage = upsampledImageInfo.m_offsetToReferenceImage;
    QCOMPARE(offsetToReferenceImage, Point(0,0));
    QCOMPARE(resampledImage.size(), referenceSize);
    
    QCOMPARE(referenceSizeIncludingLostBorder, referenceSize);
    QCOMPARE(referenceSizeWithoutLostBorder, resampledImage.size());
    
    // check corners of upsampledImage 
    
    DPoint pointUpsampledImage, expectedPointImageExactMultiple;
    for (auto entry : {
                std::tuple<DPoint, DPoint>{DPoint(0,0), DPoint(0, 0)},
                std::tuple<DPoint, DPoint>{DPoint( upsampledImage.width(), upsampledImage.height()), DPoint(referenceSizeWithoutLostBorder.width, referenceSizeWithoutLostBorder.height)}
                })
    {
        
        std::tie( pointUpsampledImage, expectedPointImageExactMultiple) = entry;
    
        auto pointRefImageExactMultiple = upsampledImageInfo.transformToReferenceImage( pointUpsampledImage.x, pointUpsampledImage.y,SamplingInformation::BorderMode::excluded);
        QCOMPARE( pointRefImageExactMultiple, expectedPointImageExactMultiple);
        
        QCOMPARE( upsampledImageInfo.transformToCurrentImage ( pointRefImageExactMultiple.x, pointRefImageExactMultiple.y , SamplingInformation::BorderMode::excluded), pointUpsampledImage );
        
        auto pointRefImageWithBorder = upsampledImageInfo.transformToReferenceImage( pointUpsampledImage.x, pointUpsampledImage.y ,SamplingInformation::BorderMode::included);
        QCOMPARE(applyTrafo(trafoResampled,pointRefImageExactMultiple), applyTrafo(trafoReference, pointRefImageWithBorder));
        
        QCOMPARE( upsampledImageInfo.transformToCurrentImage ( pointRefImageWithBorder.x, pointRefImageWithBorder.y, SamplingInformation::BorderMode::included), pointUpsampledImage );
        

    }
    {
        
        //compare pixels on upsampled image to reference (known because it's a nearest-neighbout interpolation)
        
        for ( unsigned int yUpsampled = 0; yUpsampled < (unsigned int) upsampledImage.height(); yUpsampled++ )
        {        
            for (unsigned int xUpsampled = 0; xUpsampled < (unsigned int) upsampledImage.width(); xUpsampled++ )
            {
                auto oReferencePosition = upsampledImageInfo.transformToReferenceImage( xUpsampled, yUpsampled, SamplingInformation::BorderMode::excluded);
                int x_ = int(oReferencePosition.x);
                int y_ = int(oReferencePosition.y);
                
                auto oEquivalentUpsampledPosition = upsampledImageInfo.transformToCurrentImage( oReferencePosition.x, oReferencePosition.y, SamplingInformation::BorderMode::excluded);
                QCOMPARE(std::round( oEquivalentUpsampledPosition.x), double( xUpsampled ));
                QCOMPARE(std::round( oEquivalentUpsampledPosition.y), double( yUpsampled ));
                
                QCOMPARE( upsampledImage[yUpsampled][xUpsampled], resampledImage[y_][x_]);
                QCOMPARE( upsampledImage[yUpsampled][xUpsampled], referenceImage[y_+offsetToReferenceImage.y][x_ + offsetToReferenceImage.x]);
            }
        }
        
        
        for ( unsigned int yResampled = 0; yResampled < (unsigned int)resampledImage.height(); yResampled++ )
        {        
            for (unsigned int xResampled = 0; xResampled < (unsigned int)resampledImage.width(); xResampled++ )
            {
                auto oPositionOnUpsampledImage = upsampledImageInfo.transformToCurrentImage( xResampled, yResampled, SamplingInformation::BorderMode::excluded);
                int y_ = int( oPositionOnUpsampledImage.y);
                int x_ = int( oPositionOnUpsampledImage.x);
                
                QCOMPARE(resampledImage[yResampled][xResampled], upsampledImage[y_][x_]);
                
                auto oEquivalentReferencePosition = upsampledImageInfo.transformToReferenceImage( oPositionOnUpsampledImage.x, oPositionOnUpsampledImage.y, SamplingInformation::BorderMode::excluded);
                QCOMPARE(oEquivalentReferencePosition,  DPoint( xResampled, yResampled ));
                
                if ( x_ == oPositionOnUpsampledImage.x &&  y_  == oPositionOnUpsampledImage.y)
                {
                    QCOMPARE(referenceImage[yResampled + offsetToReferenceImage.y][xResampled+offsetToReferenceImage.x], upsampledImage[y_][x_]);
                }
            
            }
        }            
    }
}


void TestResampleImage::testResampleDownsampledImage_data()
{
    QTest::addColumn<int>("referenceWidth");
    QTest::addColumn<int>("referenceHeight");    
    QTest::addColumn<int>("downsampleFactorX");
    QTest::addColumn<int>("downsampleFactorY");
    QTest::addColumn<int>("trafoX");
    QTest::addColumn<int>("trafoY");
    QTest::addRow("noTrafo") << 50 << 60 << 5 << 10 << 0 << 0 ;
    QTest::addRow("withTrafo") << 50 << 60 << 5 << 10 << 10 << 20 ;
    QTest::addRow("lostBorder") << 50 << 60 << 4 << 8 << 10 << 20 ;
    QTest::addRow("noSampling") << 50 << 60 << 1 << 1 << 10 << 20 ;

}


void TestResampleImage::testResampleDownsampledImage()
{
    QFETCH(int, referenceWidth );
    QFETCH(int, referenceHeight );
    QFETCH(int, downsampleFactorX );
    QFETCH(int, downsampleFactorY );
    QFETCH(int, trafoX);
    QFETCH(int, trafoY);
    
    
    Size2d referenceSize {referenceWidth, referenceHeight};
    const int seed =1;
    BImage referenceImage = precitec::filter::genNoiseImage(referenceSize, seed);
    
    ImageContext contextReference{ ImageContext{}, 
                             precitec::interface::SmpTrafo{new precitec::interface::LinearTrafo{trafoX, trafoY}}};
    auto & trafoReference = *contextReference.trafo();
                             
                            
    BImage downsampledImage{Size2d{referenceWidth/downsampleFactorX, referenceHeight/downsampleFactorY}};
    downsampleImage( downsampledImage,referenceImage, downsampleFactorX, downsampleFactorY);
    Point expectedOffsetToReferenceImage((referenceWidth%downsampleFactorX)/2, (referenceHeight%downsampleFactorY)/2);
    
    ImageContext contextDownsampled{ ImageContext{}, 
        SmpTrafo{new precitec::interface::LinearTrafo{trafoReference(expectedOffsetToReferenceImage)}}};
    contextDownsampled.SamplingX_ = downsampledImage.width() / double( referenceSize.width);
    contextDownsampled.SamplingY_ = downsampledImage.height() / double( referenceSize.height);
    auto & trafoDownsampled = *contextDownsampled.trafo();
    
    BImage resampledImage;
    ImageContext contextResampled;
    resampleFrame(resampledImage, contextResampled, downsampledImage, contextDownsampled );
    QCOMPARE( contextResampled.SamplingX_, 1.0);
    QCOMPARE( contextResampled.SamplingY_, 1.0);
    auto & trafoResampled = *contextResampled.trafo();
    
    
    SamplingInformation downsampledImageInfo{downsampledImage.size(), contextDownsampled.SamplingX_, contextDownsampled.SamplingY_ };
    
    auto referenceSizeWithoutLostBorder = downsampledImageInfo.getReferenceImageSize(SamplingInformation::BorderMode::excluded);
    auto referenceSizeIncludingLostBorder = downsampledImageInfo.getReferenceImageSize(SamplingInformation::BorderMode::included);
    
    QCOMPARE(referenceSizeIncludingLostBorder, referenceSize);
    QCOMPARE(referenceSizeWithoutLostBorder, resampledImage.size());
    
    if (referenceSize.width % downsampleFactorX == 0)
    {
        QCOMPARE( resampledImage.width(), referenceSize.width );
    }
    else
    {
        QVERIFY( resampledImage.width() < referenceSize.width);
    }
    
    if (referenceSize.height % downsampleFactorY == 0)
    {
        QCOMPARE( resampledImage.height(), referenceSize.height );
    }
    else
    {
        QVERIFY( resampledImage.height() < referenceSize.height);
    }
    
    auto offsetToReferenceImage = downsampledImageInfo.m_offsetToReferenceImage;  // the border which was lost during downsampling

    // check corners of downsampledImage 
    QCOMPARE(trafoReference.dx(), trafoX);
    QCOMPARE(trafoReference.dy(), trafoY);
    QCOMPARE(trafoDownsampled.dx(), trafoX + offsetToReferenceImage.x);
    QCOMPARE(trafoDownsampled.dy(), trafoY + offsetToReferenceImage.y);
    QCOMPARE(trafoResampled.dx(), trafoX + offsetToReferenceImage.x);
    QCOMPARE(trafoResampled.dy(), trafoY + offsetToReferenceImage.y);
    

    DPoint pointDownsampledImage, expectedPointImageExactMultiple;
    for (auto entry : {
                std::tuple<DPoint, DPoint>{DPoint(0,0), DPoint(0, 0)},
                std::tuple<DPoint, DPoint>{DPoint(downsampledImage.width(), downsampledImage.height()), DPoint(referenceSizeWithoutLostBorder.width, referenceSizeWithoutLostBorder.height)}
                })
    {
        
        std::tie(pointDownsampledImage, expectedPointImageExactMultiple) = entry;
    
        auto pointRefImageExactMultiple = downsampledImageInfo.transformToReferenceImage( pointDownsampledImage.x, pointDownsampledImage.y,SamplingInformation::BorderMode::excluded);
        QCOMPARE( pointRefImageExactMultiple, expectedPointImageExactMultiple);
        
        QCOMPARE( downsampledImageInfo.transformToCurrentImage ( pointRefImageExactMultiple.x, pointRefImageExactMultiple.y , SamplingInformation::BorderMode::excluded), pointDownsampledImage );
        
        auto pointRefImageWithBorder = downsampledImageInfo.transformToReferenceImage( pointDownsampledImage.x, pointDownsampledImage.y ,SamplingInformation::BorderMode::included);
        QCOMPARE(applyTrafo(trafoResampled,pointRefImageExactMultiple), applyTrafo(trafoReference, pointRefImageWithBorder));
        
        QCOMPARE( downsampledImageInfo.transformToCurrentImage ( pointRefImageWithBorder.x, pointRefImageWithBorder.y, SamplingInformation::BorderMode::included), pointDownsampledImage );
        
    }
    {        
        //compare pixels on downsampled image to reference (known because it's a nearest-neighbout interpolation)
        
        for ( unsigned int yDownsampled = 0; yDownsampled < (unsigned int) downsampledImage.height(); yDownsampled++ )
        {        
            for (unsigned int xDownsampled = 0; xDownsampled < (unsigned int) downsampledImage.width(); xDownsampled++ )
            {
                auto oReferencePosition = downsampledImageInfo.transformToReferenceImage( xDownsampled, yDownsampled, SamplingInformation::BorderMode::excluded);
                int x_ = int(oReferencePosition.x);
                int y_ = int(oReferencePosition.y);
                
                auto oEquivalentDownsampledPosition = downsampledImageInfo.transformToCurrentImage( oReferencePosition.x, oReferencePosition.y, SamplingInformation::BorderMode::excluded);
                QCOMPARE(std::round( oEquivalentDownsampledPosition.x), double( xDownsampled ));
                QCOMPARE(std::round( oEquivalentDownsampledPosition.y), double( yDownsampled ));
                
                QCOMPARE( downsampledImage[yDownsampled][xDownsampled], resampledImage[y_][x_]);
                QCOMPARE( downsampledImage[yDownsampled][xDownsampled], referenceImage[y_+offsetToReferenceImage.y][x_ + offsetToReferenceImage.x]);
            }
        }
        
        
        for ( unsigned int yResampled = 0; yResampled < (unsigned int)resampledImage.height(); yResampled++ )
        {        
            for (unsigned int xResampled = 0; xResampled < (unsigned int)resampledImage.width(); xResampled++ )
            {
                auto oPositionOnDownsampledImage = downsampledImageInfo.transformToCurrentImage( xResampled, yResampled, SamplingInformation::BorderMode::excluded);
                int y_ = int( oPositionOnDownsampledImage.y);
                int x_ = int( oPositionOnDownsampledImage.x);
                
                QCOMPARE(resampledImage[yResampled][xResampled], downsampledImage[y_][x_]);
                
                auto oEquivalentReferencePosition = downsampledImageInfo.transformToReferenceImage(oPositionOnDownsampledImage.x, oPositionOnDownsampledImage.y, SamplingInformation::BorderMode::excluded);
                QCOMPARE(oEquivalentReferencePosition,  DPoint( xResampled, yResampled ));
                
                if ( x_ == oPositionOnDownsampledImage.x &&  y_  == oPositionOnDownsampledImage.y)
                {
                    QCOMPARE(referenceImage[yResampled + offsetToReferenceImage.y][xResampled+offsetToReferenceImage.x], downsampledImage[y_][x_]);
                }
            
            }
        }
        
        
    }

}



QTEST_MAIN(TestResampleImage)
#include "testResampleImage.moc"
