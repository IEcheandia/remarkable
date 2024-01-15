add_executable(dxf2precitec
    dxf2precitec.cpp
    run_tests.py
    dxf2svg_via_freecad.py
)

target_link_libraries(dxf2precitec
    dxfReader
)

install(TARGETS dxf2precitec DESTINATION ${WM_BIN_INSTALL_DIR})
