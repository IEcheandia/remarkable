include_directories(${CMAKE_SOURCE_DIR}/thirdParty/nlohmann)

set(plugin_SRCS
    main.cpp
    availableTasksModel.cpp
    availableTriggersModel.cpp
    backupLocalDirectoryConfigurationManager.cpp
    cronConfigurationManager.cpp
    deleteBackupsConfigurationManager.cpp
    schedulerModel.cpp
    transferDirectoryConfigurationManager.cpp
    triggerFilterByTaskModel.cpp
    pathValidator.cpp
    optionalPortValidator.cpp
)
qt5_add_resources(plugin_SRCS precitecschedulercomponentplugin.qrc)

add_library(precitecschedulercomponentplugin MODULE ${plugin_SRCS})
set_target_properties(precitecschedulercomponentplugin PROPERTIES LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/../qml/precitec/gui/components/scheduler/")
target_link_libraries(precitecschedulercomponentplugin
    Qt5::Core
    Qt5::Quick
    Qt5::Concurrent
    Mod_Scheduler
)

# copy qml files to build dir
# required for executing from build directory
configure_file(qml/qmldir "${CMAKE_CURRENT_BINARY_DIR}/../qml/precitec/gui/components/scheduler/qmldir" COPYONLY)

install(TARGETS precitecschedulercomponentplugin DESTINATION ${WM_LIB_INSTALL_DIR}/plugins/qml/precitec/gui/components/scheduler)
install(FILES qml/qmldir DESTINATION ${WM_LIB_INSTALL_DIR}/plugins/qml/precitec/gui/components/scheduler)
