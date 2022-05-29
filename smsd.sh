#!/bin/sh

if [ "$1" = "-s" ]; then
    mqbus-send smsd
    exit
fi

mqbus-receive smsd |  {

    SMSD_DIR=${XDG_CONFIG_DIR:-$HOME/.config}/smsd
    mkdir -p "$SMSD_DIR"
    export NUMBER SENT MSG
    while read -r NUMBER SENT RAW_MSG; do
        (
            MSG=$(echo "$RAW_MSG" | tr "\v" "\n")
            for file in "$SMSD_DIR"/*.sh; do
                [ -r "$file" ] && . "$file"
            done
        )
    done
}
