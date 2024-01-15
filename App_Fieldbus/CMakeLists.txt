include_directories(${HILSCHER_INCLUDE_PATH})

add_executable(App_Fieldbus App_Fieldbus.cc AppMain.cpp)

target_link_libraries(App_Fieldbus
    ${POCO_LIBS}
    Framework_Module
    Mod_Fieldbus
    Interfaces
)

install(TARGETS App_Fieldbus DESTINATION ${WM_BIN_INSTALL_DIR})
