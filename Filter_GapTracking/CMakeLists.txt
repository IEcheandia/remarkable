file(GLOB FILTER_GAPTRACKING_SRCS
    "*.cpp"
)

add_library(Filter_GapTracking MODULE ${FILTER_GAPTRACKING_SRCS})

install(TARGETS Filter_GapTracking DESTINATION ${WM_LIB_INSTALL_DIR})

if (BUILD_TESTING)
    add_subdirectory(autotests)
endif ()
