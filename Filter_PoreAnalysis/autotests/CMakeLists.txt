set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)

qtTestCase(
    NAME
        testPoreClassifierOutputTriple
    SRCS
        poreClassifierOutputTripleTest.cpp
        ../poreClassifierOutputTriple.cpp
        ../poreClassifierTypes.h
    LIBS
        Qt5::Test
        Qt5::Core
        ${POCO_LIBS}
        fliplib
        Interfaces
        Analyzer_Interface
)

qtTestCase(
    NAME
        testBlobDetection
    SRCS
        blobDetectionTest.cpp
        ../blobDetection.cpp
        ../segmentateImage.cpp
    LIBS
        Qt5::Test
        Qt5::Core
        ${POCO_LIBS}
        fliplib
        Interfaces
        Analyzer_Interface
)

qtTestCase(
    NAME
        testPoreDetection
    SRCS
        poreDetectionTest.cpp
        ../poreDetection.cpp
        ../segmentateImage.cpp
        ../calcContour.cpp
        ../direction.cpp
        ../majorAxes.cpp
        ../poreStatistics.cpp
    LIBS
        Qt5::Test
        Qt5::Core
        ${POCO_LIBS}
        fliplib
        Interfaces
        Analyzer_Interface
)
