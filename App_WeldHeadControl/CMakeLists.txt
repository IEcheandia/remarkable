add_executable(App_WeldHeadControl AppMain.cpp VI_WeldHeadControl.cc)

include_directories(${SCANLAB_INCLUDE_PATH})

target_link_libraries(App_WeldHeadControl
     ${POCO_LIBS}
     Framework_Module
     Interfaces
     Mod_WeldHeadControl
)

install(TARGETS App_WeldHeadControl DESTINATION ${WM_BIN_INSTALL_DIR})
