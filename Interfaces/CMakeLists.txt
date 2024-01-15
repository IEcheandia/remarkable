file(GLOB INTERFACES_SRCS
    "src/*.cpp"
)

add_library(Interfaces SHARED ${INTERFACES_SRCS})
add_library(WeldMaster::Interfaces ALIAS Interfaces)

target_link_libraries(Interfaces
    PUBLIC
        Framework_Module
        System
)

install(TARGETS Interfaces EXPORT WeldMasterTargets DESTINATION ${WM_LIB_INSTALL_DIR})

file(GLOB INTERFACES_COMMON_INCLUDES "include/common/*.h")
file(GLOB INTERFACES_EVENT_INCLUDES "include/event/*.h")
file(GLOB INTERFACES_FILTER_INCLUDES "include/filter/*.h")
file(GLOB INTERFACES_GEO_INCLUDES "include/geo/*.h*")
file(GLOB INTERFACES_MESSAGE_INCLUDES "include/message/*.h")
file(GLOB INTERFACES_OVERLAY_INCLUDES "include/overlay/*.h")

installWeldmasterHeaders(
    FILES
        ${INTERFACES_COMMON_INCLUDES}
    DESTINATION
        common
)
installWeldmasterHeaders(
    FILES
        ${INTERFACES_EVENT_INCLUDES}
    DESTINATION
        event
)
installWeldmasterHeaders(
    FILES
        ${INTERFACES_FILTER_INCLUDES}
    DESTINATION
        filter
)
installWeldmasterHeaders(
    FILES
        ${INTERFACES_GEO_INCLUDES}
    DESTINATION
        geo
)
installWeldmasterHeaders(
    FILES
        ${INTERFACES_MESSAGE_INCLUDES}
    DESTINATION
        message
)
installWeldmasterHeaders(
    FILES
        ${INTERFACES_OVERLAY_INCLUDES}
    DESTINATION
        overlay
)
installWeldmasterHeaders(
    FILES
        include/InterfacesManifest.h
    DESTINATION
        ""
)

if (${CPPUNIT_FOUND})
    add_subdirectory(autotests)
endif()
