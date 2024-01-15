add_executable(App_TCPCommunication App_TCPCommunication.cc AppMain.cpp)

target_link_libraries(App_TCPCommunication
    ${POCO_LIBS}
    Framework_Module
    Mod_TCPCommunication
    Interfaces
)

install(TARGETS App_TCPCommunication DESTINATION ${WM_BIN_INSTALL_DIR})
