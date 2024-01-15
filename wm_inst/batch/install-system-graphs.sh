#!/bin/bash

PRODUCT=$1

RELATIVE_PATH="`dirname ${0}`"
WM_BASE_DIR="`( cd \"$RELATIVE_PATH\" && cd .. && pwd )`"

GRAPH_DIR="${WM_BASE_DIR}/system_graphs/graphs_${PRODUCT}/"

TARGETDIR="${WM_BASE_DIR}/system_graphs/"
mkdir -p "${TARGETDIR}"

for F in ${GRAPH_DIR}*; do
    if [ -e "${F}" ]; then
        LINK=$(readlink -f "${F}")
        NAME=$(basename "${F}")
        ln -s -f "${LINK}" "${TARGETDIR}/${NAME}"
    fi
done
