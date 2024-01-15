configure_file(weldmaster.desktop.in weldmaster.desktop)
install(
    FILES
        ${CMAKE_CURRENT_BINARY_DIR}/weldmaster.desktop
    DESTINATION
        etc/xdg/autostart
)
configure_file(weldmaster.sh.in weldmaster.sh)
install(
    FILES
        ${CMAKE_CURRENT_BINARY_DIR}/weldmaster.sh
    DESTINATION
        etc/profile.d
)

configure_file(etc/systemd/system/disable-hyper-threading.service.in disable-hyper-threading.service)
configure_file(etc/systemd/system/cpugov-performance.service.in cpugov-performance.service)
configure_file(etc/systemd/system/weldmaster-network-devices.service.in weldmaster-network-devices.service)
configure_file(etc/systemd/system/weldmaster-nftables.service.in weldmaster-nftables.service)
configure_file(etc/systemd/system/weldmaster-mount-backup.service.in weldmaster-mount-backup.service)
install(
    FILES
        ${CMAKE_CURRENT_BINARY_DIR}/disable-hyper-threading.service
        ${CMAKE_CURRENT_BINARY_DIR}/cpugov-performance.service
        ${CMAKE_CURRENT_BINARY_DIR}/weldmaster-network-devices.service
        ${CMAKE_CURRENT_BINARY_DIR}/weldmaster-nftables.service
        ${CMAKE_CURRENT_BINARY_DIR}/weldmaster-mount-backup.service
    DESTINATION
        etc/systemd/system/
)

install(
    FILES
        etc/systemd/system/autovt@.service.d/99-weldmaster.conf
    DESTINATION
        etc/systemd/system/autovt@.service.d
)

install(
    FILES
        etc/systemd/system/mnt-backup.mount
    DESTINATION
        etc/systemd/system
)

install(
    FILES
        etc/systemd/system/getty@tty1.service.d/99-weldmaster.conf
    DESTINATION
        etc/systemd/system/getty@tty1.service.d
)

install(
    FILES
        etc/systemd/coredump.conf.d/99-weldmaster.conf
    DESTINATION
        etc/systemd/coredump.conf.d
)

install(
    FILES
        batch/config-updater.sh
        batch/runC
        batch/runPF
        batch/runStandalone
        batch/setupFifo.sh
        batch/start-gs.sh
        batch/stopAll.sh
        batch/packLogFiles
        batch/packSingleLogFile
        batch/disable-hyper-threading.sh
        batch/cpugov-performance.sh
        batch/restartConnectServer.sh
        batch/runFiltertest
        batch/install-sub-graphs.sh
        batch/install-macros.sh
        batch/install-system-graphs.sh
        batch/mount_backup.sh
    PERMISSIONS
        OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
    DESTINATION
        ${WM_INSTALL_DIR}/batch/
)

configure_file(batch/firewall.sh.in firewall.sh @ONLY)
install(
    FILES
        ${CMAKE_CURRENT_BINARY_DIR}/firewall.sh
    PERMISSIONS
        OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
    DESTINATION
        ${WM_INSTALL_DIR}/batch/
)

install(
    FILES
        batch/prepare-ethercat
    PERMISSIONS
        OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
    DESTINATION
        opt/
)


install(
    FILES
        batch/notifycmd
    PERMISSIONS
        OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
    DESTINATION
        usr/sbin/
)

install(
    FILES
        calib/GraphCalibrationNoAxis.xml
        calib/GraphCalibrationLED.xml
        calib/GraphCalibrationScanField.xml
        calib/IntelliScanIII30_F_Theta_255.ct5
        calib/IntelliScanIII30_F_Theta_255Pilot.ct5
        calib/IntelliScanIII30_F_Theta_255HeightMeasurement.ct5
        calib/IntelliScanIII30_F_Theta_340.ct5
        calib/IntelliScanIII30_F_Theta_340Pilot.ct5
        calib/IntelliScanIII30_F_Theta_340HeightMeasurement.ct5
        calib/IntelliScanIII30_F_Theta_460.ct5
        calib/IntelliScanIII30_F_Theta_460Pilot.ct5
        calib/IntelliScanIII30_F_Theta_460HeightMeasurement.ct5
        calib/ME_F255.sbd
        calib/ME_F340.sbd
        calib/ME_F460.sbd
    DESTINATION
        ${WM_INSTALL_DIR}/calib/
)

install(
    DIRECTORY
        config_templates
    DESTINATION
        ${WM_INSTALL_DIR}
    PATTERN "oct_track" EXCLUDE
    PATTERN "track_compact" EXCLUDE
    PATTERN "souvis6000" EXCLUDE
    PATTERN "scanmaster" EXCLUDE
    PATTERN "scanwelder2d" EXCLUDE
)

install(
    DIRECTORY
        pdfs
    DESTINATION
        ${WM_INSTALL_DIR}
)

install(
    FILES
        config_templates/defaultProduct.json
    DESTINATION
        ${WM_INSTALL_DIR}/config/products/
)

install(
    FILES
        config_templates/qualityNorms.json
    DESTINATION
        ${WM_INSTALL_DIR}/config/
)

install(
    DIRECTORY
        config_templates/oct_track/
    DESTINATION
        ${WM_INSTALL_DIR}/config_oct_track/
)

install(
    DIRECTORY
        config_templates/track_compact/
    DESTINATION
        ${WM_INSTALL_DIR}/config_track_compact/
)

install(
    DIRECTORY
        config_templates/souvis6000/
    DESTINATION
        ${WM_INSTALL_DIR}/config_souvis6000/
)

install(
    DIRECTORY
        config_templates/scanmaster/
    DESTINATION
        ${WM_INSTALL_DIR}/config_scanmaster/
)

install(
    DIRECTORY
        config_templates/scanwelder2d/
    DESTINATION
        ${WM_INSTALL_DIR}/config_scanwelder2d/
)

option(LINK_SYSTEMGRAPHSSOURCE "LINK SYSTEMGRAPHS SOURCE FOLDER" OFF)

macro(install_symlink filepath sympath)
    install(CODE "execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink ${filepath} ${sympath})")
    install(CODE "message(\"-- Created symlink: ${sympath} -> ${filepath}\")")
endmacro(install_symlink)

if (${LINK_SYSTEMGRAPHSSOURCE})
    install_symlink(${CMAKE_CURRENT_SOURCE_DIR}/system_graphs ${CMAKE_INSTALL_PREFIX}/${WM_INSTALL_DIR}/system_graphs )
else()
    install(
        DIRECTORY
            system_graphs
        DESTINATION
            ${WM_INSTALL_DIR}
    )
endif()


install(
    FILES
        etc/security/limits/99-weldmaster.conf
    DESTINATION
        etc/security/limits.d/
)

install(
    FILES
        etc/modules-load.d/preciRS422.conf
        etc/modules-load.d/menable.conf
    DESTINATION
        etc/modules-load.d/
)

install(
    FILES
        etc/udev/rules.d/99-cpudma.rules
    DESTINATION
        etc/udev/rules.d/
)

file(GLOB MODULE_LANGUAGE_FILES
    "languages/*.xml"
)

install(
    DIRECTORY
        languages/filterImages/
    DESTINATION
        ${WM_INSTALL_DIR}/languages/filterImages/
)

install(
    DIRECTORY
        languages/images/
    DESTINATION
        ${WM_INSTALL_DIR}/languages/images/
)

install(
    FILES
        ${MODULE_LANGUAGE_FILES}
    DESTINATION
        ${WM_INSTALL_DIR}/languages/
)

install(
    FILES
        ../../deploy/version.ini
    DESTINATION
        ${WM_INSTALL_DIR}/config/
)

install(
    FILES
        etc/modprobe.d/weldmaster-blacklist.conf
        etc/modprobe.d/audio-blacklist.conf
        etc/modprobe.d/bluetooth-blacklist.conf
    DESTINATION
        etc/modprobe.d/
)

install(
    FILES
        etc/sysctl.d/99-weldmaster.conf
    DESTINATION
        etc/sysctl.d/
)

install(
    FILES
        etc/xdg/kwinrc
        etc/xdg/kscreenlockerrc
        etc/xdg/krfbrc
        etc/xdg/kcminputrc
    DESTINATION
        etc/weldmaster-kiosk/
)

install(
    FILES
        etc/sudoers.d/99-ethercat
    DESTINATION
        etc/sudoers.d/
)

install(
    FILES
        etc/NetworkManager/conf.d/99-nm-weldmaster.conf
    DESTINATION
        etc/NetworkManager/conf.d/
)

install(
    FILES
        etc/NetworkManager/system-connections/Peripheral
        etc/NetworkManager/system-connections/Weldmaster
        etc/NetworkManager/system-connections/GigE-Camera
    DESTINATION
        etc/NetworkManager/system-connections/
)

install(
    FILES
        etc/NetworkManager/configure-devices.sh
        etc/NetworkManager/configure-network-devices.sh
        etc/NetworkManager/configure-gige.sh
    DESTINATION
        opt/
    PERMISSIONS
        OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
)

install(
    FILES
        etc/dnsmasq.d/weldmaster
    DESTINATION
        etc/dnsmasq.d/
)

install(
    FILES
        etc/weldmaster-hosts
    DESTINATION
        etc/
)

install(
    FILES
        etc/nftables/weldmaster.nft
    DESTINATION
        etc/nftables/
)

configure_file(etc/openvpn/server/weldmaster.conf.in weldmaster.conf)
install(
    FILES
        ${CMAKE_CURRENT_BINARY_DIR}/weldmaster.conf
    DESTINATION
        etc/openvpn/server/
)


install(
    FILES
        etc/network/if-up.d/weldmaster-tun
        etc/network/if-up.d/weldmaster-gige
        etc/network/if-up.d/weldmaster-wireguard
    DESTINATION
        etc/network/if-up.d/
    PERMISSIONS
        OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
)
