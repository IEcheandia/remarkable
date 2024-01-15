file(GLOB FILTER_SEAMSEARCH_SRCS
    "*.cpp"
)

add_library(Filter_SeamSearch MODULE ${FILTER_SEAMSEARCH_SRCS})

install(TARGETS Filter_SeamSearch DESTINATION ${WM_LIB_INSTALL_DIR})

if (BUILD_TESTING)
    add_subdirectory(autotests)
endif ()
