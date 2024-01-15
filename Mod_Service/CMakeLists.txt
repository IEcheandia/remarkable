file(GLOB MOD_SERVICE_SRCS
    "src/*.cpp"
)

add_library(Mod_Service SHARED ${MOD_SERVICE_SRCS})

install(TARGETS Mod_Service DESTINATION ${WM_LIB_INSTALL_DIR})
