set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)

qtTestCase(
    NAME
        testLaserlineTracker
    SRCS
        LaserlineTrackerTest.cpp
        ../laserlineTracker1.cpp
        ../laserlineResults.cpp
        ../../Filtertest/dummyLogger.cpp
    LIBS
        ${POCO_LIBS}
        fliplib
        Interfaces
        Analyzer_Interface
)

qtTestCase(
    NAME
        testFindGap
    SRCS
        findGapTest.cpp
        ../findGap.cpp
        ../../Filtertest/dummyLogger.cpp
    LIBS
        ${POCO_LIBS}
        fliplib
        Interfaces
        Analyzer_Interface
)
