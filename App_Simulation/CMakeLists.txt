include_directories(include)

add_executable(App_Simulation
    AppMain.cpp
    main.cpp
    commandServer.cpp
)

target_link_libraries(App_Simulation
    ${POCO_LIBS}
    fliplib
    Framework_Module
    Mod_Analyzer
    Mod_InspectionControl
    Mod_GrabberNoHw
    Mod_Workflow
    System
)

install(TARGETS App_Simulation DESTINATION ${WM_BIN_INSTALL_DIR})

if (${CPPUNIT_FOUND})
    add_subdirectory(autotests)
endif()
