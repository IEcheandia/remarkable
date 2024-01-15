file(GLOB FILTER_UTILITY_SRCS
    "*.cpp"
    "lua/*.cpp"
)
include_directories(${CMAKE_SOURCE_DIR}/thirdParty/nlohmann)
include_directories(${CMAKE_SOURCE_DIR}/thirdParty/luaLib/include)
include_directories(${CMAKE_SOURCE_DIR}/thirdParty/vince_csv)

add_library(Filter_Utility MODULE ${FILTER_UTILITY_SRCS})
target_link_libraries(Filter_Utility
    PUBLIC
        luaLib
        stdc++fs # neccessary for old GCC under Debian 10
)
# valueFromCSVFile uses a csv parser library which uses std::thread.
# To compile this with GCC we need the pthread linker flag.
target_link_options(Filter_Utility PRIVATE -pthread)

install(TARGETS Filter_Utility DESTINATION ${WM_LIB_INSTALL_DIR})

if (BUILD_TESTING)
    add_subdirectory(autotests)
endif ()
