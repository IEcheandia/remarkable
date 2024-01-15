set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
add_compile_definitions( __debugSignalSumError)
qtTestCase(
    NAME
        testSumError
    SRCS
        testSumError.cpp
        ../src/sumError.cpp
        ../src/SignalSumError.cpp
        ../src/SignalSumErrorAccumulatedOutlier.cpp
        ../src/SignalSumErrorDualOutlierInRange.cpp
        ../src/SignalSumErrorSingleInlier.cpp
        ../src/SignalSumErrorSingleOutlier.cpp
        ../src/LevelTwoError.cpp
        ../src/LevelTwoErrorAccumulated.cpp
        ../src/LevelTwoErrorAdjacent.cpp
        ../src/AnalysisSumError.cpp
        ../src/AnalysisSumErrorSingleOutlier.cpp
        ../src/AnalysisSumErrorAccumulatedOutlier.cpp

    LIBS
        Qt5::Test
        Qt5::Core
        ${POCO_LIBS}
        fliplib
        Interfaces
)

