file(GLOB RHOTHOR_LIB_SRCS
    "*.cpp"
)

add_library(rhothor SHARED ${RHOTHOR_LIB_SRCS})

set_target_properties(rhothor PROPERTIES
                COMPILE_FLAGS "-fno-strict-aliasing -Wno-error=maybe-uninitialized -Wno-error=stringop-truncation"
            )

install(TARGETS rhothor DESTINATION ${WM_LIB_INSTALL_DIR})
