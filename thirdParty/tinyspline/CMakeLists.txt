set(CMAKE_CXX_STANDARD 14)

add_library(tinyspline STATIC
    parson.c
    parson.h
    tinyspline.c
    tinysplinecxx.cxx
    tinysplinecxx.h
    tinyspline.h
)

set_property(TARGET tinyspline PROPERTY POSITION_INDEPENDENT_CODE ON)
