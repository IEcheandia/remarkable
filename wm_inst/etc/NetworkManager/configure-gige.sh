#!/bin/bash

for ifName in `ls /sys/class/net/`; do
    if [ "${ifName}" = "lo" ]; then
        continue
    fi

    vendor=`cat /sys/class/net/${ifName}/device/vendor`
    device=`cat /sys/class/net/${ifName}/device/device`
    macAddress=`cat /sys/class/net/${ifName}/address`
    if [ "${vendor}" = "0x1fc9" ]; then
        # Tehuti
        if [ "${device}" = "0x4027" ] || [ "${device}" = "0x4022" ]; then
            # TN9710P and Edimax
            sed -i "s/interface-name=/interface-name=${ifName}/" /etc/NetworkManager/system-connections/GigE-Camera
            sed -i "s/mac-address=/mac-address=${macAddress}/" /etc/NetworkManager/system-connections/GigE-Camera
            chmod 600 /etc/NetworkManager/system-connections/GigE-Camera
        fi
    fi
    if [ "${vendor}" = "0x1d6a" ]; then
        # Acquatia/Marvell
        if [ "${device}" = "0x07b1" ] || [ "${device}" = "0x00b1" ]; then
            sed -i "s/interface-name=/interface-name=${ifName}/" /etc/NetworkManager/system-connections/GigE-Camera
            sed -i "s/mac-address=/mac-address=${macAddress}/" /etc/NetworkManager/system-connections/GigE-Camera
            chmod 600 /etc/NetworkManager/system-connections/GigE-Camera
        fi
    fi
done
