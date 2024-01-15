file(GLOB FILTER_CALIBRATION_SRCS
    "*.cpp"
)

add_library(Filter_Calibration MODULE ${FILTER_CALIBRATION_SRCS})

target_link_libraries(Filter_Calibration PRIVATE Analyzer_Interface)

install(TARGETS Filter_Calibration DESTINATION ${WM_LIB_INSTALL_DIR})

if (BUILD_TESTING)
    add_subdirectory(autotests)
endif ()
