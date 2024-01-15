file(GLOB MOD_ETHERCATMASTER_SRCS
    "src/*.cpp"
)

include_directories(${ETHERCAT_INCLUDE_PATH})

add_library(Mod_EtherCATMaster SHARED ${MOD_ETHERCATMASTER_SRCS})
target_link_libraries(Mod_EtherCATMaster 
    PUBLIC
        ${ETHERCAT_LIB}
        GlobalDefs
)

install(TARGETS Mod_EtherCATMaster DESTINATION ${WM_LIB_INSTALL_DIR})
