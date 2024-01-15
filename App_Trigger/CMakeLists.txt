include_directories(${PRECIENCODER_INCLUDE_PATH})

add_executable(App_Trigger App_Trigger.cc AppMain.cpp)

target_link_libraries(App_Trigger
    ${POCO_LIBS}
    Framework_Module
    Mod_Trigger
    Interfaces
)

install(TARGETS App_Trigger DESTINATION ${WM_BIN_INSTALL_DIR})
