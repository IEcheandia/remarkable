#!/bin/bash

grep '^interface-name=$' /etc/NetworkManager/system-connections/Weldmaster > /dev/null
WeldmasterFound=$?

grep '^interface-name=$' /etc/NetworkManager/system-connections/Peripheral > /dev/null
PeripheralFound=$?

for ifName in `ls /sys/class/net/`; do
    if [ "${ifName}" = "lo" ]; then
        continue
    fi

    vendor=`cat /sys/class/net/${ifName}/device/vendor`
    device=`cat /sys/class/net/${ifName}/device/device`
    macAddress=`cat /sys/class/net/${ifName}/address`
    if [ "${vendor}" = "0x8086" ]; then
        # Intel
        if [ "${device}" = "0x15b7" ]; then
            # Intel Corporation Ethernet Connection (2) I219-LM
            if [[ $WeldmasterFound == 0 ]]; then
                sed -i "s/interface-name=/interface-name=${ifName}/" /etc/NetworkManager/system-connections/Weldmaster
                sed -i "s/mac-address=/mac-address=${macAddress}/" /etc/NetworkManager/system-connections/Weldmaster
                WeldmasterFound=1
            fi
        fi
        if [ "${device}" = "0x1533" ]; then
            # Intel Corporation I210 Gigabit Network Connection
            if [[ $PeripheralFound == 0 ]]; then
                sed -i "s/interface-name=/interface-name=${ifName}/" /etc/NetworkManager/system-connections/Peripheral
                sed -i "s/mac-address=/mac-address=${macAddress}/" /etc/NetworkManager/system-connections/Peripheral
                PeripheralFound=1
            fi
        fi
        
        
        if [ "${device}" = "0x15b8" ]; then
            # Intel Corporation Ethernet Connection (2) I219-V
            if [[ $WeldmasterFound == 0 ]]; then
                sed -i "s/interface-name=/interface-name=${ifName}/" /etc/NetworkManager/system-connections/Weldmaster
                sed -i "s/mac-address=/mac-address=${macAddress}/" /etc/NetworkManager/system-connections/Weldmaster
                WeldmasterFound=1
            fi
        fi
        if [ "${device}" = "0x1539" ]; then
            # Intel Corporation I211 Gigabit Network Connection
            if [[ $PeripheralFound == 0 ]]; then
                sed -i "s/interface-name=/interface-name=${ifName}/" /etc/NetworkManager/system-connections/Peripheral
                sed -i "s/mac-address=/mac-address=${macAddress}/" /etc/NetworkManager/system-connections/Peripheral
                PeripheralFound=1
            fi
        fi

        # Intel i9-10900 system
        if [ "${device}" = "0x0d4c" ]; then
            # Intel Corporation Ethernet Connection (11) I219-LM
            if [[ $WeldmasterFound == 0 ]]; then
                sed -i "s/interface-name=/interface-name=${ifName}/" /etc/NetworkManager/system-connections/Weldmaster
                sed -i "s/mac-address=/mac-address=${macAddress}/" /etc/NetworkManager/system-connections/Weldmaster
                WeldmasterFound=1
            fi
        fi
        if [ "${device}" = "0x15fc" ]; then
            # Intel Corporation Ethernet Connection I225-LM
            if [[ $PeripheralFound == 0 ]]; then
                sed -i "s/interface-name=/interface-name=${ifName}/" /etc/NetworkManager/system-connections/Peripheral
                sed -i "s/mac-address=/mac-address=${macAddress}/" /etc/NetworkManager/system-connections/Peripheral
                PeripheralFound=1
            fi
        fi
    fi
done

if [[ $WeldmasterFound == 0 ]]; then
    sed -i "/interface-name=/d" /etc/NetworkManager/system-connections/Weldmaster
    sed -i "/mac-address=/d" /etc/NetworkManager/system-connections/Weldmaster
fi

if [[ $PeripheralFound == 0 ]]; then
    sed -i "/interface-name=/d" /etc/NetworkManager/system-connections/Peripheral
    sed -i "/mac-address=/d" /etc/NetworkManager/system-connections/Peripheral
fi
