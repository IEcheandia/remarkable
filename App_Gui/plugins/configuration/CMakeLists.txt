set(LIBRARY_SRCS
    etherCATConfigurationController.cpp
    etherCATConfigurationChangeEntry.cpp
    hardwareConfigurationBackupModel.cpp
    hardwareConfigurationBackupChangeEntry.cpp
    hardwareConfigurationRestoreChangeEntry.cpp
    shutdownChangeEntry.cpp
    shutdownService.cpp
    systemHardwareBackupHelper.cpp
    upsConfigurationChangeEntry.cpp
    upsModel.cpp
)

add_library(precitecweldmasterguiconfiguration SHARED ${LIBRARY_SRCS})
target_link_libraries(precitecweldmasterguiconfiguration
    Qt5::Core
    Precitec::precitecusermanagement
    Precitec::precitecnetworkmanagement
    ${POCO_LIBS}
    Interfaces
)

set(PLUGIN_SRCS
    main.cpp
)
qt5_add_resources(PLUGIN_SRCS precitecweldmasterguiconfiguration.qrc)

add_library(precitecweldmasterguiconfigurationplugin MODULE ${PLUGIN_SRCS})

set_target_properties(precitecweldmasterguiconfigurationplugin PROPERTIES LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/../qml/precitec/gui/configuration/")
target_link_libraries(precitecweldmasterguiconfigurationplugin
    Qt5::Quick
    Precitec::precitecusermanagement
    Precitec::precitecnetworkmanagement
    precitecweldmasterguiconfiguration
)

install(TARGETS precitecweldmasterguiconfiguration DESTINATION ${WM_LIB_INSTALL_DIR})
install(TARGETS precitecweldmasterguiconfigurationplugin DESTINATION ${WM_LIB_INSTALL_DIR})
install(TARGETS precitecweldmasterguiconfigurationplugin DESTINATION ${WM_LIB_INSTALL_DIR}/plugins/qml/precitec/gui/configuration)
install(FILES qml/qmldir DESTINATION ${WM_LIB_INSTALL_DIR}/plugins/qml/precitec/gui/configuration)

if (CPPUNIT_FOUND)
    add_subdirectory(autotests)
endif()
