file(GLOB MOD_FIELDBUS_SRCS
    "src/*.cpp"
)

include_directories(${HILSCHER_INCLUDE_PATH})

add_library(Mod_Fieldbus SHARED ${MOD_FIELDBUS_SRCS})
target_link_libraries(Mod_Fieldbus 
    PUBLIC
        ${HILSCHER_LIB}
        GlobalDefs
)

install(TARGETS Mod_Fieldbus DESTINATION ${WM_LIB_INSTALL_DIR})
