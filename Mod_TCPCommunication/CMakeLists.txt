file(GLOB MOD_TCPCOMMUNICATION_SRCS
    "src/*.cpp"
)

include_directories(${CMAKE_SOURCE_DIR}/thirdParty/nlohmann)

add_library(Mod_TCPCommunication SHARED ${MOD_TCPCOMMUNICATION_SRCS})

target_link_libraries(Mod_TCPCommunication 
    PUBLIC
        GlobalDefs
)

install(TARGETS Mod_TCPCommunication DESTINATION ${WM_LIB_INSTALL_DIR})
