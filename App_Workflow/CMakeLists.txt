add_executable(App_Workflow AppMain.cpp main.cc)

target_link_libraries(App_Workflow
    ${POCO_LIBS}
    fliplib
    Framework_Module
    Mod_Analyzer
    Mod_InspectionControl
    Mod_Workflow
    System
)

install(TARGETS App_Workflow DESTINATION ${WM_BIN_INSTALL_DIR})
