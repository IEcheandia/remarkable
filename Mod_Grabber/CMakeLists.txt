file(GLOB MOD_GRABBER_SRCS
    "src/*.cpp"
)

if (${MOD_GRABBER_DEBUG_DISPLAY})
    add_definitions(-DDEBUGDISPLAY=1)
else()
    list(REMOVE_ITEM MOD_GRABBER_SRCS "${CMAKE_CURRENT_SOURCE_DIR}/src/debugDisplay.cpp")
endif()

include_directories(
    ${FgLib5_INCLUDE_DIR}
    include/grabber/
    include/trigger/
)

add_library(Mod_Grabber SHARED ${MOD_GRABBER_SRCS})

target_link_libraries(Mod_Grabber
    PUBLIC
        Interfaces
        ${FgLib5_LIBRARIES}
        ${CMAKE_SOURCE_DIR}/thirdParty/photonfocus/lib/libpfcam.so
        ${CMAKE_SOURCE_DIR}/thirdParty/photonfocus/lib/libcomdll_fg.so
        ${clsersis_LIBRARY}
        Mod_GrabberNoHw
)

if (${MOD_GRABBER_DEBUG_DISPLAY})
	target_link_libraries(Mod_Grabber
		PRIVATE
			XCB::XCB
			XCB::ICCCM
	)
endif()

install(TARGETS Mod_Grabber DESTINATION ${WM_LIB_INSTALL_DIR})

if (${CPPUNIT_FOUND})
    add_subdirectory(autotests)
endif()
