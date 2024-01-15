add_library(dxfReader STATIC
    linalg.h
    frnn.h
    dxfreader.h
    dxfreader.cpp
    pathopt.cpp
)

set_property(TARGET dxfReader PROPERTY POSITION_INDEPENDENT_CODE ON)
include_directories(${CMAKE_SOURCE_DIR}/thirdParty/tinyspline)

target_link_libraries(dxfReader
    tinyspline
    )
