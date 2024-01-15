file(GLOB MOD_TRIGGER_SRCS
    "src/*.cpp"
)

include_directories(${PRECIENCODER_INCLUDE_PATH})

add_library(Mod_Trigger SHARED ${MOD_TRIGGER_SRCS})

target_link_libraries(Mod_Trigger 
    PUBLIC
        GlobalDefs
)

install(TARGETS Mod_Trigger DESTINATION ${WM_LIB_INSTALL_DIR})
