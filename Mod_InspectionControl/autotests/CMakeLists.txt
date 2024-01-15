set (CMAKE_INCLUDE_CURRENT_DIR ON)
set (CMAKE_AUTOMOC ON)


testCase(
    NAME
        testVI_InspectionControl
    SRCS
        VI_InspectionControlTest.cpp
        ../src/VI_InspectionControl.cpp
        ../src/SAX_VIConfigParser.cpp
        ../src/TCPClientLWM.cpp
        ../../Filtertest/dummyLogger.cpp
        ../src/dbNotificationsServer.cpp
    LIBS
        Qt5::Test
        Qt5::Core
        ${POCO_LIBS}
        Interfaces
        System
        GlobalDefs
)

