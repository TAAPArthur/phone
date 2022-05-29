#!/bin/sh

if [ "$1" = "-s" ]; then
    echo "$2" | mqbus-send ringd
    exit
fi

mqbus-receive ringd |  {

    RINGD_DIR=${XDG_CONFIG_DIR:-$HOME/.config}/ringd
    mkdir -p "$RINGD_DIR"
    export NUMBER TYPE SUBADDR SATYPE ALPHA VALIDITY
    while IFS="," read -r NUMBER TYPE SUBADDR SATYPE ALPHA VALIDITY; do
        [ -n "$NUMBER" ] || continue
        (
            for file in "$RINGD_DIR"/*.sh; do
                [ -r "$file" ] && . "$file"
            done
        )
    done
}
