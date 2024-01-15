file(GLOB FILTER_LINEGEOMETRY_SRCS
    "*.cpp"
)

add_library(Filter_LineGeometry MODULE ${FILTER_LINEGEOMETRY_SRCS})

install(TARGETS Filter_LineGeometry DESTINATION ${WM_LIB_INSTALL_DIR})

if (BUILD_TESTING)
    add_subdirectory(autotests)
endif ()
