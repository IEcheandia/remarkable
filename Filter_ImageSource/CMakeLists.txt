add_library(Filter_ImageSource MODULE imageSource.cpp sampleToImageSource.cpp manifest.cpp)

install(TARGETS Filter_ImageSource DESTINATION ${WM_LIB_INSTALL_DIR})
