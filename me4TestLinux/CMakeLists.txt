file(GLOB ME4TESTLINUX_SRCS
    "src/*.cpp"
)

include_directories(
    include/me4TestLinux/
)

add_executable(me4TestLinux ${ME4TESTLINUX_SRCS})

target_link_libraries(me4TestLinux
    pthread
    ${POCO_LIBS}
    ${FgLib5_LIBRARIES}
    ${CMAKE_SOURCE_DIR}/thirdParty/photonfocus/lib/libpfcam.so
    ${CMAKE_SOURCE_DIR}/thirdParty/photonfocus/lib/libcomdll_fg.so
    ${clsersis_LIBRARY}
)

install(TARGETS me4TestLinux DESTINATION ${WM_BIN_INSTALL_DIR})
