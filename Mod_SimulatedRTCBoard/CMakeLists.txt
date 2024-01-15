set(MOD_SIMULATEDRTCBOARD_SRCS
    src/log.cpp
    src/rtc6.cpp
)

include_directories(${SCANLAB_INCLUDE_PATH})

add_library(Mod_SimulatedRTCBoard SHARED ${MOD_SIMULATEDRTCBOARD_SRCS})
target_link_libraries(Mod_SimulatedRTCBoard
    ${POCO_LIBS}
)



install(TARGETS Mod_SimulatedRTCBoard DESTINATION ${WM_LIB_INSTALL_DIR})



