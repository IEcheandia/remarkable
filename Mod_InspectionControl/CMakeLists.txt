file(GLOB MOD_INSPECTIONCONTROL_SRCS
    "src/*.cpp"
)

add_library(Mod_InspectionControl SHARED ${MOD_INSPECTIONCONTROL_SRCS})

target_link_libraries(Mod_InspectionControl
    PUBLIC
        Mod_Analyzer      
        Mod_GrabberNoHw
        Analyzer_Interface
        fliplib
        Interfaces
        ${POCO_LIBS}                
        GlobalDefs
       
)

install(TARGETS Mod_InspectionControl DESTINATION ${WM_LIB_INSTALL_DIR})

if (BUILD_TESTING)
    add_subdirectory(autotests)
endif ()

