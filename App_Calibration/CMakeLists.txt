add_executable(App_Calibration AppMain.cpp main.cc)

target_link_libraries(App_Calibration
     ${POCO_LIBS}
     Framework_Module
     Interfaces
     Mod_Calibration
)

install(TARGETS App_Calibration DESTINATION ${WM_BIN_INSTALL_DIR})


