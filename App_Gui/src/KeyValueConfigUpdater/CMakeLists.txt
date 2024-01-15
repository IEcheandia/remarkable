set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(etherCATConfigurationHelper
    etherCATConfigurationHelper.cpp
    etherCATStartupUpdater.cpp
    etherCATConfigUpdater.cpp
    keyValueConfigUpdater.cpp
)
target_link_libraries(etherCATConfigurationHelper Qt5::Core)


add_executable(nutConfigurationHelper
    nutConfigurationHelper.cpp
    nutConfigUpdater.cpp
    keyValueConfigUpdater.cpp
)
target_link_libraries(nutConfigurationHelper Qt5::Core)

install(
    TARGETS
        etherCATConfigurationHelper
        nutConfigurationHelper
    DESTINATION
        ${WM_BIN_INSTALL_DIR}
    PERMISSIONS
        OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE SETUID
)
