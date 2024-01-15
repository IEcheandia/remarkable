set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)

file(GLOB FLIPLIB_SRCS
    "src/*.cpp"
)

add_library(fliplib SHARED ${FLIPLIB_SRCS})
add_library(WeldMaster::fliplib ALIAS fliplib)
target_link_libraries(fliplib ${POCO_LIBS})
install(TARGETS fliplib EXPORT WeldMasterTargets DESTINATION ${WM_LIB_INSTALL_DIR})

if (BUILD_TESTING)
    add_subdirectory(autotests)
endif ()
