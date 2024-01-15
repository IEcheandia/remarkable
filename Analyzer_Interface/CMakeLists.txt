# uses the register keyword which is not allowed in C++17
set(CMAKE_CXX_STANDARD 14)

file(GLOB ANALYZERINTERFACE_SRCS
    "src/*.cpp"
    "src/math/*.cpp"
    "src/math/2D/*.cpp"
    "src/math/3D/*.cpp"
    "src/math/tinyexpr.c"
    "src/memory/memoryFile.cpp"
)

add_library(Analyzer_Interface SHARED ${ANALYZERINTERFACE_SRCS})
set_target_properties(Analyzer_Interface PROPERTIES EXPORT_NAME AnalyzerInterface)
add_library(WeldMaster::AnalyzerInterface ALIAS Analyzer_Interface)

target_link_libraries(Analyzer_Interface
    PUBLIC
        Interfaces
        opencv_core opencv_imgproc
)

install(TARGETS Analyzer_Interface EXPORT WeldMasterTargets DESTINATION ${WM_LIB_INSTALL_DIR})


file(GLOB ANALYZER_COORDINATES_INCLUDES "include/coordinates/*.h")
file(GLOB ANALYZER_FILTER_INCLUDES "include/filter/*.h")
file(GLOB ANALYZER_IMAGE_INCLUDES "include/image/*.h")
file(GLOB ANALYZER_MATH_INCLUDES "include/math/*.h")
file(GLOB ANALYZER_MEMORY_INCLUDES "include/memory/*.h")
file(GLOB ANALYZER_UTIL_INCLUDES "include/util/*.h")

installWeldmasterHeaders(
    FILES
        ${ANALYZER_COORDINATES_INCLUDES}
    DESTINATION
        coordinates
)
installWeldmasterHeaders(
    FILES
        ${ANALYZER_FILTER_INCLUDES}
    DESTINATION
        filter
)
installWeldmasterHeaders(
    FILES
        ${ANALYZER_IMAGE_INCLUDES}
    DESTINATION
        image
)
installWeldmasterHeaders(
    FILES
        ${ANALYZER_MATH_INCLUDES}
    DESTINATION
        math
)
installWeldmasterHeaders(
    FILES
        ${ANALYZER_MEMORY_INCLUDES}
    DESTINATION
        memory
)
installWeldmasterHeaders(
    FILES
        ${ANALYZER_UTIL_INCLUDES}
    DESTINATION
        util
)

installWeldmasterHeaders(
    FILES
        include/Analyzer_Interface.h
    DESTINATION
        ""
)

if (BUILD_TESTING)
    add_subdirectory(autotests)
endif ()
