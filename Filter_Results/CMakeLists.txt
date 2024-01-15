file(GLOB FILTER_RESULT_SRCS
    "*.cpp"
)

add_library(Filter_Results MODULE ${FILTER_RESULT_SRCS})

install(TARGETS Filter_Results DESTINATION ${WM_LIB_INSTALL_DIR})

if (BUILD_TESTING)
    add_subdirectory(autotests)
endif ()

