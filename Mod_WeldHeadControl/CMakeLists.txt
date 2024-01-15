file(GLOB MOD_WELDHEADCONTROL_SRCS
    "src/*.cpp"
    "src/LEDControl/*.cpp"
    "src/Scanlab/*.cpp"
    "src/ZCollimatorV2/*.c"
    "src/ZCollimatorV2/*.cpp"
)
include_directories(${CMAKE_SOURCE_DIR}/thirdParty/nlohmann)

include_directories(${PRECIRS422_INCLUDE_PATH})

include_directories(${SCANLAB_INCLUDE_PATH})

add_library(Mod_WeldHeadControl SHARED ${MOD_WELDHEADCONTROL_SRCS})

target_link_libraries(Mod_WeldHeadControl
    PUBLIC
        ${SCANLAB_LIB}
        Analyzer_Interface
        GlobalDefs
)

install(TARGETS Mod_WeldHeadControl DESTINATION ${WM_LIB_INSTALL_DIR})

if (BUILD_TESTING)
    add_subdirectory(autotests)
endif ()
