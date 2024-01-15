file(GLOB FILTER_POREANALYSIS_SRCS
    "*.cpp"
)

add_library(Filter_PoreAnalysis MODULE ${FILTER_POREANALYSIS_SRCS})

install(TARGETS Filter_PoreAnalysis DESTINATION ${WM_LIB_INSTALL_DIR})

if (BUILD_TESTING)
    add_subdirectory(autotests)
endif ()
