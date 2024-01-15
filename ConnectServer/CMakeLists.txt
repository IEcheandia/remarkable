file(GLOB CONNECT_SERVER_SRCS
    "*.cpp"
)

if (${BUILD_ETHERCAT_MASTER})
    get_filename_component(ETHERCAT_DIR ${ETHERCAT_LIB} DIRECTORY)
else()
    set(ETHERCAT_LIB "")
endif()

configure_file(config-connectServer.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-connectServer.h)
include_directories(${CMAKE_CURRENT_BINARY_DIR})

link_libraries(stdc++fs)

add_executable(ConnectServer ${CONNECT_SERVER_SRCS})
target_link_libraries(ConnectServer ${POCO_LIBS} Interfaces)

install(TARGETS ConnectServer DESTINATION ${WM_BIN_INSTALL_DIR})

if (${CPPUNIT_FOUND})
    add_subdirectory(autotests)
endif()
