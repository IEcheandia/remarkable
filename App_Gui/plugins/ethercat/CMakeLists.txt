set(plugin_SRCS
    main.cpp
    abstractAnalogInOutController.cpp
    analogInController.cpp
    analogOutController.cpp
    digitalSlaveModel.cpp
    gatewayFilterModel.cpp
    gatewayModel.cpp
    recordedSignalAnalyzerController.cpp
    slaveInfoModel.cpp
    slaveInfoFilterModel.cpp
    viConfigModel.cpp
    viConfigService.cpp
)
qt5_add_resources(plugin_SRCS precitecethercatcomponentplugin.qrc)

add_library(precitecethercatcomponentplugin MODULE ${plugin_SRCS})
set_target_properties(precitecethercatcomponentplugin PROPERTIES LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/../qml/precitec/gui/components/ethercat/")
target_link_libraries(precitecethercatcomponentplugin
    Qt5::Core
    Qt5::Concurrent
    Qt5::Quick
    Precitec::precitecplotter
)

# copy qml files to build dir
# required for executing from build directory
configure_file(qml/qmldir "${CMAKE_CURRENT_BINARY_DIR}/../qml/precitec/gui/components/ethercat/qmldir" COPYONLY)

install(TARGETS precitecethercatcomponentplugin DESTINATION ${WM_LIB_INSTALL_DIR}/plugins/qml/precitec/gui/components/ethercat)
install(FILES qml/qmldir DESTINATION ${WM_LIB_INSTALL_DIR}/plugins/qml/precitec/gui/components/ethercat)

if (Qt5Test_FOUND)
    add_subdirectory(autotests)
endif()
