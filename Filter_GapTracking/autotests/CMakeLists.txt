set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)

qtTestCase(
    NAME
        testTCPDistance
    SRCS
        TCPDistanceTest.cpp
        ../TCPDistance.cpp
    LIBS
        ${POCO_LIBS}
        fliplib
        Interfaces
        Analyzer_Interface
)

