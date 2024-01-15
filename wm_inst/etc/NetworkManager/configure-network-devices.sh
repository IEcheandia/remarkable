#!/bin/bash

/opt/configure-devices.sh
chmod 600 /etc/NetworkManager/system-connections/Peripheral
chmod 600 /etc/NetworkManager/system-connections/Weldmaster

if [ $(stat -c "%a" /etc/NetworkManager/system-connections/GigE-Camera) != "600" ]
then
    /opt/configure-gige.sh
fi
