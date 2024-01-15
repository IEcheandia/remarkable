add_executable(App_Scheduler App_Scheduler.cc AppMain.cpp)

include_directories(${CMAKE_SOURCE_DIR}/thirdParty/nlohmann)

target_link_libraries(App_Scheduler
    ${POCO_LIBS}
    Framework_Module
    Mod_Scheduler
    Interfaces
)

install(TARGETS App_Scheduler DESTINATION ${WM_BIN_INSTALL_DIR})
