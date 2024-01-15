#! /bin/bash

if [ "$1" = "START" ]; then
    if [ -b "/dev/sdb1" ]; then
        systemctl start mnt-backup.mount
        chmod 0777 /mnt/backup
    fi
elif [ "$1" = "STOP" ]; then
    if [ -b "/dev/sdb1" ]; then
        systemctl stop mnt-backup.mount
    fi
fi

