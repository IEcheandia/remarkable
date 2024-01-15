#include <QTest>

#include "../startEndMisalignmentDetection.h"
#include "testUtilities.h"

class TestStartEndMisalignmentDetection : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void initTestCase();
    void testOnlyStartEdge(); // search only for start edge
    void testOnlyEndEdge(); // search only for end edge
    void test3VisibleEdgesOffset(); // similar to test2VisibleEdges but with offsetLeftRight > 0
   void test2VisibleEdges();
   void test1VisibleEdge();
   void testValidSequence_data();
   void testValidSequence();
private:
    TestCombinedImage m_oTestCombinedImage;
};

using namespace  precitec::image;
using precitec::geo2d::Rect;
typedef precitec::geo2d::StartEndInfo::ImageStateEvaluation ImageStateEvaluation;
typedef precitec::geo2d::StartEndInfo::ImageState ImageState;



using precitec::filter::start_end_detection::StartEndDetectionInSeam;

bool compareMisalignment(const StartEndDetectionInSeam::EdgeMisalignment & actual, 
                         const StartEndDetectionInSeam::EdgeMisalignment  & expected, double tolerance, bool mZeroNotOk = true)
{
    // support for non-horizontal lines not implement
    for (auto && rLine : {actual.m_leftEdge.m_edgeLocalPosition.line, actual.m_rightEdge.m_edgeLocalPosition.line, 
        expected.m_leftEdge.m_edgeLocalPosition.line, expected.m_rightEdge.m_edgeLocalPosition.line
    })
    {
        if (rLine.m != 0 && mZeroNotOk)
        {
            QWARN("assert(rLine.m == 0) failed");
            return false;
        }
    }
    
    auto yDiffActual =  actual.m_rightEdge.m_edgeLocalPosition.line.q - actual.m_leftEdge.m_edgeLocalPosition.line.q ;
    auto yDiffExpected =  expected.m_rightEdge.m_edgeLocalPosition.line.q  - expected.m_leftEdge.m_edgeLocalPosition.line.q ;
    
    auto contextDiffActual = actual.m_rightEdge.m_contextImage_mm - actual.m_leftEdge.m_contextImage_mm;
    auto contextDiffExpected = expected.m_rightEdge.m_contextImage_mm - expected.m_leftEdge.m_contextImage_mm;
    if (std::abs(yDiffActual - yDiffExpected ) > tolerance)
    {
        std::cout << "yDiffActual " << yDiffActual << " yDiffExpected " << yDiffExpected << std::endl;
    }
    return actual.m_rightEdge.m_edgeLocalPosition.mAppearance == expected.m_rightEdge.m_edgeLocalPosition.mAppearance
        && actual.m_leftEdge.m_edgeLocalPosition.mAppearance == expected.m_leftEdge.m_edgeLocalPosition.mAppearance 
        && contextDiffActual == contextDiffExpected
        && std::abs(yDiffActual - yDiffExpected ) <= tolerance;
}

void TestStartEndMisalignmentDetection::initTestCase()
{
    
    enum index{LeftEdgeStart, RightEdgeStart, LeftEdgeEnd, RightEdgeEnd};
    const std::array<FittedLine,4> globalEdges 
    {
        FittedLine{0.0, 130.0}, FittedLine{0.0, 150.0}, 
        FittedLine{0.0, 630.0}, FittedLine{0.0, 600.0}
    };

    int xLeft = 190;
    int xRight = 210;

    
    bool initialized = m_oTestCombinedImage.init(globalEdges[0],globalEdges[1],globalEdges[2],globalEdges[3],
        xLeft, xRight,
        400, 750, //combined image size
        20,150 // intensity
        );
    QVERIFY(initialized);
}

void TestStartEndMisalignmentDetection::testOnlyStartEdge()
{
    using precitec::filter::start_end_detection::BackgroundPosition;
    using precitec::filter::start_end_detection::EdgePositionInImage;
    using precitec::filter::start_end_detection::EdgePositionInSeam;
    using precitec::filter::start_end_detection::Appearance;

    StartEndDetectionInSeam test;
    test.setDirection(StartEndDetectionInSeam::Direction::fromBelow);
    test.updateSearchForEdges(1);

    const double trigger_mm = 1.1;

    double currentTransitionFromBG = -1;
    double currentTransitionFromMaterial = -1;

    int expectedYStartLeft = 102;
    int expectedYStartRight = 99;

    int expectedYEndLeft =  185;
    int expectedYEndRight = 180;

    auto getContext = [&](int imageNumber)
    {
        precitec::filter::start_end_detection::InputImageContext oContext;
        oContext.imageCounter = imageNumber;
        oContext.imagePosition_mm = trigger_mm*imageNumber;
        oContext.pixel_to_mm = 1.0;
        oContext.offsetX = 0;
        oContext.offsetY = 0;
        return oContext;
    };


    StartEndDetectionInSeam::EdgeMisalignment expectedMisaligmentStart{
        EdgePositionInSeam{EdgePositionInImage{Appearance::BackgroundOnBottom, {0.0, double(expectedYStartLeft)}}, getContext(2), true},
        EdgePositionInSeam{EdgePositionInImage{Appearance::BackgroundOnBottom, {0.0, double(expectedYStartRight)}},getContext(2), true}
    };

    StartEndDetectionInSeam::EdgeMisalignment expectedMisaligmentEnd{
        EdgePositionInSeam{EdgePositionInImage{Appearance::BackgroundOnTop, {0.0, double(expectedYEndLeft)}}, getContext(4), true},
        EdgePositionInSeam{EdgePositionInImage{Appearance::BackgroundOnTop, {0.0, double(expectedYEndRight)}}, getContext(4), true}
    };


    StartEndDetectionInSeam::EdgeMisalignment foundMisaligmentStart;
    StartEndDetectionInSeam::EdgeMisalignment foundMisaligmentEnd;

#ifndef NDEBUG
    std::cout << "TEST VISIBLE EDGES" << std::endl;
#endif

    int counter = 0;
    for (; counter < 2; counter++)
    {
#ifndef NDEBUG
    std::cout << "BLACK IMAGE" << std::endl;
#endif

        BImage blackImage = TestImage::createBackgroundImage();
        test.process( blackImage, counter, counter*trigger_mm ,1.0, 0,0, 1024);

        QVERIFY(test.getSeamState() != StartEndDetectionInSeam::SeamState::Invalid);

        auto misalignment =  test.computeEdgeMisalignment(true);
        QCOMPARE(misalignment.found(), false);
        misalignment =  test.computeEdgeMisalignment(false);
        QCOMPARE(misalignment.found(), false);
        auto info = test.getLastImageStartEndInfo(1);
        QVERIFY(info.isTopDark && info.isBottomDark);
        QCOMPARE(info.m_oImageStateEvaluation , ImageStateEvaluation::BackgroundBeforeStart);
        QCOMPARE(test.getTransitionFromBackground(),-1);
        QCOMPARE(test.getTransitionFromFullImage(), -1);
        currentTransitionFromBG = test.getTransitionFromBackground();
        currentTransitionFromMaterial = test.getTransitionFromFullImage();
    }

#ifndef NDEBUG
        std::cout << "IMAGE 2 EDGES START" << std::endl;
#endif
    {
        BImage imageWith2Edges = TestImage::createEdgesBackgroundOnBottom(expectedYStartLeft, expectedYStartRight);
        test.process( imageWith2Edges, counter, counter*trigger_mm,1.0, 0,0, 1024);

        QCOMPARE(test.getSeamState(), StartEndDetectionInSeam::SeamState::FullStartEdgeFound);

        auto misalignmentStart =  test.computeEdgeMisalignment(true);
        QVERIFY( compareMisalignment(misalignmentStart, expectedMisaligmentStart,1));
        foundMisaligmentStart = misalignmentStart;

        auto misalignmentEnd =  test.computeEdgeMisalignment(false);
        QCOMPARE(misalignmentEnd.found(), false);
        QVERIFY(test.getTransitionFromBackground() > 0);
        QCOMPARE(test.getTransitionFromFullImage(), -1);
        currentTransitionFromBG = test.getTransitionFromBackground();
        currentTransitionFromMaterial = test.getTransitionFromFullImage();

        int offset = 0;
        auto info_0 = test.getLastImageStartEndInfo(offset);
            QCOMPARE(info_0.isTopDark, false);
            QCOMPARE(info_0.isBottomDark, true);
            QCOMPARE(info_0.isCropped, true);
            QCOMPARE(info_0.m_oStartValidRangeY,0);
            QVERIFY(info_0.m_oEndValidRangeY < imageWith2Edges.height() -1 );
            QCOMPARE(info_0.m_oImageStateEvaluation , ImageStateEvaluation::StartEdge);
            QCOMPARE(info_0.m_oImageState, ImageState::FullEdgeVisible);

        offset = 2;
        auto info_1 = test.getLastImageStartEndInfo(offset);
            QCOMPARE(info_1.isTopDark, false);
            QCOMPARE(info_1.isBottomDark, true);
            QCOMPARE(info_1.isCropped, true);
            QCOMPARE(info_1.m_oStartValidRangeY, 0);
            QCOMPARE(info_1.m_oEndValidRangeY, info_0.m_oEndValidRangeY - offset);
            QCOMPARE(info_1.m_oImageStateEvaluation , ImageStateEvaluation::StartEdge);
            QCOMPARE(info_1.m_oImageState , ImageState::FullEdgeVisible);
    }
    counter ++;

#ifndef NDEBUG
    std::cout << "FULL IMAGE" << std::endl;
#endif
    {
        BImage fullImage = TestImage::createFullImage();

        test.process( fullImage, counter, counter*trigger_mm,1.0, 0,0, 1024);

        QCOMPARE(test.getSeamState(), StartEndDetectionInSeam::SeamState::FullStartEdgeFound_EndMissing);

        auto misalignmentStart =  test.computeEdgeMisalignment(true);
        QVERIFY( compareMisalignment(misalignmentStart, foundMisaligmentStart,0));

        auto misalignmentEnd =  test.computeEdgeMisalignment(false);
        QCOMPARE(misalignmentEnd.found(), false);
        QCOMPARE(test.getTransitionFromBackground(), currentTransitionFromBG); //the same as found previously
        QCOMPARE(test.getTransitionFromFullImage(), -1);
        currentTransitionFromBG = test.getTransitionFromBackground();
        currentTransitionFromMaterial = test.getTransitionFromFullImage();

        int offset = 1;
        auto info = test.getLastImageStartEndInfo(offset);
            QVERIFY(!info.isTopDark && !info.isBottomDark);
            QCOMPARE(info.isCropped, false);
            QCOMPARE(info.m_oStartValidRangeY,0);
            QCOMPARE(info.m_oEndValidRangeY, fullImage.height() -1 );
            QCOMPARE(info.m_oImageStateEvaluation , ImageStateEvaluation::OnlyMaterial);
    }
    counter ++;

#ifndef NDEBUG
    std::cout << "IMAGE 2 EDGES" << counter << std::endl;
#endif
    {
        BImage imageWith2Edges = TestImage::createEdgesBackgroundOnTop(expectedYEndLeft, expectedYEndRight);
        test.process(imageWith2Edges, counter, counter*trigger_mm, 1.0, 0, 0, 1024);

        QCOMPARE(test.getSeamState(), StartEndDetectionInSeam::SeamState::FullStartEdgeFound_EndMissing);

        auto misalignmentStart = test.computeEdgeMisalignment(true);
        QVERIFY(compareMisalignment(misalignmentStart, foundMisaligmentStart,0));

        auto misalignmentEnd = test.computeEdgeMisalignment(false);
        QCOMPARE(misalignmentEnd.found(), false);
        QCOMPARE(test.getTransitionFromBackground(), currentTransitionFromBG); //the same as found previously
        QCOMPARE(test.getTransitionFromFullImage(), -1);
        currentTransitionFromBG = test.getTransitionFromBackground();
        currentTransitionFromMaterial = test.getTransitionFromFullImage();

        int offset = 0;
        auto info_0 = test.getLastImageStartEndInfo(offset);
            QCOMPARE(info_0.isTopDark, true);
            QCOMPARE(info_0.isBottomDark, false);
            QCOMPARE(info_0.isCropped, true);
            QVERIFY(info_0.m_oStartValidRangeY>0);
            QCOMPARE(info_0.m_oEndValidRangeY ,  imageWith2Edges.height() -1 );

        offset = 2;
        auto info_1 = test.getLastImageStartEndInfo(offset);
            QCOMPARE(info_1.isTopDark, true);
            QCOMPARE(info_1.isBottomDark, false);
            QCOMPARE(info_1.isCropped, true);
            QCOMPARE(info_1.m_oStartValidRangeY,  info_0.m_oStartValidRangeY + offset);
            QCOMPARE(info_1.m_oEndValidRangeY ,  imageWith2Edges.height() -1 );
    }
    counter ++;

#ifndef NDEBUG
    std::cout << "BLACK IMAGE" << std::endl;
#endif
    {
        BImage blackImage = TestImage::createBackgroundImage();

        test.process(blackImage, counter, counter*trigger_mm, 1.0, 0, 0, 1024);

        QCOMPARE(test.getSeamState(), StartEndDetectionInSeam::SeamState::FullStartEdgeFound_EndMissing);

        auto misalignmentStart = test.computeEdgeMisalignment(true);
        QVERIFY(compareMisalignment(misalignmentStart, foundMisaligmentStart,0));

        auto misalignmentEnd = test.computeEdgeMisalignment(false);
        QCOMPARE(misalignmentEnd.found(), false);
        QCOMPARE(test.getTransitionFromBackground(), currentTransitionFromBG); //the same as found previously
        QVERIFY(test.getTransitionFromFullImage() > 0);
        currentTransitionFromBG = test.getTransitionFromBackground();
        currentTransitionFromMaterial = test.getTransitionFromFullImage();

        int offset = 1;
        auto info = test.getLastImageStartEndInfo(offset);
            QVERIFY(info.isTopDark && info.isBottomDark);
            QCOMPARE(info.isCropped, true);
            QCOMPARE(info.m_oStartValidRangeY, -1);
            QCOMPARE(info.m_oEndValidRangeY, -1);
            QCOMPARE(info.m_oImageStateEvaluation, ImageStateEvaluation::BackgroundAfterEnd);
    }
    counter ++;

#ifndef NDEBUG
    std::cout << "FULL IMAGE (UNEXPECTED)" << std::endl;
#endif
    {
        BImage fullImage = TestImage::createFullImage();
        test.process( fullImage, counter, counter*trigger_mm,1.0, 0,0, 1024);

        QCOMPARE(test.getSeamState(), StartEndDetectionInSeam::SeamState::FullStartEdgeFound_EndMissing);

        auto misalignmentStart =  test.computeEdgeMisalignment(true);
        QVERIFY( compareMisalignment(misalignmentStart, foundMisaligmentStart,0));

        auto misalignmentEnd =  test.computeEdgeMisalignment(false);
        QCOMPARE(misalignmentEnd.found(), false);

        QCOMPARE(test.getTransitionFromBackground(),currentTransitionFromBG);
        QCOMPARE(test.getTransitionFromFullImage(), currentTransitionFromMaterial);
        currentTransitionFromBG = test.getTransitionFromBackground();
        currentTransitionFromMaterial = test.getTransitionFromFullImage();


        int offset = 1;
        auto info = test.getLastImageStartEndInfo(offset);
            QVERIFY(!info.isTopDark && !info.isBottomDark);
            QCOMPARE(info.isCropped, false);
            QCOMPARE(info.m_oStartValidRangeY,0);
            QCOMPARE(info.m_oEndValidRangeY, fullImage.height() -1 );
    }
    counter ++;

#ifndef NDEBUG
    std::cout << "IMAGE 2 EDGES (UNEXPECTED)" << std::endl;
#endif
    {
        int YRight = 50;
        int YLeft = 100;
        BImage imageWith2Edges = TestImage::createEdgesBackgroundOnBottom(YLeft, YRight);
        test.process( imageWith2Edges, counter, counter*trigger_mm,1.0, 0,0, 1024);

        QCOMPARE(test.getSeamState(), StartEndDetectionInSeam::SeamState::FullStartEdgeFound_EndMissing);

        auto misalignmentStart =  test.computeEdgeMisalignment(true);
        QVERIFY( compareMisalignment(misalignmentStart, foundMisaligmentStart,0));

        auto misalignmentEnd =  test.computeEdgeMisalignment(false);
        QCOMPARE(misalignmentEnd.found(), false);

        QCOMPARE(test.getTransitionFromBackground(),currentTransitionFromBG);
        QCOMPARE(test.getTransitionFromFullImage(), currentTransitionFromMaterial);
        currentTransitionFromBG = test.getTransitionFromBackground();
        currentTransitionFromMaterial = test.getTransitionFromFullImage();

        int offset = 0;
        auto info_0 = test.getLastImageStartEndInfo(offset);
            QCOMPARE(info_0.isTopDark, false);
            QCOMPARE(info_0.isBottomDark, true);
            QCOMPARE(info_0.isCropped, true);
            QCOMPARE(info_0.m_oStartValidRangeY,0);
            QVERIFY(info_0.m_oEndValidRangeY < imageWith2Edges.height() -1 );
            QCOMPARE(info_0.m_oImageStateEvaluation , ImageStateEvaluation::Unknown);

        offset = 2;
        auto info_1 = test.getLastImageStartEndInfo(offset);
            QCOMPARE(info_1.isTopDark, false);
            QCOMPARE(info_1.isBottomDark, true);
            QCOMPARE(info_1.isCropped, true);
            QCOMPARE(info_1.m_oStartValidRangeY, 0);
            QCOMPARE(info_1.m_oEndValidRangeY, info_0.m_oEndValidRangeY - offset);
    }
}

void TestStartEndMisalignmentDetection::testOnlyEndEdge()
{
    using precitec::filter::start_end_detection::BackgroundPosition;
    using precitec::filter::start_end_detection::EdgePositionInImage;
    using precitec::filter::start_end_detection::EdgePositionInSeam;
    using precitec::filter::start_end_detection::Appearance;

    StartEndDetectionInSeam test;
    test.setDirection(StartEndDetectionInSeam::Direction::fromBelow);
    test.updateSearchForEdges(2);

    const double trigger_mm = 1.1;

    double currentTransitionFromBG = -1;
    double currentTransitionFromMaterial = -1;

    int expectedYStartLeft = 102;
    int expectedYStartRight = 99;

    int expectedYEndLeft =  185;
    int expectedYEndRight = 180;

    auto getContext = [&](int imageNumber)
    {
        precitec::filter::start_end_detection::InputImageContext oContext;
        oContext.imageCounter = imageNumber;
        oContext.imagePosition_mm = trigger_mm*imageNumber;
        oContext.pixel_to_mm = 1.0;
        oContext.offsetX = 0;
        oContext.offsetY = 0;
        return oContext;
    };


    StartEndDetectionInSeam::EdgeMisalignment expectedMisaligmentStart{
        EdgePositionInSeam{EdgePositionInImage{Appearance::BackgroundOnBottom, {0.0, double(expectedYStartLeft)}}, getContext(2), true},
        EdgePositionInSeam{EdgePositionInImage{Appearance::BackgroundOnBottom, {0.0, double(expectedYStartRight)}},getContext(2), true}
    };

    StartEndDetectionInSeam::EdgeMisalignment expectedMisaligmentEnd{
        EdgePositionInSeam{EdgePositionInImage{Appearance::BackgroundOnTop, {0.0, double(expectedYEndLeft)}}, getContext(4), true},
        EdgePositionInSeam{EdgePositionInImage{Appearance::BackgroundOnTop, {0.0, double(expectedYEndRight)}}, getContext(4), true}
    };


    StartEndDetectionInSeam::EdgeMisalignment foundMisaligmentStart;
    StartEndDetectionInSeam::EdgeMisalignment foundMisaligmentEnd;

#ifndef NDEBUG
    std::cout << "TEST VISIBLE EDGES" << std::endl;
#endif

    int counter = 0;
    for (; counter < 2; counter++)
    {
#ifndef NDEBUG
    std::cout << "BLACK IMAGE" << std::endl;
#endif

        BImage blackImage = TestImage::createBackgroundImage();
        test.process( blackImage, counter, counter*trigger_mm ,1.0, 0,0, 1024);

        QVERIFY(test.getSeamState() != StartEndDetectionInSeam::SeamState::Invalid);

        auto misalignment =  test.computeEdgeMisalignment(true);
        QCOMPARE(misalignment.found(), false);
        misalignment =  test.computeEdgeMisalignment(false);
        QCOMPARE(misalignment.found(), false);
        auto info = test.getLastImageStartEndInfo(1);
        QVERIFY(info.isTopDark && info.isBottomDark);
        QCOMPARE(info.m_oImageStateEvaluation , ImageStateEvaluation::BackgroundBeforeStart);
        QCOMPARE(test.getTransitionFromBackground(),-1);
        QCOMPARE(test.getTransitionFromFullImage(), -1);
        currentTransitionFromBG = test.getTransitionFromBackground();
        currentTransitionFromMaterial = test.getTransitionFromFullImage();
    }

#ifndef NDEBUG
        std::cout << "IMAGE 2 EDGES START" << std::endl;
#endif
    {
        BImage imageWith2Edges = TestImage::createEdgesBackgroundOnBottom(expectedYStartLeft, expectedYStartRight);
        test.process( imageWith2Edges, counter, counter*trigger_mm,1.0, 0,0, 1024);

        QCOMPARE(test.getSeamState(), StartEndDetectionInSeam::SeamState::WaitingFirstEndEdge);

        auto misalignmentStart =  test.computeEdgeMisalignment(true);
        QCOMPARE(misalignmentStart.found(), false);
        auto misalignmentEnd =  test.computeEdgeMisalignment(false);
        QCOMPARE(misalignmentEnd.found(), false);
        QCOMPARE(test.getTransitionFromBackground(), -1);
        QCOMPARE(test.getTransitionFromFullImage(), -1);
        currentTransitionFromBG = test.getTransitionFromBackground();
        currentTransitionFromMaterial = test.getTransitionFromFullImage();
    }
    counter ++;

#ifndef NDEBUG
    std::cout << "FULL IMAGE" << std::endl;
#endif
    {
        BImage fullImage = TestImage::createFullImage();

        test.process( fullImage, counter, counter*trigger_mm,1.0, 0,0, 1024);

        QCOMPARE(test.getSeamState(), StartEndDetectionInSeam::SeamState::FullImageFound_StartMissing);

        auto misalignmentStart =  test.computeEdgeMisalignment(true);
        QCOMPARE(misalignmentStart.found(), false);
        auto misalignmentEnd =  test.computeEdgeMisalignment(false);
        QCOMPARE(misalignmentEnd.found(), false);
        QVERIFY(test.getTransitionFromBackground() > 0);
        QCOMPARE(test.getTransitionFromFullImage(), -1);
        currentTransitionFromBG = test.getTransitionFromBackground();
        currentTransitionFromMaterial = test.getTransitionFromFullImage();

        int offset = 1;
        auto info = test.getLastImageStartEndInfo(offset);
            QVERIFY(!info.isTopDark && !info.isBottomDark);
            QCOMPARE(info.isCropped, false);
            QCOMPARE(info.m_oStartValidRangeY,0);
            QCOMPARE(info.m_oEndValidRangeY, fullImage.height() -1 );
            QCOMPARE(info.m_oImageStateEvaluation , ImageStateEvaluation::OnlyMaterial);
    }
    counter ++;

#ifndef NDEBUG
    std::cout << "IMAGE 2 EDGES" << counter << std::endl;
#endif
    {
        BImage imageWith2Edges = TestImage::createEdgesBackgroundOnTop(expectedYEndLeft, expectedYEndRight);
        test.process(imageWith2Edges, counter, counter*trigger_mm, 1.0, 0, 0, 1024);

        QCOMPARE(test.getSeamState(), StartEndDetectionInSeam::SeamState::SecondEndEdgeFound_StartMissing);

        auto misalignmentStart = test.computeEdgeMisalignment(true);
        QCOMPARE(misalignmentStart.found(), false);
        auto misalignmentEnd = test.computeEdgeMisalignment(false);
        QCOMPARE(misalignmentEnd.found(), true);
        QVERIFY(compareMisalignment(misalignmentEnd, expectedMisaligmentEnd, 0));
        foundMisaligmentEnd = misalignmentEnd;
        QCOMPARE(test.getTransitionFromBackground(), currentTransitionFromBG); //the same as found previously
        QVERIFY(test.getTransitionFromFullImage() > 0);
        currentTransitionFromBG = test.getTransitionFromBackground();
        currentTransitionFromMaterial = test.getTransitionFromFullImage();

        int offset = 0;
        auto info_0 = test.getLastImageStartEndInfo(offset);
            QCOMPARE(info_0.isTopDark, true);
            QCOMPARE(info_0.isBottomDark, false);
            QCOMPARE(info_0.isCropped, true);
            QVERIFY(info_0.m_oStartValidRangeY>0);
            QCOMPARE(info_0.m_oEndValidRangeY ,  imageWith2Edges.height() -1 );

        offset = 2;
        auto info_1 = test.getLastImageStartEndInfo(offset);
            QCOMPARE(info_1.isTopDark, true);
            QCOMPARE(info_1.isBottomDark, false);
            QCOMPARE(info_1.isCropped, true);
            QCOMPARE(info_1.m_oStartValidRangeY,  info_0.m_oStartValidRangeY + offset);
            QCOMPARE(info_1.m_oEndValidRangeY ,  imageWith2Edges.height() -1 );
    }
    counter ++;

#ifndef NDEBUG
    std::cout << "BLACK IMAGE" << std::endl;
#endif
    {
        BImage blackImage = TestImage::createBackgroundImage();

        test.process(blackImage, counter, counter*trigger_mm, 1.0, 0, 0, 1024);

        QCOMPARE(test.getSeamState(), StartEndDetectionInSeam::SeamState::EndBackgroundImageFound_StartMissing);

        auto misalignmentStart = test.computeEdgeMisalignment(true);
        QCOMPARE(misalignmentStart.found(), false);
        auto misalignmentEnd = test.computeEdgeMisalignment(false);
        QCOMPARE(misalignmentEnd.found(), true);
        QVERIFY(compareMisalignment(misalignmentEnd, foundMisaligmentEnd, 0));
        QCOMPARE(test.getTransitionFromBackground(), currentTransitionFromBG); //the same as found previously
        QCOMPARE(test.getTransitionFromFullImage(), currentTransitionFromMaterial);
        currentTransitionFromBG = test.getTransitionFromBackground();
        currentTransitionFromMaterial = test.getTransitionFromFullImage();

        int offset = 1;
        auto info = test.getLastImageStartEndInfo(offset);
            QVERIFY(info.isTopDark && info.isBottomDark);
            QCOMPARE(info.isCropped, true);
            QCOMPARE(info.m_oStartValidRangeY, -1);
            QCOMPARE(info.m_oEndValidRangeY, -1);
            QCOMPARE(info.m_oImageStateEvaluation, ImageStateEvaluation::BackgroundAfterEnd);
    }
    counter ++;

#ifndef NDEBUG
    std::cout << "FULL IMAGE (UNEXPECTED)" << std::endl;
#endif
    {
        BImage fullImage = TestImage::createFullImage();
        test.process( fullImage, counter, counter*trigger_mm,1.0, 0,0, 1024);

        QCOMPARE(test.getSeamState(), StartEndDetectionInSeam::SeamState::EndBackgroundImageFound_StartMissing);

        auto misalignmentStart = test.computeEdgeMisalignment(true);
        QCOMPARE(misalignmentStart.found(), false);
        auto misalignmentEnd = test.computeEdgeMisalignment(false);
        QCOMPARE(misalignmentEnd.found(), true);
        QVERIFY(compareMisalignment(misalignmentEnd, foundMisaligmentEnd, 0));
        QCOMPARE(test.getTransitionFromBackground(), currentTransitionFromBG); //the same as found previously
        QCOMPARE(test.getTransitionFromFullImage(), currentTransitionFromMaterial);
        currentTransitionFromBG = test.getTransitionFromBackground();
        currentTransitionFromMaterial = test.getTransitionFromFullImage();


        int offset = 1;
        auto info = test.getLastImageStartEndInfo(offset);
            QVERIFY(!info.isTopDark && !info.isBottomDark);
            QCOMPARE(info.isCropped, false);
            QCOMPARE(info.m_oStartValidRangeY,0);
            QCOMPARE(info.m_oEndValidRangeY, fullImage.height() -1 );
    }
    counter ++;

#ifndef NDEBUG
    std::cout << "IMAGE 2 EDGES (UNEXPECTED)" << std::endl;
#endif
    {
        int YRight = 50;
        int YLeft = 100;
        BImage imageWith2Edges = TestImage::createEdgesBackgroundOnBottom(YLeft, YRight);
        test.process( imageWith2Edges, counter, counter*trigger_mm,1.0, 0,0, 1024);

        QCOMPARE(test.getSeamState(), StartEndDetectionInSeam::SeamState::EndBackgroundImageFound_StartMissing);

        auto misalignmentStart = test.computeEdgeMisalignment(true);
        QCOMPARE(misalignmentStart.found(), false);
        auto misalignmentEnd = test.computeEdgeMisalignment(false);
        QCOMPARE(misalignmentEnd.found(), true);
        QVERIFY(compareMisalignment(misalignmentEnd, foundMisaligmentEnd, 0));
        QCOMPARE(test.getTransitionFromBackground(), currentTransitionFromBG); //the same as found previously
        QCOMPARE(test.getTransitionFromFullImage(), currentTransitionFromMaterial);
        currentTransitionFromBG = test.getTransitionFromBackground();
        currentTransitionFromMaterial = test.getTransitionFromFullImage();

        int offset = 0;
        auto info_0 = test.getLastImageStartEndInfo(offset);
            QCOMPARE(info_0.isTopDark, false);
            QCOMPARE(info_0.isBottomDark, true);
            QCOMPARE(info_0.isCropped, true);
            QCOMPARE(info_0.m_oStartValidRangeY,0);
            QVERIFY(info_0.m_oEndValidRangeY < imageWith2Edges.height() -1 );
            QCOMPARE(info_0.m_oImageStateEvaluation , ImageStateEvaluation::Unknown);

        offset = 2;
        auto info_1 = test.getLastImageStartEndInfo(offset);
            QCOMPARE(info_1.isTopDark, false);
            QCOMPARE(info_1.isBottomDark, true);
            QCOMPARE(info_1.isCropped, true);
            QCOMPARE(info_1.m_oStartValidRangeY, 0);
            QCOMPARE(info_1.m_oEndValidRangeY, info_0.m_oEndValidRangeY - offset);
    }
}

void TestStartEndMisalignmentDetection::test3VisibleEdgesOffset()
{
    using precitec::filter::start_end_detection::BackgroundPosition;
    using precitec::filter::start_end_detection::EdgePositionInImage;
    using precitec::filter::start_end_detection::EdgePositionInSeam;
    using precitec::filter::start_end_detection::Appearance;


    const double trigger_mm = 1.1;

    StartEndDetectionInSeam test;

    int expectedYStartLeft = 102;
    int expectedYStartRight = 99;

    int expectedYEndLeft =  185;
    int expectedYEndRight = 180;

    auto offsetLeftRight = 50.;
    test.updateOffsetLeftRight(offsetLeftRight);

    auto getContext = [&](int imageNumber)
    {
        precitec::filter::start_end_detection::InputImageContext oContext;
        oContext.imageCounter = imageNumber;
        oContext.imagePosition_mm = trigger_mm*imageNumber;
        oContext.pixel_to_mm = 1.0;
        oContext.offsetX = 0;
        oContext.offsetY = 0;
        return oContext;
    };

    double currentTransitionFromBG = -1;
    double currentTransitionFromMaterial = -1;


    StartEndDetectionInSeam::EdgeMisalignment expectedMisaligmentStart{
        EdgePositionInSeam{EdgePositionInImage{Appearance::BackgroundOnBottom, {0., double(expectedYStartLeft)}}, getContext(2), true},
        EdgePositionInSeam{EdgePositionInImage{Appearance::BackgroundOnBottom, {0., double(expectedYStartRight)}},getContext(2), true}
    };

    StartEndDetectionInSeam::EdgeMisalignment expectedMisaligmentStartHorizontal{
        EdgePositionInSeam{EdgePositionInImage{Appearance::BackgroundOnBottom, {0., double(expectedYStartRight)}}, getContext(2), true},
        EdgePositionInSeam{EdgePositionInImage{Appearance::BackgroundOnBottom, {0., double(expectedYStartRight)}},getContext(2), true}
    };

    StartEndDetectionInSeam::EdgeMisalignment expectedMisaligmentEnd{
        EdgePositionInSeam{EdgePositionInImage{Appearance::BackgroundOnTop, {0., double(expectedYEndLeft)}}, getContext(4), true},
        EdgePositionInSeam{EdgePositionInImage{Appearance::BackgroundOnTop, {0., double(expectedYEndRight)}}, getContext(4), true}
    };


    StartEndDetectionInSeam::EdgeMisalignment foundMisaligmentStart;
    StartEndDetectionInSeam::EdgeMisalignment foundMisaligmentEnd;

#ifndef NDEBUG
    std::cout << "TEST 3" << std::endl;
#endif

    int counter = 0;
    for (; counter < 2; counter++)
    {

#ifndef NDEBUG
    std::cout << "BLACK IMAGE" << std::endl;
#endif

        BImage blackImage = TestImage::createBackgroundImage();
        test.process( blackImage, counter, counter*trigger_mm ,1.0, 0,0, 1024);

        QVERIFY(test.getSeamState() != StartEndDetectionInSeam::SeamState::Invalid);

        auto misalignment =  test.computeEdgeMisalignment(true);
        QCOMPARE(misalignment.found(), false);
        misalignment =  test.computeEdgeMisalignment(false);
        QCOMPARE(misalignment.found(), false);
        auto info = test.getLastImageStartEndInfo(1);
        QVERIFY(info.isTopDark && info.isBottomDark);
        QCOMPARE(info.m_oImageStateEvaluation , ImageStateEvaluation::BackgroundBeforeStart);
        QCOMPARE(test.getTransitionFromBackground(),-1);
        QCOMPARE(test.getTransitionFromFullImage(), -1);
        currentTransitionFromBG = test.getTransitionFromBackground();
        currentTransitionFromMaterial = test.getTransitionFromFullImage();

        //TODO: modify startEndDynamicRoiSimple for empty images? or return a default roi?
    }

    {
#ifndef NDEBUG
        std::cout << "IMAGE 2 EDGES START" << std::endl;
#endif
        BImage imageWith2Edges = TestImage::createEdgesBackgroundOnBottom(expectedYStartLeft, expectedYStartRight, 50);
        test.process( imageWith2Edges, counter, counter*trigger_mm,1.0, 0,0, 1024);

        QCOMPARE(test.getSeamState(), StartEndDetectionInSeam::SeamState::FullStartEdgeFound);

        auto misalignmentStart =  test.computeEdgeMisalignment(true);
        QVERIFY( compareMisalignment(misalignmentStart, expectedMisaligmentStartHorizontal, 1, false));
        foundMisaligmentStart = misalignmentStart;

        auto misalignmentEnd =  test.computeEdgeMisalignment(false);
        QCOMPARE(misalignmentEnd.found(), false);
        QVERIFY(test.getTransitionFromBackground() > 0);
        QCOMPARE(test.getTransitionFromFullImage(), -1);
        currentTransitionFromBG = test.getTransitionFromBackground();
        currentTransitionFromMaterial = test.getTransitionFromFullImage();

        int offset = 0;
        auto info_0 = test.getLastImageStartEndInfo(offset);
            QCOMPARE(info_0.isTopDark, false);
            QCOMPARE(info_0.isBottomDark, true);
            QCOMPARE(info_0.isCropped, true);
            QCOMPARE(info_0.m_oStartValidRangeY,0);
            QVERIFY(info_0.m_oEndValidRangeY < imageWith2Edges.height() -1 );
            QCOMPARE(info_0.m_oImageStateEvaluation , ImageStateEvaluation::StartEdge);
            QCOMPARE(info_0.m_oImageState, ImageState::FullEdgeVisible);

            //TODO QCOMPARE(info_0.m_oEndValidRangeY, std::min(expectedYStartLeft, expectedYStartRight) );


        offset = 2;
        auto info_1 = test.getLastImageStartEndInfo(offset);
            QCOMPARE(info_1.isTopDark, false);
            QCOMPARE(info_1.isBottomDark, true);
            QCOMPARE(info_1.isCropped, true);
            QCOMPARE(info_1.m_oStartValidRangeY, 0);
            QCOMPARE(info_1.m_oEndValidRangeY, info_0.m_oEndValidRangeY - offset);
            QCOMPARE(info_1.m_oImageStateEvaluation , ImageStateEvaluation::StartEdge);
            QCOMPARE(info_1.m_oImageState , ImageState::FullEdgeVisible);

    }
    counter ++;

    {
        BImage fullImage = TestImage::createFullImage();

#ifndef NDEBUG
    std::cout << "FULL IMAGE" << std::endl;
#endif

        test.process( fullImage, counter, counter*trigger_mm,1.0, 0,0, 1024);

        QCOMPARE(test.getSeamState(), StartEndDetectionInSeam::SeamState::FullImageFound);

        auto misalignmentStart =  test.computeEdgeMisalignment(true);
        QVERIFY( compareMisalignment(misalignmentStart, foundMisaligmentStart,0));

        auto misalignmentEnd =  test.computeEdgeMisalignment(false);
        QCOMPARE(misalignmentEnd.found(), false);
        QCOMPARE(test.getTransitionFromBackground(), currentTransitionFromBG); //the same as found previously
        QCOMPARE(test.getTransitionFromFullImage(), -1);
        currentTransitionFromBG = test.getTransitionFromBackground();
        currentTransitionFromMaterial = test.getTransitionFromFullImage();

        int offset = 1;
        auto info = test.getLastImageStartEndInfo(offset);
            QVERIFY(!info.isTopDark && !info.isBottomDark);
            QCOMPARE(info.isCropped, false);
            QCOMPARE(info.m_oStartValidRangeY,0);
            QCOMPARE(info.m_oEndValidRangeY, fullImage.height() -1 );
            QCOMPARE(info.m_oImageStateEvaluation , ImageStateEvaluation::OnlyMaterial);
    }
    counter ++;

    {

#ifndef NDEBUG
    std::cout << "IMAGE 2 EDGES " << counter << std::endl;
#endif
        BImage imageWith2Edges = TestImage::createEdgesBackgroundOnTop(expectedYEndLeft, expectedYEndRight);
        test.process( imageWith2Edges, counter, counter*trigger_mm,1.0, 0,0, 1024);

        QCOMPARE(test.getSeamState(), StartEndDetectionInSeam::SeamState::SecondEndEdgeFound);

        auto misalignmentStart =  test.computeEdgeMisalignment(true);
        QVERIFY( compareMisalignment(misalignmentStart, foundMisaligmentStart,0));

        auto misalignmentEnd =  test.computeEdgeMisalignment(false);
        QVERIFY( compareMisalignment(misalignmentEnd, expectedMisaligmentEnd,1));
        foundMisaligmentEnd = misalignmentEnd;

        QCOMPARE(test.getTransitionFromBackground(), currentTransitionFromBG);
        QVERIFY(test.getTransitionFromFullImage() > 0);
        currentTransitionFromBG = test.getTransitionFromBackground();
        currentTransitionFromMaterial = test.getTransitionFromFullImage();

        int offset = 0;
        auto info_0 = test.getLastImageStartEndInfo(offset);
            QCOMPARE(info_0.isTopDark, true);
            QCOMPARE(info_0.isBottomDark, false);
            QCOMPARE(info_0.isCropped, true);
            QVERIFY(info_0.m_oStartValidRangeY > 0);
            QCOMPARE(info_0.m_oEndValidRangeY , imageWith2Edges.height() -1 );
            QCOMPARE(info_0.m_oImageStateEvaluation , ImageStateEvaluation::EndEdge);
            QCOMPARE(info_0.m_oImageState , ImageState::FullEdgeVisible);

        offset = 2;
        auto info_1 = test.getLastImageStartEndInfo(offset);
            QCOMPARE(info_1.isTopDark, true);
            QCOMPARE(info_1.isBottomDark, false);
            QCOMPARE(info_1.isCropped, true);
            QCOMPARE(info_1.m_oStartValidRangeY, info_0.m_oStartValidRangeY + offset);
            QCOMPARE(info_1.m_oEndValidRangeY , imageWith2Edges.height() -1 );

    }
    counter ++;

    {
        BImage blackImage = TestImage::createBackgroundImage();

        #ifndef NDEBUG
        std::cout << "BLACK IMAGE" << std::endl;
        #endif

        test.process( blackImage, counter, counter*trigger_mm,1.0, 0,0, 1024);

        QCOMPARE(test.getSeamState(), StartEndDetectionInSeam::SeamState::EndBackgroundImageFound);

        auto misalignmentStart =  test.computeEdgeMisalignment(true);
        QVERIFY( compareMisalignment(misalignmentStart, foundMisaligmentStart,0));

        auto misalignmentEnd =  test.computeEdgeMisalignment(false);
        QVERIFY( compareMisalignment(misalignmentEnd, foundMisaligmentEnd,0));

        QCOMPARE(test.getTransitionFromBackground(),currentTransitionFromBG);
        QCOMPARE(test.getTransitionFromFullImage(), currentTransitionFromMaterial);
        currentTransitionFromBG = test.getTransitionFromBackground();
        currentTransitionFromMaterial = test.getTransitionFromFullImage();

        auto info = test.getLastImageStartEndInfo(1);
        //TODO: modify startEndDynamicRoiSimple for empty images? or return a default roi?
        QVERIFY(info.isTopDark && info.isBottomDark);
        QCOMPARE(info.m_oImageStateEvaluation , ImageStateEvaluation::BackgroundAfterEnd);
    }
    counter ++;


    {
#ifndef NDEBUG
    std::cout << "FULL IMAGE (UNEXPECTED)" << std::endl;
#endif

        BImage fullImage = TestImage::createFullImage();
        test.process( fullImage, counter, counter*trigger_mm,1.0, 0,0, 1024);

        QCOMPARE(test.getSeamState(), StartEndDetectionInSeam::SeamState::EndBackgroundImageFound);

        auto misalignmentStart =  test.computeEdgeMisalignment(true);
        QVERIFY( compareMisalignment(misalignmentStart, foundMisaligmentStart,0));

        auto misalignmentEnd =  test.computeEdgeMisalignment(false);
        QVERIFY( compareMisalignment(misalignmentEnd, foundMisaligmentEnd,0));

        QCOMPARE(test.getTransitionFromBackground(),currentTransitionFromBG);
        QCOMPARE(test.getTransitionFromFullImage(), currentTransitionFromMaterial);
        currentTransitionFromBG = test.getTransitionFromBackground();
        currentTransitionFromMaterial = test.getTransitionFromFullImage();


        int offset = 1;
        auto info = test.getLastImageStartEndInfo(offset);
            QVERIFY(!info.isTopDark && !info.isBottomDark);
            QCOMPARE(info.isCropped, false);
            QCOMPARE(info.m_oStartValidRangeY,0);
            QCOMPARE(info.m_oEndValidRangeY, fullImage.height() -1 );
    }
    counter ++;

    {

#ifndef NDEBUG
    std::cout << "IMAGE 2 EDGES (UNEXPECTED)" << std::endl;
#endif

        int YRight = 50;
        int YLeft = 100;
        BImage imageWith2Edges = TestImage::createEdgesBackgroundOnBottom(YLeft, YRight);
        test.process( imageWith2Edges, counter, counter*trigger_mm,1.0, 0,0, 1024);

        QCOMPARE(test.getSeamState(), StartEndDetectionInSeam::SeamState::EndBackgroundImageFound);

        auto misalignmentStart =  test.computeEdgeMisalignment(true);
        QVERIFY( compareMisalignment(misalignmentStart, foundMisaligmentStart,0));

        auto misalignmentEnd =  test.computeEdgeMisalignment(false);
        QVERIFY( compareMisalignment(misalignmentEnd, foundMisaligmentEnd,0));


        QCOMPARE(test.getTransitionFromBackground(),currentTransitionFromBG);
        QCOMPARE(test.getTransitionFromFullImage(), currentTransitionFromMaterial);
        currentTransitionFromBG = test.getTransitionFromBackground();
        currentTransitionFromMaterial = test.getTransitionFromFullImage();


        int offset = 0;
        auto info_0 = test.getLastImageStartEndInfo(offset);
            QCOMPARE(info_0.isTopDark, false);
            QCOMPARE(info_0.isBottomDark, true);
            QCOMPARE(info_0.isCropped, true);
            QCOMPARE(info_0.m_oStartValidRangeY,0);
            //TODO QCOMPARE(info_0.m_oEndValidRangeY, std::min(expectedYStartLeft, expectedYStartRight) );
            QVERIFY(info_0.m_oEndValidRangeY < imageWith2Edges.height() -1 );
            QCOMPARE(info_0.m_oImageStateEvaluation , ImageStateEvaluation::Unknown);

        offset = 2;
        auto info_1 = test.getLastImageStartEndInfo(offset);
            QCOMPARE(info_1.isTopDark, false);
            QCOMPARE(info_1.isBottomDark, true);
            QCOMPARE(info_1.isCropped, true);
            QCOMPARE(info_1.m_oStartValidRangeY, 0);
            QCOMPARE(info_1.m_oEndValidRangeY, info_0.m_oEndValidRangeY - offset);

    }
}

void TestStartEndMisalignmentDetection::test2VisibleEdges()
{
    using precitec::filter::start_end_detection::BackgroundPosition;
    using precitec::filter::start_end_detection::EdgePositionInImage;
    using precitec::filter::start_end_detection::EdgePositionInSeam;
    using precitec::filter::start_end_detection::Appearance;

    
    const double trigger_mm = 1.1;
    
    StartEndDetectionInSeam test;
    
    int expectedYStartLeft = 102;
    int expectedYStartRight = 99;
    
    int expectedYEndLeft =  185;
    int expectedYEndRight = 180;    
    
    auto getContext = [&](int imageNumber)
    {
        precitec::filter::start_end_detection::InputImageContext oContext;
        oContext.imageCounter = imageNumber;
        oContext.imagePosition_mm = trigger_mm*imageNumber;
        oContext.pixel_to_mm = 1.0;
        oContext.offsetX = 0;
        oContext.offsetY = 0;
        return oContext;
    };
    
    double currentTransitionFromBG = -1;
    double currentTransitionFromMaterial = -1;
    
  
    StartEndDetectionInSeam::EdgeMisalignment expectedMisaligmentStart{
        EdgePositionInSeam{EdgePositionInImage{Appearance::BackgroundOnBottom, {0.0, double(expectedYStartLeft)}}, getContext(2), true},
        EdgePositionInSeam{EdgePositionInImage{Appearance::BackgroundOnBottom, {0.0, double(expectedYStartRight)}},getContext(2), true}
    };

    StartEndDetectionInSeam::EdgeMisalignment expectedMisaligmentEnd{
        EdgePositionInSeam{EdgePositionInImage{Appearance::BackgroundOnTop, {0.0, double(expectedYEndLeft)}}, getContext(4), true},
        EdgePositionInSeam{EdgePositionInImage{Appearance::BackgroundOnTop, {0.0, double(expectedYEndRight)}}, getContext(4), true}
    };

    
    StartEndDetectionInSeam::EdgeMisalignment foundMisaligmentStart;
    StartEndDetectionInSeam::EdgeMisalignment foundMisaligmentEnd;;
    
#ifndef NDEBUG
    std::cout << "TEST 2 VISIBLE EDGES" << std::endl;
#endif

    int counter = 0;
    for (; counter < 2; counter++)
    {          
#ifndef NDEBUG
    std::cout << "BLACK IMAGE" << std::endl;
#endif
    
        BImage blackImage = TestImage::createBackgroundImage();
        test.process( blackImage, counter, counter*trigger_mm ,1.0, 0,0, 1024);
        
        QVERIFY(test.getSeamState() != StartEndDetectionInSeam::SeamState::Invalid);
        
        auto misalignment =  test.computeEdgeMisalignment(true);
        QCOMPARE(misalignment.found(), false);
        misalignment =  test.computeEdgeMisalignment(false);
        QCOMPARE(misalignment.found(), false);
        auto info = test.getLastImageStartEndInfo(1);
        QVERIFY(info.isTopDark && info.isBottomDark);
        QCOMPARE(info.m_oImageStateEvaluation , ImageStateEvaluation::BackgroundBeforeStart);
        QCOMPARE(test.getTransitionFromBackground(),-1);
        QCOMPARE(test.getTransitionFromFullImage(), -1);
        currentTransitionFromBG = test.getTransitionFromBackground();
        currentTransitionFromMaterial = test.getTransitionFromFullImage();
        
        //TODO: modify startEndDynamicRoiSimple for empty images? or return a default roi?
    }

    {
#ifndef NDEBUG
        std::cout << "IMAGE 2 EDGES START" << std::endl;
#endif
        BImage imageWith2Edges = TestImage::createEdgesBackgroundOnBottom(expectedYStartLeft, expectedYStartRight);
        test.process( imageWith2Edges, counter, counter*trigger_mm,1.0, 0,0, 1024);
        
        QCOMPARE(test.getSeamState(), StartEndDetectionInSeam::SeamState::FullStartEdgeFound);
        
        auto misalignmentStart =  test.computeEdgeMisalignment(true);
        QVERIFY( compareMisalignment(misalignmentStart, expectedMisaligmentStart,1));
        foundMisaligmentStart = misalignmentStart;
        
        auto misalignmentEnd =  test.computeEdgeMisalignment(false);
        QCOMPARE(misalignmentEnd.found(), false);
        QVERIFY(test.getTransitionFromBackground() > 0);
        QCOMPARE(test.getTransitionFromFullImage(), -1);
        currentTransitionFromBG = test.getTransitionFromBackground();
        currentTransitionFromMaterial = test.getTransitionFromFullImage();

        int offset = 0;
        auto info_0 = test.getLastImageStartEndInfo(offset);
            QCOMPARE(info_0.isTopDark, false);
            QCOMPARE(info_0.isBottomDark, true);
            QCOMPARE(info_0.isCropped, true);
            QCOMPARE(info_0.m_oStartValidRangeY,0);
            QVERIFY(info_0.m_oEndValidRangeY < imageWith2Edges.height() -1 );
            QCOMPARE(info_0.m_oImageStateEvaluation , ImageStateEvaluation::StartEdge);
            QCOMPARE(info_0.m_oImageState, ImageState::FullEdgeVisible);

            //TODO QCOMPARE(info_0.m_oEndValidRangeY, std::min(expectedYStartLeft, expectedYStartRight) );

        
        offset = 2;
        auto info_1 = test.getLastImageStartEndInfo(offset);
            QCOMPARE(info_1.isTopDark, false);
            QCOMPARE(info_1.isBottomDark, true);
            QCOMPARE(info_1.isCropped, true);
            QCOMPARE(info_1.m_oStartValidRangeY, 0);
            QCOMPARE(info_1.m_oEndValidRangeY, info_0.m_oEndValidRangeY - offset);
            QCOMPARE(info_1.m_oImageStateEvaluation , ImageStateEvaluation::StartEdge);
            QCOMPARE(info_1.m_oImageState , ImageState::FullEdgeVisible);
            
    }
    counter ++;
    
    {
        BImage fullImage = TestImage::createFullImage();

#ifndef NDEBUG
    std::cout << "FULL IMAGE" << std::endl;
#endif
                    
        test.process( fullImage, counter, counter*trigger_mm,1.0, 0,0, 1024);
        
        QCOMPARE(test.getSeamState(), StartEndDetectionInSeam::SeamState::FullImageFound);
        
        auto misalignmentStart =  test.computeEdgeMisalignment(true);
        QVERIFY( compareMisalignment(misalignmentStart, foundMisaligmentStart,0));
        
        auto misalignmentEnd =  test.computeEdgeMisalignment(false);
        QCOMPARE(misalignmentEnd.found(), false);
        QCOMPARE(test.getTransitionFromBackground(), currentTransitionFromBG); //the same as found previously
        QCOMPARE(test.getTransitionFromFullImage(), -1);
        currentTransitionFromBG = test.getTransitionFromBackground();
        currentTransitionFromMaterial = test.getTransitionFromFullImage();
        
        int offset = 1;
        auto info = test.getLastImageStartEndInfo(offset);
            QVERIFY(!info.isTopDark && !info.isBottomDark);
            QCOMPARE(info.isCropped, false);
            QCOMPARE(info.m_oStartValidRangeY,0);
            QCOMPARE(info.m_oEndValidRangeY, fullImage.height() -1 );         
            QCOMPARE(info.m_oImageStateEvaluation , ImageStateEvaluation::OnlyMaterial);
    }
    counter ++;

    {

#ifndef NDEBUG
    std::cout << "IMAGE 2 EDGES " << counter << std::endl;
#endif
        BImage imageWith2Edges = TestImage::createEdgesBackgroundOnTop(expectedYEndLeft, expectedYEndRight);
        test.process( imageWith2Edges, counter, counter*trigger_mm,1.0, 0,0, 1024);
        
        QCOMPARE(test.getSeamState(), StartEndDetectionInSeam::SeamState::SecondEndEdgeFound);
        
        auto misalignmentStart =  test.computeEdgeMisalignment(true);
        QVERIFY( compareMisalignment(misalignmentStart, foundMisaligmentStart,0));
        
        auto misalignmentEnd =  test.computeEdgeMisalignment(false);
        QVERIFY( compareMisalignment(misalignmentEnd, expectedMisaligmentEnd,1));
        foundMisaligmentEnd = misalignmentEnd;

        QCOMPARE(test.getTransitionFromBackground(), currentTransitionFromBG);
        QVERIFY(test.getTransitionFromFullImage() > 0);
        currentTransitionFromBG = test.getTransitionFromBackground();
        currentTransitionFromMaterial = test.getTransitionFromFullImage();

        int offset = 0;
        auto info_0 = test.getLastImageStartEndInfo(offset);
            QCOMPARE(info_0.isTopDark, true);
            QCOMPARE(info_0.isBottomDark, false);
            QCOMPARE(info_0.isCropped, true);
            QVERIFY(info_0.m_oStartValidRangeY > 0);
            QCOMPARE(info_0.m_oEndValidRangeY , imageWith2Edges.height() -1 );
            QCOMPARE(info_0.m_oImageStateEvaluation , ImageStateEvaluation::EndEdge);
            QCOMPARE(info_0.m_oImageState , ImageState::FullEdgeVisible);
        
        offset = 2;
        auto info_1 = test.getLastImageStartEndInfo(offset);
            QCOMPARE(info_1.isTopDark, true);
            QCOMPARE(info_1.isBottomDark, false);
            QCOMPARE(info_1.isCropped, true);
            QCOMPARE(info_1.m_oStartValidRangeY, info_0.m_oStartValidRangeY + offset);
            QCOMPARE(info_1.m_oEndValidRangeY , imageWith2Edges.height() -1 );
        
    }
    counter ++;
    
    {
        BImage blackImage = TestImage::createBackgroundImage();
            
        #ifndef NDEBUG
        std::cout << "BLACK IMAGE" << std::endl;
        #endif

        test.process( blackImage, counter, counter*trigger_mm,1.0, 0,0, 1024);

        QCOMPARE(test.getSeamState(), StartEndDetectionInSeam::SeamState::EndBackgroundImageFound);

        auto misalignmentStart =  test.computeEdgeMisalignment(true);
        QVERIFY( compareMisalignment(misalignmentStart, foundMisaligmentStart,0));
        
        auto misalignmentEnd =  test.computeEdgeMisalignment(false);
        QVERIFY( compareMisalignment(misalignmentEnd, foundMisaligmentEnd,0));

        QCOMPARE(test.getTransitionFromBackground(),currentTransitionFromBG);
        QCOMPARE(test.getTransitionFromFullImage(), currentTransitionFromMaterial);
        currentTransitionFromBG = test.getTransitionFromBackground();
        currentTransitionFromMaterial = test.getTransitionFromFullImage();
        
        auto info = test.getLastImageStartEndInfo(1);
        //TODO: modify startEndDynamicRoiSimple for empty images? or return a default roi?
        QVERIFY(info.isTopDark && info.isBottomDark);
        QCOMPARE(info.m_oImageStateEvaluation , ImageStateEvaluation::BackgroundAfterEnd);
    }
    counter ++;
    

    {
#ifndef NDEBUG
    std::cout << "FULL IMAGE (UNEXPECTED)" << std::endl;
#endif
    
        BImage fullImage = TestImage::createFullImage();
        test.process( fullImage, counter, counter*trigger_mm,1.0, 0,0, 1024);
        
        QCOMPARE(test.getSeamState(), StartEndDetectionInSeam::SeamState::EndBackgroundImageFound);

        auto misalignmentStart =  test.computeEdgeMisalignment(true);
        QVERIFY( compareMisalignment(misalignmentStart, foundMisaligmentStart,0));
        
        auto misalignmentEnd =  test.computeEdgeMisalignment(false);
        QVERIFY( compareMisalignment(misalignmentEnd, foundMisaligmentEnd,0));
        
        QCOMPARE(test.getTransitionFromBackground(),currentTransitionFromBG);
        QCOMPARE(test.getTransitionFromFullImage(), currentTransitionFromMaterial);
        currentTransitionFromBG = test.getTransitionFromBackground();
        currentTransitionFromMaterial = test.getTransitionFromFullImage();

        
        int offset = 1;
        auto info = test.getLastImageStartEndInfo(offset);
            QVERIFY(!info.isTopDark && !info.isBottomDark);
            QCOMPARE(info.isCropped, false);
            QCOMPARE(info.m_oStartValidRangeY,0);
            QCOMPARE(info.m_oEndValidRangeY, fullImage.height() -1 );
    }
    counter ++;
    
    {
        
#ifndef NDEBUG
    std::cout << "IMAGE 2 EDGES (UNEXPECTED)" << std::endl;
#endif
    
        int YRight = 50;
        int YLeft = 100;
        BImage imageWith2Edges = TestImage::createEdgesBackgroundOnBottom(YLeft, YRight);
        test.process( imageWith2Edges, counter, counter*trigger_mm,1.0, 0,0, 1024);
        
        QCOMPARE(test.getSeamState(), StartEndDetectionInSeam::SeamState::EndBackgroundImageFound);
        
        auto misalignmentStart =  test.computeEdgeMisalignment(true);
        QVERIFY( compareMisalignment(misalignmentStart, foundMisaligmentStart,0));
        
        auto misalignmentEnd =  test.computeEdgeMisalignment(false);
        QVERIFY( compareMisalignment(misalignmentEnd, foundMisaligmentEnd,0));


        QCOMPARE(test.getTransitionFromBackground(),currentTransitionFromBG);
        QCOMPARE(test.getTransitionFromFullImage(), currentTransitionFromMaterial);
        currentTransitionFromBG = test.getTransitionFromBackground();
        currentTransitionFromMaterial = test.getTransitionFromFullImage();


        int offset = 0;
        auto info_0 = test.getLastImageStartEndInfo(offset);
            QCOMPARE(info_0.isTopDark, false);
            QCOMPARE(info_0.isBottomDark, true);
            QCOMPARE(info_0.isCropped, true);
            QCOMPARE(info_0.m_oStartValidRangeY,0);
            //TODO QCOMPARE(info_0.m_oEndValidRangeY, std::min(expectedYStartLeft, expectedYStartRight) );
            QVERIFY(info_0.m_oEndValidRangeY < imageWith2Edges.height() -1 );
            QCOMPARE(info_0.m_oImageStateEvaluation , ImageStateEvaluation::Unknown);
    
        offset = 2;
        auto info_1 = test.getLastImageStartEndInfo(offset);
            QCOMPARE(info_1.isTopDark, false);
            QCOMPARE(info_1.isBottomDark, true);
            QCOMPARE(info_1.isCropped, true);
            QCOMPARE(info_1.m_oStartValidRangeY, 0);
            QCOMPARE(info_1.m_oEndValidRangeY, info_0.m_oEndValidRangeY - offset);
        
    }
}


void TestStartEndMisalignmentDetection::test1VisibleEdge()
{
    using precitec::filter::start_end_detection::BackgroundPosition;
    using precitec::filter::start_end_detection::BackgroundPosition;
    using precitec::filter::start_end_detection::EdgePositionInImage;
    using precitec::filter::start_end_detection::EdgePositionInSeam;
    using precitec::filter::start_end_detection::Appearance;
    
    
    const double trigger_mm = 1.1;
    
    StartEndDetectionInSeam test;
    test.setDirection(StartEndDetectionInSeam::Direction::fromBelow);
    
    std::pair<int,int> expectedYStartLeft = {1,102};
    std::pair<int,int> expectedYStartRight = {2, 99};
    
    
    std::pair<int,int>  expectedYEndLeft =  {4,185};
    std::pair<int,int>  expectedYEndRight = {4,180};    
    
    auto getContext = [&](int imageNumber)
    {
        precitec::filter::start_end_detection::InputImageContext oContext;
        oContext.imageCounter = imageNumber;
        oContext.imagePosition_mm = trigger_mm*imageNumber;
        oContext.pixel_to_mm = 1.0;
        oContext.offsetX = 0;
        oContext.offsetY = 0;
        oContext.sensorImageHeight = 1024;
        return oContext;
    };
    
    StartEndDetectionInSeam::EdgeMisalignment expectedMisaligmentStart{
        EdgePositionInSeam{EdgePositionInImage{Appearance::BackgroundOnBottom, {0.0, double(expectedYStartLeft.second)}}, getContext(expectedYStartLeft.first), true },
        EdgePositionInSeam{EdgePositionInImage{Appearance::BackgroundOnBottom, {0.0, double(expectedYStartRight.second)}}, getContext(expectedYStartRight.first), true }
    };

    StartEndDetectionInSeam::EdgeMisalignment expectedMisaligmentEnd{
        EdgePositionInSeam{EdgePositionInImage{Appearance::BackgroundOnTop, {0.0, double(expectedYEndLeft.second)}}, getContext(expectedYEndLeft.first), true},
        EdgePositionInSeam{EdgePositionInImage{Appearance::BackgroundOnTop, {0.0, double(expectedYEndRight.second)}}, getContext(expectedYEndRight.first), true }
    };

    
    StartEndDetectionInSeam::EdgeMisalignment foundMisaligmentStart;
    StartEndDetectionInSeam::EdgeMisalignment foundMisaligmentEnd;
    double currentTransitionFromBG = -1;
    double currentTransitionFromMaterial = -1;
    
    int counter = 0;
    {
#ifndef NDEBUG
        std::cout << "BLACK IMAGE" << std::endl;
#endif
        BImage blackImage = TestImage::createBackgroundImage();
          
    
        test.process( blackImage, counter, counter*trigger_mm,1.0, 0,0, 1024);
        
        QVERIFY(test.getSeamState() != StartEndDetectionInSeam::SeamState::Invalid);
        
        auto misalignmentStart =  test.computeEdgeMisalignment(true);
        QCOMPARE(misalignmentStart.found(), false);
        
        auto misalignmentEnd =  test.computeEdgeMisalignment(false);
        QCOMPARE(misalignmentEnd.found(), false);


        QCOMPARE(test.getTransitionFromBackground(), -1);
        QCOMPARE(test.getTransitionFromFullImage(), -1);
        currentTransitionFromBG = test.getTransitionFromBackground();
        currentTransitionFromMaterial = test.getTransitionFromFullImage();

        
        auto info = test.getLastImageStartEndInfo(1);
        QVERIFY(info.isTopDark && info.isBottomDark);
    }
        //TODO: modify startEndDynamicRoiSimple for empty images? or return a default roi?
    counter ++;

    {
#ifndef NDEBUG
    std::cout << "IMAGE ONLY LEFT EDGE" << std::endl;
#endif
        BImage imageLeftEdge = TestImage::createEdgesBackgroundOnBottom(expectedYStartLeft.second, 0);
        test.process( imageLeftEdge, counter, counter*trigger_mm,1.0, 0,0, 1024);
        
       QCOMPARE(int(test.getSeamState()), int(StartEndDetectionInSeam::SeamState::FirstStartEdgeFound)); 
        
        QCOMPARE (test.getLeftEdgeInLastImage().mAppearance, Appearance::BackgroundOnBottom);
        QCOMPARE (test.getRightEdgeInLastImage().mAppearance, Appearance::AllBackground);
        
        auto misalignmentStart =  test.computeEdgeMisalignment(true);
        QCOMPARE(misalignmentStart.found(), false);
               
        auto misalignmentEnd =  test.computeEdgeMisalignment(false);
        QCOMPARE(misalignmentEnd.found(), false);

        QCOMPARE(test.getTransitionFromBackground(),-1);
        QCOMPARE(test.getTransitionFromFullImage(), -1);
        currentTransitionFromBG = test.getTransitionFromBackground();
        currentTransitionFromMaterial = test.getTransitionFromFullImage();


        int offset = 0;
        auto info_0 = test.getLastImageStartEndInfo(offset);
        QCOMPARE(info_0.isTopDark, true);  // top left is grey, top right is dark
        QCOMPARE(info_0.isBottomDark, true);
        
    }
    counter ++;
    
    {
#ifndef NDEBUG
    std::cout << "IMAGE ONLY RIGHT EDGE" << std::endl;
#endif
    
        //black on bottom, only rightEdge 
        //the left side is all material already (left edge already met)
        BImage imageRightEdge = TestImage::createEdgesBackgroundOnBottom(TestImage::imageSize().height -1, expectedYStartRight.second);
                 
        test.process( imageRightEdge, counter, counter*trigger_mm,1.0, 0,0, 1024);
        
        QCOMPARE (test.getLeftEdgeInLastImage().mAppearance, Appearance::AllMaterial);
        QCOMPARE (test.getRightEdgeInLastImage().mAppearance, Appearance::BackgroundOnBottom);
        
        QCOMPARE(test.getSeamState(), StartEndDetectionInSeam::SeamState::SecondStartEdgeFound);
        QCOMPARE(test.startEdgesFound(), true);
        
        auto misalignmentStart =  test.computeEdgeMisalignment(true);
        QVERIFY(compareMisalignment(misalignmentStart, expectedMisaligmentStart,1));
        foundMisaligmentStart = misalignmentStart;
        
        auto misalignmentEnd =  test.computeEdgeMisalignment(false);
        QCOMPARE(misalignmentEnd.found(), false);

        QVERIFY(test.getTransitionFromBackground() > 0);
        QCOMPARE(test.getTransitionFromFullImage(), currentTransitionFromMaterial);
        currentTransitionFromBG = test.getTransitionFromBackground();
        currentTransitionFromMaterial = test.getTransitionFromFullImage();

        int offset = 0;
        auto info_0 = test.getLastImageStartEndInfo(offset);
            QCOMPARE(info_0.isTopDark, false);
            QCOMPARE(info_0.isBottomDark, true);
            QCOMPARE(info_0.isCropped, true);
            QCOMPARE(info_0.m_oStartValidRangeY,0);
            //TODO QCOMPARE(info_0.m_oEndValidRangeY, std::min(expectedYStartLeft, expectedYStartRight) );
            QVERIFY(info_0.m_oEndValidRangeY < imageRightEdge.height() -1 );

        
        offset = 2;
        auto info_1 = test.getLastImageStartEndInfo(offset);
            QCOMPARE(info_1.isTopDark, false);
            QCOMPARE(info_1.isBottomDark, true);
            QCOMPARE(info_1.isCropped, true);
            QCOMPARE(info_1.m_oStartValidRangeY, 0);
            QCOMPARE(info_1.m_oEndValidRangeY, info_0.m_oEndValidRangeY - offset);
        
    }
    counter ++;
    
    {

#ifndef NDEBUG
    std::cout << "FULL IMAGE" << std::endl;
#endif
        BImage fullImage = TestImage::createFullImage();
        test.process( fullImage, counter, counter*trigger_mm,1.0, 0,0, 1024);
        
        QCOMPARE(test.getSeamState(), StartEndDetectionInSeam::SeamState::FullImageFound);
        
        auto misalignmentStart =  test.computeEdgeMisalignment(true);
        QVERIFY(compareMisalignment(misalignmentStart, foundMisaligmentStart, 0));
        
        auto misalignmentEnd =  test.computeEdgeMisalignment(false);
        QCOMPARE(misalignmentEnd.found(), false);

        QCOMPARE(test.getTransitionFromBackground(),currentTransitionFromBG);
        QCOMPARE(test.getTransitionFromFullImage(), -1);
        currentTransitionFromBG = test.getTransitionFromBackground();
        currentTransitionFromMaterial = test.getTransitionFromFullImage();
        
        int offset = 1;
        auto info = test.getLastImageStartEndInfo(offset);
            QVERIFY(!info.isTopDark && !info.isBottomDark);
            QCOMPARE(info.isCropped, false);
            QCOMPARE(info.m_oStartValidRangeY,0);
            QCOMPARE(info.m_oEndValidRangeY, fullImage.height() -1 );
    }
    counter ++;

    {

#ifndef NDEBUG
    std::cout << "FULL IMAGE (test MidSeam info)" << std::endl;
#endif
        BImage fullImage = TestImage::createFullImage();
        test.updateOnSeamPositionInfo(precitec::filter::start_end_detection::StartEndDetectionInSeam::SeamPositionInfo::Middle);
        test.process( fullImage, counter, counter*trigger_mm,1.0, 0,0, 1024);

        QCOMPARE(test.getSeamState(), StartEndDetectionInSeam::SeamState::FullImageFound);

        auto misalignmentStart =  test.computeEdgeMisalignment(true);
        QVERIFY(compareMisalignment(misalignmentStart, foundMisaligmentStart, 0));

        auto misalignmentEnd =  test.computeEdgeMisalignment(false);
        QCOMPARE(misalignmentEnd.found(), false);

        QCOMPARE(test.getTransitionFromBackground(),currentTransitionFromBG);
        QCOMPARE(test.getTransitionFromFullImage(), -1);
        currentTransitionFromBG = test.getTransitionFromBackground();
        currentTransitionFromMaterial = test.getTransitionFromFullImage();

        int offset = 1;
        auto info = test.getLastImageStartEndInfo(offset);
            QVERIFY(!info.isTopDark && !info.isBottomDark);
            QCOMPARE(info.isCropped, false);
            QCOMPARE(info.m_oStartValidRangeY,0);
            QCOMPARE(info.m_oEndValidRangeY, fullImage.height() -1 );
    }
    counter ++;
    {

#ifndef NDEBUG
    std::cout << "FULL IMAGE (test EndCandidate info)" << std::endl;
#endif
        BImage fullImage = TestImage::createFullImage();
        test.updateOnSeamPositionInfo(precitec::filter::start_end_detection::StartEndDetectionInSeam::SeamPositionInfo::EndCandidate);
        test.process( fullImage, counter, counter*trigger_mm,1.0, 0,0, 1024);

        QCOMPARE(test.getSeamState(), StartEndDetectionInSeam::SeamState::FullImageFound);

        auto misalignmentStart =  test.computeEdgeMisalignment(true);
        QVERIFY(compareMisalignment(misalignmentStart, foundMisaligmentStart, 0));

        auto misalignmentEnd =  test.computeEdgeMisalignment(false);
        QCOMPARE(misalignmentEnd.found(), false);

        QCOMPARE(test.getTransitionFromBackground(),currentTransitionFromBG);
        QCOMPARE(test.getTransitionFromFullImage(), -1);
        currentTransitionFromBG = test.getTransitionFromBackground();
        currentTransitionFromMaterial = test.getTransitionFromFullImage();

        int offset = 1;
        auto info = test.getLastImageStartEndInfo(offset);
            QVERIFY(!info.isTopDark && !info.isBottomDark);
            QCOMPARE(info.isCropped, false);
            QCOMPARE(info.m_oStartValidRangeY,0);
            QCOMPARE(info.m_oEndValidRangeY, fullImage.height() -1 );
    }
    counter ++;

    {
        
#ifndef NDEBUG
    std::cout << "IMAGE 2 EDGES" << std::endl;
#endif
        BImage imageWith2Edges = TestImage::createEdgesBackgroundOnTop(expectedYEndLeft.second, expectedYEndRight.second);
        
        QVERIFY(imageWith2Edges.getValue(50,expectedYEndLeft.second) == TestImage::materialIntensity && imageWith2Edges.getValue(50,expectedYEndLeft.second-1) == 0);
        QVERIFY(imageWith2Edges.getValue(450,expectedYEndRight.second) == TestImage::materialIntensity && imageWith2Edges.getValue(450,expectedYEndRight.second-1) == 0);
        
    
        test.process( imageWith2Edges, counter, counter*trigger_mm,1.0, 0,0, 1024);
        QCOMPARE(test.getSeamState(), StartEndDetectionInSeam::SeamState::SecondEndEdgeFound);
                
        auto misalignmentStart =  test.computeEdgeMisalignment(true);
        QVERIFY(compareMisalignment(misalignmentStart, foundMisaligmentStart, 0));
        
        auto misalignmentEnd =  test.computeEdgeMisalignment(false);
        QVERIFY(compareMisalignment(misalignmentEnd, expectedMisaligmentEnd, 1));
        foundMisaligmentEnd = misalignmentEnd;

        QCOMPARE(test.getTransitionFromBackground(),currentTransitionFromBG);
        QVERIFY(test.getTransitionFromFullImage() > 0);
        currentTransitionFromBG = test.getTransitionFromBackground();
        currentTransitionFromMaterial = test.getTransitionFromFullImage();
        
        int offset = 0;
        auto info_0 = test.getLastImageStartEndInfo(offset);
            QCOMPARE(info_0.isTopDark, true);
            QCOMPARE(info_0.isBottomDark, false);
            QCOMPARE(info_0.isCropped, true);
            QVERIFY(info_0.m_oStartValidRangeY>0);
            QCOMPARE(info_0.m_oEndValidRangeY ,  imageWith2Edges.height() -1 );

        
        offset = 2;
        auto info_1 = test.getLastImageStartEndInfo(offset);
            QCOMPARE(info_1.isTopDark, true);
            QCOMPARE(info_1.isBottomDark, false);
            QCOMPARE(info_1.isCropped, true);
            QCOMPARE(info_1.m_oStartValidRangeY,  info_0.m_oStartValidRangeY + offset);
            QCOMPARE(info_1.m_oEndValidRangeY ,  imageWith2Edges.height() -1 );
    }
    counter ++;
   
 
    {
        #ifndef NDEBUG
        std::cout << "BLACK IMAGE" << std::endl;
        #endif
        
        BImage blackImage = TestImage::createBackgroundImage();
        test.process( blackImage, counter, counter*trigger_mm,1.0, 0,0, 1024);

        QCOMPARE(test.getSeamState(), StartEndDetectionInSeam::SeamState::EndBackgroundImageFound);
        
        auto misalignmentStart =  test.computeEdgeMisalignment(true);
        QVERIFY(compareMisalignment(misalignmentStart, foundMisaligmentStart, 0));
        
        auto misalignmentEnd =  test.computeEdgeMisalignment(false);        
        QVERIFY(compareMisalignment(misalignmentEnd, foundMisaligmentEnd, 0));

        QCOMPARE(test.getTransitionFromBackground(),currentTransitionFromBG);
        QCOMPARE(test.getTransitionFromFullImage(), currentTransitionFromMaterial);
        currentTransitionFromBG = test.getTransitionFromBackground();
        currentTransitionFromMaterial = test.getTransitionFromFullImage();
        
        auto info = test.getLastImageStartEndInfo(1);
        //TODO: modify startEndDynamicRoiSimple for empty images? or return a default roi?
        QVERIFY(info.isTopDark && info.isBottomDark);
    }
    counter ++;
    
}


void TestStartEndMisalignmentDetection::testValidSequence_data()
{
    int sensorWidth = m_oTestCombinedImage.m_Image.width();
    int sensorHeight = 80;
    
    std::vector<int> imageNumbersToTest;
    auto fFillImageNumbersToTest = [& imageNumbersToTest, this] (int pixelOffset)
    {
        imageNumbersToTest.clear();
        imageNumbersToTest.reserve(m_oTestCombinedImage.m_Image.height() / pixelOffset);
        for (int i = 0, last = m_oTestCombinedImage.m_Image.height() / pixelOffset; i< last;i ++)
        {
            imageNumbersToTest.push_back(i);
        }
    };
    
    
    QTest::addColumn<bool>("fromAbove");
    QTest::addColumn<int>("sensorWidth");
    QTest::addColumn<int>("sensorHeight");
    QTest::addColumn<int>("pixelOffsetBetweenImages");
    QTest::addColumn<std::vector<int>>("imageNumbersToTest");
    QTest::addColumn<bool>("ensureSingleEdgeInTestCase");
    QTest::addColumn<bool>("ensureEdgeInTestCase");
    
    for (bool fromAbove : {true, false})
    {
        std::string strDirection{fromAbove ? "FromAbove" : "FromBelow"};
        
        {
            int pixelOffset = 8;
            fFillImageNumbersToTest(pixelOffset);
            QTest::newRow(std::string(strDirection + "_tight").c_str())         
            << fromAbove <<sensorWidth << sensorHeight<< pixelOffset <<  imageNumbersToTest  << true << true;
        }
        {
            int pixelOffset = 60;
            fFillImageNumbersToTest(pixelOffset);
            QTest::newRow(std::string(strDirection + "_overlapping").c_str())         
            << fromAbove <<sensorWidth << sensorHeight<< pixelOffset <<  imageNumbersToTest  << false << false;
        }
        {
            int pixelOffset = 80;
            fFillImageNumbersToTest(pixelOffset);
            QTest::newRow(std::string(strDirection + "_consecutive").c_str())         
            << fromAbove <<sensorWidth << sensorHeight<< pixelOffset <<  imageNumbersToTest  << false << false;
        }
        {
            int pixelOffset = 120;
            fFillImageNumbersToTest(pixelOffset);
            QTest::newRow(std::string(strDirection + "_jump").c_str())         
            << fromAbove <<sensorWidth << sensorHeight<< pixelOffset <<  imageNumbersToTest  << false << false;
        }
        // test partial sequences
        {
            int pixelOffset = 24;
            fFillImageNumbersToTest(pixelOffset);
            auto imageNumbersMaterial = m_oTestCombinedImage.getImageNumbersOnlyMaterial(fromAbove, {sensorWidth, sensorHeight}, pixelOffset);
            
            imageNumbersToTest.clear();
            for (int i = imageNumbersMaterial.first; i < imageNumbersMaterial.second; i++)
            {
                imageNumbersToTest.push_back(i);
            }            
            QTest::newRow(std::string(strDirection + "_OnlyMaterial").c_str())        
            << fromAbove <<sensorWidth << sensorHeight<< pixelOffset <<  imageNumbersToTest  << false << false;
            
            imageNumbersToTest.clear();
            for (int i = 0; i < imageNumbersMaterial.second; i++)
            {
                imageNumbersToTest.push_back(i);
            }            
            QTest::newRow(std::string(strDirection + "_NoEnd").c_str())   
            << fromAbove <<sensorWidth << sensorHeight<< pixelOffset <<  imageNumbersToTest  << false << false;
            
            imageNumbersToTest.clear();
            for (int i = imageNumbersMaterial.first; i < m_oTestCombinedImage.m_Image.height() / pixelOffset; i++)
            {
                imageNumbersToTest.push_back(i);
            }            
            QTest::newRow(std::string(strDirection + "_NoStart").c_str())         
            << fromAbove <<sensorWidth << sensorHeight<< pixelOffset <<  imageNumbersToTest  << false << false;
            
            
        }
    }
}

void TestStartEndMisalignmentDetection::testValidSequence()
{
    QFETCH(bool,fromAbove);
    QFETCH(int, pixelOffsetBetweenImages);
    QFETCH(bool,ensureSingleEdgeInTestCase);
    QFETCH(bool,ensureEdgeInTestCase);
    QFETCH(std::vector<int>, imageNumbersToTest);
        
    QVERIFY(m_oTestCombinedImage.m_Image.isValid());
            
    enum DirectionUsed {Correct=0, Wrong, Unknown};
    
    std::array<StartEndDetectionInSeam,3> test {StartEndDetectionInSeam{},StartEndDetectionInSeam{},StartEndDetectionInSeam{}};
    
    
    test[DirectionUsed::Correct].setDirection(fromAbove ? StartEndDetectionInSeam::Direction::fromAbove
                                                        :StartEndDetectionInSeam::Direction::fromBelow);
    test[DirectionUsed::Wrong].setDirection(fromAbove ? StartEndDetectionInSeam::Direction::fromBelow
                                                        : StartEndDetectionInSeam::Direction::fromAbove);
    test[DirectionUsed::Unknown].setDirection(StartEndDetectionInSeam::Direction::Unknown);

    const double pixel_to_mm = 0.25;
    const double trigger_mm = pixel_to_mm * pixelOffsetBetweenImages;
    
    const Size2d SensorImageSize{m_oTestCombinedImage.m_Image.width(),100};

    const precitec::geo2d::Rect subROI{ precitec::geo2d::Point{0,0}, SensorImageSize};
    
    
    
    // two seams, in order to test the resetState function
    for (int seamNumber = 0; seamNumber < 2; seamNumber ++)
    {
        int numProcessedImages = 0;
        bool imageWithSingleEdgeFound = false;
        bool fullImageEntered = false;
        bool fullImageExited = false;
    
        static const int NUM_DIRECTIONS_TO_TEST = 3;
        std::array<ImageStateEvaluation,NUM_DIRECTIONS_TO_TEST> lastImageStateEvaluations {ImageStateEvaluation::Unknown, ImageStateEvaluation::Unknown, ImageStateEvaluation::Unknown};
        std::array<StartEndDetectionInSeam::EdgeMisalignment,NUM_DIRECTIONS_TO_TEST>  lastMisalignmentStart;
        std::array<StartEndDetectionInSeam::EdgeMisalignment,NUM_DIRECTIONS_TO_TEST>  lastMisalignmentEnd;
        
        bool alwaysAcceptingPartialEdges = false;
        for (int i = 0; i<NUM_DIRECTIONS_TO_TEST ; i++)
        {
            test[i].resetState();
            QCOMPARE(test[i].alwaysAcceptPartialEdges(), alwaysAcceptingPartialEdges);
        }

        for (int & imageNumber : imageNumbersToTest)
        {
            auto oImageRect = m_oTestCombinedImage.getROI(fromAbove,imageNumber, SensorImageSize, pixelOffsetBetweenImages);
            
            if (!m_oTestCombinedImage.isValidROI(oImageRect))
            {
                break;
            }
            
            std::cout << "\n\n Image " << imageNumber 
                << " Range Y " << oImageRect.y().start() << " " <<  oImageRect.y().end() <<"\n"<< std::endl;
            
            auto oSensorImage = BImage{m_oTestCombinedImage.m_Image, oImageRect};
            auto oSubROIImage = BImage{oSensorImage, subROI};
            QVERIFY(oSubROIImage.isValid());
        
            Rect oSubROIRespectGlobalImage = { 
                {oImageRect.x().start() + subROI.x().start(), oImageRect.y().start() + subROI.y().start()},
                subROI.size()
            };
                        
            auto acceptableImageStateEvaluation = m_oTestCombinedImage.acceptableImageStateEvaluation(fromAbove,oSubROIRespectGlobalImage, 20 , alwaysAcceptingPartialEdges);
            if (acceptableImageStateEvaluation.front() == ImageStateEvaluation::OnlyMaterial)
            {
                fullImageEntered = true;
            }
            if (acceptableImageStateEvaluation.front() == ImageStateEvaluation::EndEdge || acceptableImageStateEvaluation.front() == ImageStateEvaluation::BackgroundAfterEnd)
            {
                fullImageExited = true;
            }
    
            for (int i = 0; i< NUM_DIRECTIONS_TO_TEST ; i++)
            {
                switch( i)
                {
                    case DirectionUsed::Correct:
                        std::cout << "process with correct direction " << std::endl;
                        break;
                    case DirectionUsed::Wrong:
                        std::cout << "process with wrong direction " << std::endl;
                        break;
                    case DirectionUsed::Unknown:
                        std::cout << "process with unknown direction " << std::endl;
                        break;

                }

                test[i].process(oSubROIImage, imageNumber, imageNumber*trigger_mm, pixel_to_mm, subROI.x().start(), subROI.y().start(), oSensorImage.height());
                
                auto oInfo = test[i].getLastImageStartEndInfo(0);
                
                                
                if ( i != DirectionUsed::Wrong)
                {  
                    if ( numProcessedImages > 0)
                    {
                        QVERIFY(int(oInfo.m_oImageStateEvaluation) - int(lastImageStateEvaluations[i]) <= 2);
                    }
                    bool isImageStateEvaluationExpected = false;
                    for (auto & rState: acceptableImageStateEvaluation)
                    {
                        if (oInfo.m_oImageStateEvaluation == rState)
                        {
                            isImageStateEvaluationExpected = true;
                            break;
                        }                                               
                    }
                    QVERIFY(isImageStateEvaluationExpected);
                                       
                    
                    if (oInfo.m_oImageState == ImageState::OnlyLeftEdgeVisible
                        || oInfo.m_oImageState == ImageState::OnlyRightEdgeVisible)
                    {
                        imageWithSingleEdgeFound = true;
                    }
            
                }
                else
                {
                    auto oEvaluationCorrect = test[DirectionUsed::Correct].getLastImageStartEndInfo(0).m_oImageStateEvaluation; 
                    if (oEvaluationCorrect == ImageStateEvaluation::StartEdge || oEvaluationCorrect == ImageStateEvaluation::EndEdge)
                    {
                        QCOMPARE(oInfo.m_oImageStateEvaluation , ImageStateEvaluation::Unknown);
                    }
                
                }
                
                auto oMisalignmentStart =test[i].computeEdgeMisalignment(true);
                auto oMisalignmentEnd =test[i].computeEdgeMisalignment(false);
                
            
                if (lastMisalignmentStart[i].found())
                {
                    QVERIFY(oMisalignmentStart.found());
                    QCOMPARE(oMisalignmentStart.misalignment_mm(), lastMisalignmentStart[i].misalignment_mm());
                }
                if (lastMisalignmentEnd[i].found())
                {
                    QVERIFY(oMisalignmentEnd.found());
                    QCOMPARE(oMisalignmentEnd.misalignment_mm(), lastMisalignmentEnd[i].misalignment_mm());
                }
                
                lastImageStateEvaluations[i] = oInfo.m_oImageStateEvaluation;
                lastMisalignmentStart[i] = oMisalignmentStart;
                lastMisalignmentEnd[i] = oMisalignmentEnd;
            }
            numProcessedImages ++;
        }
    
        //verify that the test was well built
        QVERIFY(numProcessedImages > 5);
        if (ensureSingleEdgeInTestCase)
        {
            QVERIFY(imageWithSingleEdgeFound);
        }
        
        
        //verify state at end of the sequence
        for (int i : {DirectionUsed::Correct, DirectionUsed::Unknown})
        {
            if (ensureEdgeInTestCase)
            {
                QCOMPARE(test[i].startEdgesFound(), true);
                QCOMPARE(test[i].endEdgesFound(), true);
            }
            if (fullImageEntered && fullImageExited)
            {
                //this check can not be performed in every test case, see for example the case "OnlyMaterial"
                QVERIFY(test[i].getTransitionFromFullImage() - test[i].getTransitionFromBackground() > 0);
            }
            else
            {
                QVERIFY(test[i].getTransitionFromFullImage() == -1 || test[i].getTransitionFromBackground() == -1);
            }
        }
        
        QCOMPARE(test[DirectionUsed::Wrong].computeEdgeMisalignment(true).found(), false);
        QCOMPARE(test[DirectionUsed::Wrong].computeEdgeMisalignment(false).found(), false);
    
    }
}



    //test roi height ! = image
    // test image intensity = 80  threshBG = 30 threshMaterial = 100
    // test DynamicROI
    //test sign of disalignment
QTEST_MAIN(TestStartEndMisalignmentDetection)
#include "testStartEndMisalignmentDetection.moc"
