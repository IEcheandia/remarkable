file(GLOB PF_SRCS
    "src/*.cpp"
)

include_directories(
    include/pf/
)

add_executable(Pf ${PF_SRCS})

target_link_libraries(Pf
    ${POCO_LIBS}
        ${FgLib5_LIBRARIES}
        ${CMAKE_SOURCE_DIR}/thirdParty/photonfocus/lib/libpfcam.so
        ${CMAKE_SOURCE_DIR}/thirdParty/photonfocus/lib/libcomdll_fg.so
        ${clsersis_LIBRARY}
)

install(TARGETS Pf DESTINATION ${WM_BIN_INSTALL_DIR})
