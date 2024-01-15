qtTestCase(
    NAME
        testModuleModel
    SRCS
        testModuleModel.cpp
        ../moduleModel.cpp
)

qtTestCase(
    NAME
        testLogFilterModel
    SRCS
        testLogFilterModel.cpp
        ../logFilterModel.cpp
        ../logModel.cpp
        ../logReceiver.cpp
        ../moduleModel.cpp
    LIBS
        precitecweldmasterguigeneral
        ${POCO_LIBS}
        Precitec::precitecusermanagement
        System
        Framework_Module
        Interfaces
)

qtTestCase(
    NAME
        testLogModel
    SRCS
        testLogModel.cpp
        ../logModel.cpp
        ../logReceiver.cpp
        ../moduleModel.cpp
        ../../../src/moduleLogger.cpp
    LIBS
        precitecweldmasterguigeneral
        ${POCO_LIBS}
        Precitec::precitecusermanagement
        System
        Framework_Module
        Interfaces
)

add_test(NAME Weldmaster-Quick-logging COMMAND xvfb-run -a --server-args=-screen\ 0\ 1024x768x24 ${_qt5_install_prefix}/../../bin/qmltestrunner -input ${CMAKE_CURRENT_SOURCE_DIR}/qml/)
