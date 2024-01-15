add_executable(App_GrabberNoHw AppMain.cpp main.cpp)

target_link_libraries(App_GrabberNoHw
     ${POCO_LIBS}
     Framework_Module
     Mod_GrabberNoHw
)

install(TARGETS App_GrabberNoHw DESTINATION ${WM_BIN_INSTALL_DIR})
