add_library(comdll_fg SHARED comdll_fg.c)

target_link_libraries(comdll_fg
    PUBLIC
        ${clsersis_LIBRARY}
)

install(TARGETS comdll_fg DESTINATION ${WM_LIB_INSTALL_DIR})
