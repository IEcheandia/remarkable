include_directories(${CMAKE_CURRENT_SOURCE_DIR})

add_executable(App_LoggerServer appLoggerServer.cpp)

target_link_libraries(App_LoggerServer
     ${POCO_LIBS}
     Framework_Module
     Interfaces
)

install(TARGETS App_LoggerServer DESTINATION ${WM_BIN_INSTALL_DIR})
