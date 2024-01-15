#!/bin/bash

SYSTEMCONFIG=${WM_BASE_DIR}/config/SystemConfig.xml

# migrate FramegrabberEnabled
if [[ -f "${SYSTEMCONFIG}" ]]; then
    grep FramegrabberEnabled ${SYSTEMCONFIG}
    FOUND=$?
    if [[ $FOUND == 0 ]]; then
        sed -i "s/FramegrabberEnabled/HardwareCameraEnabled/g" ${SYSTEMCONFIG}
    fi
fi

# migrate SCANMASTER_PhotonAutomation
if [[ -f "${SYSTEMCONFIG}" ]]; then
    grep SCANMASTER_PhotonAutomation ${SYSTEMCONFIG}
    FOUND=$?
    if [[ $FOUND == 0 ]]; then
        sed -i "s/SCANMASTER_PhotonAutomation/SCANMASTER_ThreeStepInterface/g" ${SYSTEMCONFIG}
    fi
fi

# migrate ScanlabScannerEnable
if [[ -f "${SYSTEMCONFIG}" ]]; then
    grep ScanlabScannerEnable ${SYSTEMCONFIG}
    FOUND=$?
    if [[ $FOUND == 0 ]]; then
        sed -i "s/ScanlabScannerEnable/Scanner2DEnable/g" ${SYSTEMCONFIG}
    fi
fi

# migrate ScanlabScanner_IP_Address
if [[ -f "${SYSTEMCONFIG}" ]]; then
    grep ScanlabScanner_IP_Address ${SYSTEMCONFIG}
    FOUND=$?
    if [[ $FOUND == 0 ]]; then
        sed -i "s/ScanlabScanner_IP_Address/Scanner2DController_IP_Address/g" ${SYSTEMCONFIG}
    fi
fi
