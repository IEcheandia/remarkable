set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)

set(App_MockAxis_SRCS
    src/main.cpp
    src/module.cpp
)
qt5_add_resources(App_MockAxis_SRCS resources.qrc)

add_executable(App_MockAxis ${App_MockAxis_SRCS})

target_link_libraries(App_MockAxis
    Qt5::Concurrent
    Qt5::Gui
    Qt5::Quick
    Qt5::QuickControls2
    ${POCO_LIBS}
    Framework_Module
    Interfaces
)
