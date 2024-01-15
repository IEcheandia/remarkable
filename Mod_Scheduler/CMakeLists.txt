file(GLOB MOD_SCHEDULER_SRCS
    "src/*.cpp"
)

link_libraries(stdc++fs)

include_directories(${CMAKE_SOURCE_DIR}/thirdParty/nlohmann)

add_library(Mod_Scheduler SHARED ${MOD_SCHEDULER_SRCS})

target_link_libraries(Mod_Scheduler PUBLIC ${POCO_LIBS}
        Precitec::precitecremovabledevices)
install(TARGETS Mod_Scheduler DESTINATION ${WM_LIB_INSTALL_DIR})

add_subdirectory(src/SchedulerHelper)

if (BUILD_TESTING)
    add_subdirectory(autotests)
endif ()
