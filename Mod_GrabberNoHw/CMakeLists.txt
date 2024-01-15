file(GLOB MOD_GRABBERNOHW_SRCS
    "src/*.cpp"
    )

if (${MOD_GRABBER_DEBUG_DISPLAY})
    add_definitions(-DDEBUGDISPLAY=1)
else()
    list(REMOVE_ITEM MOD_GRABBERNOHW_SRCS "${CMAKE_CURRENT_SOURCE_DIR}/src/debugDisplay.cpp")
endif()

include_directories(
    ${FgLib5_INCLUDE_DIR}
    include/grabber/
    include/trigger/
)

add_library(Mod_GrabberNoHw SHARED ${MOD_GRABBERNOHW_SRCS})
set_target_properties(Mod_GrabberNoHw PROPERTIES EXPORT_NAME GrabberNoHw)
add_library(WeldMaster::GrabberNoHw ALIAS Mod_GrabberNoHw)

target_link_libraries(Mod_GrabberNoHw
    PUBLIC
        Interfaces
        stdc++fs # required extra library for std::filesystem on Debian 10
)

install(TARGETS Mod_GrabberNoHw EXPORT WeldMasterTargets DESTINATION ${WM_LIB_INSTALL_DIR})

file(GLOB MOD_GRABBERNOHW_INCLUDES "include/grabber/*.h")
file(GLOB MOD_GRABBERNOHW_TRIGGER_INCLUDES "include/trigger/*.h")

installWeldmasterHeaders(
    FILES
        ${MOD_GRABBERNOHW_INCLUDES}
    DESTINATION
        grabber
)

installWeldmasterHeaders(
    FILES
        ${MOD_GRABBERNOHW_TRIGGER_INCLUDES}
    DESTINATION
        trigger
)

if (BUILD_TESTING)
    add_subdirectory(autotests)
endif ()
