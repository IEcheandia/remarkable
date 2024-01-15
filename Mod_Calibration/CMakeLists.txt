file(GLOB MOD_CALIBRATION_SRCS
    "src/*.cpp"
)
link_libraries(stdc++fs)

add_library(Mod_Calibration SHARED ${MOD_CALIBRATION_SRCS} )

target_link_libraries(Mod_Calibration
    PUBLIC
        Analyzer_Interface
        fliplib
        Mod_Analyzer
        CalibrationLibrary32
        stdc++fs
)

install(TARGETS Mod_Calibration DESTINATION ${WM_LIB_INSTALL_DIR})

if (BUILD_TESTING)
    add_subdirectory(autotests)
endif()
