#!/bin/sh
[ -n "$1" ]
mqbus-receive phone-response | {
    while read -r label status data; do
        [ "$label" = "$1" ] || continue
        case $status in
            TERM)
                exec 0<&-
                exit $data;;
            MSG)
                echo $data;;
        esac
    done
}

