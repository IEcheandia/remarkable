set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)

set (LIBS   Qt5::Test
            Qt5::Core
            ${POCO_LIBS}
            fliplib
            Interfaces
            Analyzer_Interface)

testCase(
    NAME
        testImageArithmetic
    SRCS
        testImageArithmetic.cpp
        ../imageArithmetic.cpp
        ../frameBuffer.cpp
        ../operationsOnImageVector.cpp
    LIBS
        ${LIBS}
)

#do not use testCase to avod running it with CTest
 qtBenchmarkCase(
    NAME
        benchmarkImageArithmetic
    SRCS
       benchmarkImageArithmetic.cpp
        ../imageArithmetic.cpp
        ../frameBuffer.cpp
        ../operationsOnImageVector.cpp
        ../../Filtertest/dummyLogger.cpp
    LIBS
        ${LIBS}
 )


testCase(
    NAME
        testSurfaceCalculator
    SRCS
        testSurfaceCalculator.cpp
        ../surfaceCalculator.cpp
    LIBS
        ${LIBS}
)

testCase(
    NAME
        testSurfaceCalculatorAdaptInput
    SRCS
        testSurfaceCalculatorAdaptInput.cpp
        ../surfaceCalculator.cpp
        ../surfaceCalculatorAdaptInput.cpp
    LIBS
        ${LIBS}
)


testCase(
    NAME
        testStartEndMisalignmentDetection
    SRCS
        testStartEndMisalignmentDetection.cpp
        ../startEndMisalignmentDetection.cpp
        ../startEndDetectionInImage.cpp
        ../startEndDetectionData.cpp
    LIBS
        ${LIBS}
)


testCase(
    NAME
        testCircleHough
    SRCS
        testCircleHough.cpp
        ../circleHough.cpp
        ../circleFitImpl.cpp
    LIBS
        ${LIBS}
)


testCase(
    NAME
        testCircleFit
    SRCS
        testCircleFit.cpp
        ../circleFit.cpp
        ../circleFitImpl.cpp
    LIBS
        ${LIBS}
)


testCase(
    NAME
        testCircleFitXT
    SRCS
        testCircleFitXT.cpp
        ../circleFitXT.cpp
        ../circleFitImpl.cpp
        ../../Analyzer_Interface/src/algoPoint.cpp
    LIBS
        ${LIBS}
)

testCase(
    NAME
        testTileFeature
    SRCS
        testTileFeature.cpp
        ../tileFeature.cpp
        ../crossCorrelationImpl.cpp
    LIBS
        ${LIBS}
)


testCase(
    NAME
        testAdjustContrast
    SRCS
        testAdjustContrast.cpp
        ../adjustContrast.cpp
    LIBS
        ${LIBS}
)

testCase(
    NAME
        testMetrology
    SRCS
        testMetrology.cpp
        ../metrology.cpp
    LIBS
        ${LIBS}
        opencv_core
        opencv_imgproc
)

testCase(
    NAME
        testEdgeDetection
    SRCS
        testEdgeDetection.cpp
        ../edgeDetectionImpl.cpp
        ../../Filtertest/dummyLogger.cpp
    LIBS
        ${LIBS}
)

testCase(
    NAME
        testBinarizeDynamicOnOff
    SRCS
        testBinarizeDynamicOnOff.cpp
        ../binarizeDynamic.cpp
        ../binarizeDynamicOnOff.cpp
    LIBS
        ${LIBS}
)

testCase(
    NAME
        testLineSmoothFFT
    SRCS
        testLineSmoothFFT.cpp
    LIBS
        ${LIBS}
)

testCase(
    NAME
        testParallelLocalExtremum
    SRCS
        testParallelLocalExtremum.cpp
    LIBS
        ${LIBS}
)

testCase(
    NAME
        testStartEndROIChecker
    SRCS
        testStartEndROIChecker.cpp
        ../startEndROIChecker.cpp
    LIBS
        ${LIBS}
)

testCase(
    NAME
        testDynamicRoiSimple
    SRCS
        testDynamicRoiSimple.cpp
        ../dynamicRoiSimple.cpp
    LIBS
        ${LIBS}
)

testCase(
    NAME
        testDynamicRoi
    SRCS
        testDynamicRoi.cpp
        ../dynamicRoi.cpp
    LIBS
        ${LIBS}
)

testCase(
    NAME
        testStencil
    SRCS
        testStencil.cpp
        ../stencil.cpp
    LIBS
        ${LIBS}
        Qt5::Gui
)

