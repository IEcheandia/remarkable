# Call a shell script to generate the systemConfigurationQml.h
set(SYSTEM_CONFIG_INPUT ${CMAKE_SOURCE_DIR}/Interfaces/include/common/systemConfiguration.h)
set(SYSTEM_CONFIG_GENERATE_HEADER ${CMAKE_CURRENT_BINARY_DIR}/systemConfigurationQml.h)
set(SYSTEM_CONFIG_GENERATOR_SCRIPT ${CMAKE_CURRENT_SOURCE_DIR}/generateSystemConfiguration.sh)
set(SYSTEM_CONFIG_SOURCE_FILE systemConfigurationQml.cpp)

# The moc call for the cpp of the generated header must be done after the header generation was executed.
# Thus, we cannot use automoc, because it is executed too early and call moc explicitly below using qt_wrap_cpp.
set_property(SOURCE ${SYSTEM_CONFIG_SOURCE_FILE} PROPERTY SKIP_AUTOMOC ON)

add_custom_command(
    OUTPUT ${SYSTEM_CONFIG_GENERATE_HEADER}
    DEPENDS ${SYSTEM_CONFIG_INPUT} ${SYSTEM_CONFIG_GENERATOR_SCRIPT}
    COMMAND /bin/bash ${SYSTEM_CONFIG_GENERATOR_SCRIPT} ${SYSTEM_CONFIG_INPUT} ${SYSTEM_CONFIG_GENERATE_HEADER}
    COMMENT "Generate system configuration QML header at ${SYSTEM_CONFIG_GENERATE_HEADER}"
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)

add_custom_target(systemConfigurationQml
    DEPENDS ${SYSTEM_CONFIG_GENERATE_HEADER}
)

add_library(precitecweldmasterguigeneral SHARED
    fileSystemNameValidator.cpp
    guiConfiguration.cpp
    languageSupport.cpp
    weldmasterPaths.cpp
    removableDevicePaths.cpp
)

target_link_libraries(precitecweldmasterguigeneral PUBLIC
    Qt5::Gui
    Qt5::Concurrent
    Precitec::precitecusermanagement
)

set(PLUGIN_SOURCES
    main.cpp
    ${SYSTEM_CONFIG_SOURCE_FILE}
)
qt_wrap_cpp(PLUGIN_SOURCES ${CMAKE_CURRENT_BINARY_DIR}/systemConfigurationQml.h TARGET precitecweldmasterguigeneralplugin DEPENDS systemConfigurationQml)
add_library(precitecweldmasterguigeneralplugin MODULE ${PLUGIN_SOURCES})
set_target_properties(precitecweldmasterguigeneralplugin PROPERTIES LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/../qml/precitec/gui/general/")
target_link_libraries(precitecweldmasterguigeneralplugin
    Qt5::Quick
    precitecweldmasterguigeneral
)
add_dependencies(precitecweldmasterguigeneralplugin systemConfigurationQml)

install(TARGETS precitecweldmasterguigeneral DESTINATION ${WM_LIB_INSTALL_DIR})
install(TARGETS precitecweldmasterguigeneralplugin DESTINATION ${WM_LIB_INSTALL_DIR}/plugins/qml/precitec/gui/general)
install(FILES qmldir DESTINATION ${WM_LIB_INSTALL_DIR}/plugins/qml/precitec/gui/general)

if (CPPUNIT_FOUND)
    add_subdirectory(autotests)
endif()
