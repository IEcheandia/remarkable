file(GLOB System_SRCS
    "src/*.cpp"
)

include_directories(../thirdParty/simplipc/include/)
add_library(System SHARED ${System_SRCS})
add_library(WeldMaster::System ALIAS System)
target_link_libraries(System simplIpc ${POCO_LIBS})

install(TARGETS System EXPORT WeldMasterTargets DESTINATION ${WM_LIB_INSTALL_DIR})

file(GLOB SYSTEM_MESSAGE_INCLUDES "include/message/*.h")
file(GLOB SYSTEM_MODULE_INCLUDES "include/module/*.h")
file(GLOB SYSTEM_PROTOCOL_INCLUDES "include/protocol/*.h")
file(GLOB SYSTEM_SERVER_INCLUDES "include/server/*.h")
file(GLOB SYSTEM_SYSTEM_INCLUDES "include/system/*.h")

installWeldmasterHeaders(
    FILES
        ${SYSTEM_MESSAGE_INCLUDES}
    DESTINATION
        message
)
installWeldmasterHeaders(
    FILES
        ${SYSTEM_MODULE_INCLUDES}
    DESTINATION
        module
)
installWeldmasterHeaders(
    FILES
        ${SYSTEM_PROTOCOL_INCLUDES}
    DESTINATION
        protocol
)
installWeldmasterHeaders(
    FILES
        ${SYSTEM_SERVER_INCLUDES}
    DESTINATION
        server
)
installWeldmasterHeaders(
    FILES
        ${SYSTEM_SYSTEM_INCLUDES}
    DESTINATION
        system
)
installWeldmasterHeaders(
    FILES
        include/SystemManifest.h
    DESTINATION
        ""
)
