file(GLOB FILTER_LINETRACKING_SRCS
    "*.cpp"
)

add_library(Filter_LineTracking MODULE ${FILTER_LINETRACKING_SRCS})

install(TARGETS Filter_LineTracking DESTINATION ${WM_LIB_INSTALL_DIR})

if (BUILD_TESTING)
    add_subdirectory(autotests)
endif ()
