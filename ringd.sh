#!/bin/sh -e

if [ "$1" = "-s" ]; then
    echo "$2" | mqsend phone-ring
    exit
fi

mqreceive phone-ring |  {

    RINGD_DIR=${XDG_CONFIG_DIR:-$HOME/.config}/ringd
    mkdir -p "$RINGD_DIR"
    export NUMBER TYPE SUBADDR SATYPE ALPHA VALIDITY
    while IFS="," read -r NUMBER TYPE SUBADDR SATYPE ALPHA VALIDITY; do
        (
            call --ring "$NUMBER"
            for file in "$RINGD_DIR"/*.sh; do
                [ -r "$file" ] && . "$file"
            done
        )
    done
}

