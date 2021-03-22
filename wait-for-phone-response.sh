#!/bin/sh -e
mqbus-receive phone-response | grep -q "^$1 " | {
    while read -r _ status data; do
        case $status in
            TERM)
                exit $data;;
            MSG)
                echo $data;;
        esac
    done
}

