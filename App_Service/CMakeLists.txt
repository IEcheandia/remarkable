add_executable(App_Service AppMain.cpp Service_VI.cc)

target_link_libraries(App_Service
     ${POCO_LIBS}
     Framework_Module
     Interfaces
     Mod_Service
)

install(TARGETS App_Service DESTINATION ${WM_BIN_INSTALL_DIR})
