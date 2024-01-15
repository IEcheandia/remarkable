include_directories(include/)

add_executable(App_ModuleManager src/moduleManager.cpp ../Filtertest/dummyLogger.cpp)

target_link_libraries(App_ModuleManager
     ${POCO_LIBS}
     Interfaces
     System
     wmLogger
     Framework_Module
     Mod_ModuleManager
)

install(TARGETS App_ModuleManager DESTINATION ${WM_BIN_INSTALL_DIR})

