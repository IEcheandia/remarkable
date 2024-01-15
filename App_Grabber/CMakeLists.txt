add_executable(App_Grabber AppMain.cpp main.cc)

target_link_libraries(App_Grabber
     ${POCO_LIBS}
     Framework_Module
     Mod_Grabber
)

install(TARGETS App_Grabber DESTINATION ${WM_BIN_INSTALL_DIR})
