file(GLOB FILTER_IMAGEPROCESSING_SRCS
    "*.cpp"
)

add_library(Filter_ImageProcessing MODULE ${FILTER_IMAGEPROCESSING_SRCS})

target_link_libraries(
    Filter_ImageProcessing
    opencv_core opencv_imgproc
)

install(TARGETS Filter_ImageProcessing DESTINATION ${WM_LIB_INSTALL_DIR})

if (BUILD_TESTING)
    add_subdirectory(autotests)
endif ()
