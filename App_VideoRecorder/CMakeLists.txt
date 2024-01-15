add_executable(App_VideoRecorder AppMain.cpp main.cc)

target_link_libraries(App_VideoRecorder
     ${POCO_LIBS}
     Framework_Module
     Interfaces
     Mod_VideoRecorder
)

install(TARGETS App_VideoRecorder DESTINATION ${WM_BIN_INSTALL_DIR})
