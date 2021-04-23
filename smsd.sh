#!/bin/sh -e

if [ "$1" = "-s" ]; then
    mqbus-send smsd
    exit
fi

mqbus-receive smsd |  {

    SMSD_DIR=${XDG_CONFIG_DIR:-$HOME/.config}/smsd
    mkdir -p "$SMSD_DIR"
    export NUMBER MSG
    while read -r NUMBER MSG; do
        (
            for file in "$SMSD_DIR"/*.sh; do
                [ -r "$file" ] && . "$file"
            done
        )
    done
}
