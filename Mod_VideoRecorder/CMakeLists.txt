include_directories(${CMAKE_SOURCE_DIR}/thirdParty/nlohmann)

file(GLOB MOD_VIDEORECORDER_SRCS
    "src/*.cpp"
)

add_library(Mod_VideoRecorder SHARED ${MOD_VIDEORECORDER_SRCS})
set_target_properties(Mod_VideoRecorder PROPERTIES EXPORT_NAME VideoRecorder)
target_link_libraries(Mod_VideoRecorder stdc++fs) # required for the old GCC on Debian 10
add_library(WeldMaster::VideoRecorder ALIAS Mod_VideoRecorder)

install(TARGETS Mod_VideoRecorder EXPORT WeldMasterTargets DESTINATION ${WM_LIB_INSTALL_DIR})
