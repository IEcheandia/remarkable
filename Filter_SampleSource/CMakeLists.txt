file(GLOB FILTER_SAMPLESOURCE_SRCS
    "*.cpp"
)

add_library(Filter_SampleSource MODULE ${FILTER_SAMPLESOURCE_SRCS})

install(TARGETS Filter_SampleSource DESTINATION ${WM_LIB_INSTALL_DIR})
