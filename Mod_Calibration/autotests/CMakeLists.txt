set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)

qtTestCase(
    NAME
        testCalibrationOCTData
    SRCS
        testCalibrationOCTData.cpp
        ../src/CalibrationOCTData.cpp
        ../src/CalibrationOCTMeasurementModel.cpp
        ../src/CalibrationOCTConfigurationIDM.cpp
        ../src/CalibrationOCTLineParameters.cpp
        ../../Filtertest/dummyLogger.cpp
    LIBS
        Interfaces
        Analyzer_Interface        
        ${POCO_LIBS}
)

qtTestCase(
    NAME
        testOCTCoordinates
    SRCS
        testOCTCoordinates.cpp
        ../src/CalibrationOCTData.cpp
        ../src/CalibrationOCTMeasurementModel.cpp
        ../src/CalibrationOCTConfigurationIDM.cpp
        ../src/CalibrationOCTLineParameters.cpp
        ../../Filtertest/dummyLogger.cpp

    LIBS
        Interfaces
        Analyzer_Interface        
        ${POCO_LIBS}
)

qtTestCase(
    NAME
        testOCTParameters
    SRCS
        testOCTParameters.cpp
        ../src/CalibrationOCTData.cpp
        ../src/CalibrationOCTMeasurementModel.cpp
        ../src/CalibrationOCTConfigurationIDM.cpp
        ../src/CalibrationOCTLineParameters.cpp
        ../../Filtertest/dummyLogger.cpp

    LIBS
        Interfaces
        Analyzer_Interface        
        ${POCO_LIBS}
)

qtTestCase(
    NAME 
        testCalibrationOCTLineParameters
    SRCS
        testCalibrationOCTLineParameters.cpp 
        ../src/CalibrationOCTLineParameters
        ../../Filtertest/dummyLogger.cpp
    LIBS
        Interfaces
        ${POCO_LIBS}
)


qtTestCase (
    NAME
        testScanFieldImageCalculator
    SRCS
        testScanFieldImageCalculator.cpp
        ../src/ScanFieldImageCalculator.cpp
        ../../Filtertest/dummyLogger.cpp
        ../../Analyzer_Interface/src/CalibrationScanMasterData.cpp
        ../../Analyzer_Interface/src/CalibrationParamMap.cpp
        ../../Analyzer_Interface/src/ScanFieldImageParameters.cpp
    LIBS
        Interfaces
        ${POCO_LIBS}
)

qtTestCase(
    NAME
        testChessboardRecognition
    SRCS
        testChessboardRecognition.cpp
        ../src/chessboardRecognition.cpp
        ../src/chessboardFilteringAlgorithms.cpp
        ../../Filtertest/dummyLogger.cpp
    LIBS
        Interfaces
        Analyzer_Interface
        ${POCO_LIBS}
)
