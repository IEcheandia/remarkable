set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)

include_directories(../include)

set (LIBS   Qt5::Test
            #Qt5::Core
            #${POCO_LIBS}
            Interfaces)

testCase(
    NAME
        testMovingWindow
    SRCS
        testMovingWindow.cpp
    LIBS
        ${LIBS})



qtTestCase(
    NAME
        testCornerGrid
    SRCS
        testCornerGrid.cpp
        ../src/camGridData.cpp
        ../src/math/mathUtils.cpp
        ../src/cellData.cpp
        ../src/calibrationCornerGrid.cpp
        ../src/math/mathUtils.cpp
        ../src/camGridData.cpp
        ../src/math/2D/LineEquation.cpp
        ../src/math/3D/projectiveMathStructures.cpp
        ../src/math/2D/avgAndRegression.cpp
        ../src/CalibrationParamMap.cpp
        ../src/calibration3DCoords.cpp
        ../src/calibration3DCoordsInterpolator.cpp
        ../src/Calibration3DCoordsLoader.cpp
        ../src/UniformMapBuilder.cpp
        ../src/CalibrationScanMasterData.cpp
        ../src/linearMagnificationModel.cpp
        ../../Filtertest/dummyLogger.cpp
        
    LIBS
        ${LIBS}
        ${POCO_LIBS}
)




qtTestCase(
    NAME
        testCalibration3DCoords
    SRCS
        testCalibration3DCoords.cpp
        ../src/calibration3DCoords.cpp
        ../src/calibrationCornerGrid.cpp
        ../src/calibration3DCoordsInterpolator.cpp
        ../src/Calibration3DCoordsLoader.cpp
        ../src/math/3D/projectiveMathStructures.cpp
        ../src/math/2D/LineEquation.cpp
        ../src/math/2D/avgAndRegression.cpp
        ../src/cellData.cpp
        ../src/camGridData.cpp
        ../src/linearMagnificationModel.cpp
        ../../Filtertest/dummyLogger.cpp
    LIBS
        ${LIBS}
        ${POCO_LIBS}
)

qtTestCase(
    NAME
        testCalibrationCorrectionContainer
    SRCS
        testCalibrationCorrectionContainer.cpp
        ../src/CalibrationCorrectionContainer.cpp
        ../../Interfaces/src/logMessage.cpp
    LIBS
        Qt5::Test
        ${POCO_LIBS}
)

qtTestCase(
    NAME
        testImage
    SRCS
        testImage.cpp
        ../../Filtertest/dummyLogger.cpp
    LIBS
        ${POCO_LIBS}
        Interfaces
)

qtTestCase(
    NAME
        testResampleImage
    SRCS
        testResampleImage.cpp
        ../src/algoImage.cpp
        ../src/samplingInformation.cpp
        ../src/math/3D/projectiveMathStructures.cpp
        ../../Filtertest/dummyLogger.cpp
    LIBS
        ${LIBS}
)

qtTestCase(
    NAME
        testLinearMagnificationModel
    SRCS
        testLinearMagnificationModel.cpp
        ../src/math/2D/LineEquation.cpp
        ../src/math/2D/avgAndRegression.cpp
        ../src/linearMagnificationModel.cpp
        ../../Filtertest/dummyLogger.cpp
    LIBS
        ${LIBS}
        ${POCO_LIBS}
)

qtTestCase(
    NAME
        testMemoryFile
    SRCS
        testMemoryFile.cpp
    LIBS
        ${POCO_LIBS}
        Analyzer_Interface
)
