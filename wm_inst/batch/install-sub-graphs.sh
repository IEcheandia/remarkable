#!/bin/bash

PRODUCT=$1

RELATIVE_PATH="`dirname ${0}`"
WM_BASE_DIR="`( cd \"$RELATIVE_PATH\" && cd .. && pwd )`"

GRAPH_DIR="${WM_BASE_DIR}/config_${PRODUCT}/sub_graphs/"

for D in ${GRAPH_DIR}*; do
    if [ -d "${D}" ]; then
        SUBDIR=`realpath --relative-to=${GRAPH_DIR} "${D}"`
        TARGETDIR="${WM_BASE_DIR}/system_graphs/sub_graphs/${SUBDIR}"
        mkdir -p "${TARGETDIR}"

        for F in "${D}/"*; do
            if [ -e "${F}" ]; then
                RELPATH=`realpath --relative-to=${GRAPH_DIR} "${F}"`
                echo "${RELPATH}"
                ln -s -f "${GRAPH_DIR}${RELPATH}" "${WM_BASE_DIR}/system_graphs/sub_graphs/${RELPATH}"
            fi
         done
    fi
done
