include_directories(${ETHERCAT_INCLUDE_PATH})

add_executable(App_EtherCATMaster App_EtherCATMaster.cc AppMain.cpp)

target_link_libraries(App_EtherCATMaster
    ${POCO_LIBS}
    Framework_Module
    Mod_EtherCATMaster
    Interfaces
)

install(TARGETS App_EtherCATMaster DESTINATION ${WM_BIN_INSTALL_DIR})
