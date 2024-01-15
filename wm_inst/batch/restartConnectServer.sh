#!/bin/sh

export DISPLAY=:0
export XDG_RUNTIME_DIR=/run/user/`id -u`/
unset DBUS_SESSION_BUS_ADDRESS
dbus-send --print-reply --session --dest=org.kde.klauncher5 /KLauncher org.kde.KLauncher.start_service_by_desktop_path string:"/etc/xdg/autostart/weldmaster.desktop" array:string: array:string: string:"" boolean:false
