set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)

qtTestCase(
    NAME
        testRangeCheck
    SRCS
        testRangeCheck.cpp
        ../rangeCheck.cpp
    LIBS
        ${POCO_LIBS}
        fliplib
        Interfaces
        Analyzer_Interface
)


qtTestCase(
    NAME
        testSeamWeldingResult
    SRCS
        testSeamWeldingResult.cpp
        ../seamWeldingResult.cpp
    LIBS
        ${POCO_LIBS}
        fliplib
        Interfaces
        Analyzer_Interface
)



qtTestCase(
    NAME
        testSpotWeldingResult
    SRCS
        testSpotWeldingResult.cpp
        ../spotWeldingResult.cpp
    LIBS
        ${POCO_LIBS}
        fliplib
        Interfaces
        Analyzer_Interface
)



