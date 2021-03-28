#!/bin/sh -e
changePower(){
    printf "%s" $1 > /sys/class/modem-power/modem-power/device/powered
    while [ "$1" -ne "$(cat /sys/class/modem-power/modem-power/device/powered)" ]; do
        sleep 1
        printf .
    done
    echo "done"
}
case $1 in
    on)
        changePower 1
    ;;
    off)
        changePower 0
    ;;
    "")
        cat /sys/class/modem-power/modem-power/device/powered
    ;;
esac
