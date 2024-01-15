set(wmLogger_SRCS
    wmLogger.cpp
    wmLogStreams.cpp
)

add_library(wmLogger SHARED ${wmLogger_SRCS})

install(TARGETS wmLogger DESTINATION ${WM_LIB_INSTALL_DIR})
