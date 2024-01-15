file(GLOB MOD_WORKFLOW_SRCS
    "src/*.cpp"
)

add_library(Mod_Workflow SHARED ${MOD_WORKFLOW_SRCS})

install(TARGETS Mod_Workflow DESTINATION ${WM_LIB_INSTALL_DIR})
