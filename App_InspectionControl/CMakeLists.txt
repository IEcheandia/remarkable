add_executable(App_InspectionControl AppMain.cpp VI_Main.cc)

target_link_libraries(App_InspectionControl
     ${POCO_LIBS}     
     Interfaces
     Mod_InspectionControl     
)

install(TARGETS App_InspectionControl DESTINATION ${WM_BIN_INSTALL_DIR})
