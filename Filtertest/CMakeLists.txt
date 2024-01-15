set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
add_definitions(-DHAVE_QT)

find_package(Gperftools)

add_library(dummyLogger STATIC dummyLogger.cpp)

if (Gperftools_FOUND)
    option(GPERF_PROFILE "Enable GPerf Tools" OFF)
    if (${GPERF_PROFILE})
        include_directories(${GPERFTOOLS_INCLUDE_DIR})
        set(PROFILER_LIB profiler) 
        add_definitions(-DHAS_GPERFTOOLS)
    endif()
endif()


option(CALLGRIND_PROFILE "Turn on callgrind instrumentation only during graph processing" OFF)

if (${CALLGRIND_PROFILE})
    include_directories("/usr/include")
    add_definitions(-DCALLGRIND_PROFILE)
endif()


add_executable(filtertest dummyLogger.cpp graphManager.cpp main.cpp resultHandler.cpp QtCanvas.cpp)

target_link_libraries(filtertest
        ${POCO_LIBS}
        fliplib
        Analyzer_Interface
        Mod_Analyzer
        ${POCO_XML}
        Framework_Module
        Qt5::Gui
        ${PROFILER_LIB}
)

install(TARGETS filtertest DESTINATION ${WM_BIN_INSTALL_DIR})
install(TARGETS dummyLogger DESTINATION ${WM_LIB_INSTALL_DIR})
