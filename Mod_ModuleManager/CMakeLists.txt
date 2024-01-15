file(GLOB MOD_MODULEMANAGER_SRCS
    "src/*.cpp"
)

add_library(Mod_ModuleManager SHARED ${MOD_MODULEMANAGER_SRCS})

install(TARGETS Mod_ModuleManager DESTINATION ${WM_LIB_INSTALL_DIR})
