file(GLOB GLOBALDEFS_SRCS
    "*.cpp"
)

add_library(GlobalDefs SHARED ${GLOBALDEFS_SRCS})

install(TARGETS GlobalDefs DESTINATION ${WM_LIB_INSTALL_DIR})
