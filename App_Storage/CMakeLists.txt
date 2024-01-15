set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)

include_directories(../Mod_Storage/src)

set(App_Storage_SRCS
    src/main.cpp
    src/module.cpp
    src/deviceServer.cpp
    src/dbServer.cpp
    src/storageUpdateServer.cpp
    src/productChangeNotifier.cpp
)

add_executable(App_Storage ${App_Storage_SRCS})

target_link_libraries(App_Storage
    Qt5::Core
    ${POCO_LIBS}
    Framework_Module
    Interfaces
    Mod_Storage
    fliplib
)

install(TARGETS App_Storage DESTINATION ${WM_BIN_INSTALL_DIR})

if (BUILD_TESTING)
    add_subdirectory(autotests)
endif()
