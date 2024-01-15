file(GLOB App_GrabberGigE_SRCS
    "src/*.cpp"
)

set(App_GrabberGigE_SRCS
    ../Mod_GrabberNoHw/src/sharedMemoryImageProvider.cpp
    ${App_GrabberGigE_SRCS}
)

add_executable(App_GrabberGigE ${App_GrabberGigE_SRCS})

find_package(Threads REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_search_module(GLIB REQUIRED glib-2.0)
pkg_search_module(GOBJECT REQUIRED gobject-2.0)

target_link_libraries(App_GrabberGigE
    ${GLIB_LDFLAGS}
    ${GOBJECT_LDFLAGS}
    ${POCO_LIBS}
    Framework_Module
    Interfaces
    ${ARAVIS_LIBRARIES}
    Threads::Threads
)

include_directories(
    ${ARAVIS_INCLUDE_DIRS}
    ${GLIB_INCLUDE_DIRS}
)
 
install(TARGETS App_GrabberGigE DESTINATION ${WM_BIN_INSTALL_DIR})

if (BUILD_TESTING)
    add_subdirectory(autotests)
endif()
