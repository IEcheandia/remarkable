set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)

set (LIBS   Qt5::Test
            Qt5::Core
            ${POCO_LIBS}
            fliplib
            Interfaces
            Analyzer_Interface)

qtTestCase(
    NAME
        testLineSelectLocalExtremum
    SRCS
        lineSelectLocalExtremumTest.cpp
        ../lineSelectLocalExtremum.cpp
    LIBS
        ${LIBS}
)

qtTestCase(
    NAME
        testLineModelToLaserline
    SRCS
        lineModelToLaserlineTest.cpp
        ../lineModelToLaserline.cpp
        ../../Filtertest/dummyLogger.cpp
    LIBS
        ${LIBS}
)


qtTestCase(
    NAME
        testLineTemporalFilter
    SRCS
        testLineTemporalFilter.cpp
        ../lineTemporalFilter.cpp
        ../../Filtertest/dummyLogger.cpp
    LIBS
        ${LIBS}
)

qtTestCase(
    NAME
        testLineArithmetic
    SRCS
        testLineArithmetic.cpp
        ../lineArithmetic.cpp
    LIBS
        ${LIBS}
)
