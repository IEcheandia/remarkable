#!/bin/sh
rm -rf ${XDG_RUNTIME_DIR}/fifo/${WM_STATION_NAME}/ 2> /dev/null
mkdir -p ${XDG_RUNTIME_DIR}/fifo/${WM_STATION_NAME}/

rm -rf ${XDG_RUNTIME_DIR}/wmpipes/${WM_STATION_NAME}/ 2> /dev/null
mkdir -p ${XDG_RUNTIME_DIR}/wmpipes/${WM_STATION_NAME}/
