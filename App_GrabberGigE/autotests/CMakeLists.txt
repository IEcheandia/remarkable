set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
add_definitions(-DQT_NO_KEYWORDS)

qtTestCase(
    NAME
        testSharedMemoryRing
    SRCS
        testSharedMemoryRing.cpp
        ../src/camera.cpp
        ../src/parameter.cpp
        ../../Filtertest/dummyLogger.cpp
    LIBS
        ${GLIB_LDFLAGS}
        ${GOBJECT_LDFLAGS}
        ${POCO_LIBS}
        Framework_Module
        Interfaces
        ${ARAVIS_LIBRARIES}
        Threads::Threads
        Mod_GrabberNoHw
)

