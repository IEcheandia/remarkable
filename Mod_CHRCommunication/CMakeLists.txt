file(GLOB MOD_CHRCOMMUNICATION_SRCS
    "src/*.cpp"
)

include_directories(${CHROCODILE_INCLUDE_PATH})

add_library(Mod_CHRCommunication SHARED ${MOD_CHRCOMMUNICATION_SRCS})

target_link_libraries(Mod_CHRCommunication 
    PUBLIC
        ${CHROCODILE_LIB}
)

install(TARGETS Mod_CHRCommunication DESTINATION ${WM_LIB_INSTALL_DIR})
