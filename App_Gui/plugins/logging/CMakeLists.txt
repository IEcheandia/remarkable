set(plugin_SRCS
    logFilterModel.cpp
    logModel.cpp
    logReceiver.cpp
    main.cpp
    moduleModel.cpp
)

add_library(precitecloggercomponentplugin MODULE ${plugin_SRCS})
set_target_properties(precitecloggercomponentplugin PROPERTIES LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/../qml/precitec/gui/components/logging/")
target_link_libraries(precitecloggercomponentplugin
    precitecweldmasterguigeneral
    Precitec::precitecusermanagement
    Qt5::Core
    Qt5::Quick
    ${POCO_LIBS}
    Framework_Module
    Interfaces
)

# copy qml files to build dir
# required for executing from build directory
configure_file(qml/qmldir "${CMAKE_CURRENT_BINARY_DIR}/../qml/precitec/gui/components/logging/qmldir" COPYONLY)
configure_file(qml/private/LogLabel.qml "${CMAKE_CURRENT_BINARY_DIR}/../qml/precitec/gui/components/logging/private/LogLabel.qml" COPYONLY)
configure_file(qml/LogView.qml "${CMAKE_CURRENT_BINARY_DIR}/../qml/precitec/gui/components/logging/LogView.qml" COPYONLY)
configure_file(qml/LogStatusBarItem.qml "${CMAKE_CURRENT_BINARY_DIR}/../qml/precitec/gui/components/logging/LogStatusBarItem.qml" COPYONLY)
configure_file(qml/private/LogFilteringControl.qml "${CMAKE_CURRENT_BINARY_DIR}/../qml/precitec/gui/components/logging/private/LogFilteringControl.qml" COPYONLY)
configure_file(qml/private/LogDelegate.qml "${CMAKE_CURRENT_BINARY_DIR}/../qml/precitec/gui/components/logging/private/LogDelegate.qml" COPYONLY)
configure_file(qml/private/LogComponent.qml "${CMAKE_CURRENT_BINARY_DIR}/../qml/precitec/gui/components/logging/private/LogComponent.qml" COPYONLY)

install(TARGETS precitecloggercomponentplugin DESTINATION ${WM_LIB_INSTALL_DIR}/plugins/qml/precitec/gui/components/logging)
install(
    DIRECTORY
        qml/
    DESTINATION
        ${WM_LIB_INSTALL_DIR}/plugins/qml/precitec/gui/components/logging
    PATTERN "dummydata" EXCLUDE)

if (Qt5Test_FOUND  AND CPPUNIT_FOUND)
    add_subdirectory(autotests)
endif()
