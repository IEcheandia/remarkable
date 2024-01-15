add_library(simplIpc SHARED simpl.cpp simplUtils.cpp)
add_library(WeldMaster::simplIpc ALIAS simplIpc)
install(TARGETS simplIpc EXPORT WeldMasterTargets DESTINATION ${WM_LIB_INSTALL_DIR})
