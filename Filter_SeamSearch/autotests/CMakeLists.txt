set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)

qtTestCase(
    NAME
        testSelectFourPeaks
    SRCS
        SelectFourPeaksTest.cpp
        ../selectFourPeaks.cpp
        ../selectFourPeaks.h
    LIBS
        Qt5::Test
        Qt5::Core
        ${POCO_LIBS}
        fliplib
        Interfaces
        Analyzer_Interface
)
