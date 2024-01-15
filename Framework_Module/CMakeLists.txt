set(FRAMEWORK_MODULE_SRCS
    src/baseModule.cpp
    src/module.server.cpp
    src/moduleLogger.cpp
    src/moduleManagerConnector.cpp
)

add_library(Framework_Module SHARED ${FRAMEWORK_MODULE_SRCS})
set_target_properties(Framework_Module PROPERTIES EXPORT_NAME FrameworkModule)
add_library(WeldMaster::FrameworkModule ALIAS Framework_Module)

target_link_libraries(Framework_Module
    PUBLIC
        System
)

install(TARGETS Framework_Module EXPORT WeldMasterTargets DESTINATION ${WM_LIB_INSTALL_DIR})

file(GLOB FRAMEWORK_MODULE_INCLUDES "include/module/*.h")

installWeldmasterHeaders(
    FILES
        ${FRAMEWORK_MODULE_INCLUDES}
    DESTINATION
        module
)

if (${CPPUNIT_FOUND})
   add_subdirectory(autotests)
endif()

