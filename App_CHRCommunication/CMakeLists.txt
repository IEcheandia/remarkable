include_directories(${CHROCODILE_INCLUDE_PATH})

add_executable(App_CHRCommunication App_CHRCommunication.cc AppMain.cpp)

target_link_libraries(App_CHRCommunication
    ${POCO_LIBS}
    Framework_Module
    Interfaces
    Mod_CHRCommunication
)

install(TARGETS App_CHRCommunication DESTINATION ${WM_BIN_INSTALL_DIR})
