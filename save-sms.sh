#!/bin/sh -e

PHONE_DIR=${PHONE_DIR:-/var/phone}
[ -e "$PHONE_DIR" ] || mkdir -p "$PHONE_DIR"
sms -d "$1" | {
    read -r date time tz
    timestamp="$date $time $tz"
    read -r number
    [ -n "$number" ]
    [ "$number" = "1${number#1}" ] && number="${number#1}"

    if [ -n "$SMS_INDEX" ]; then
        INDEX_DIR="$PHONE_DIR/.index"
        mkdir -p "$INDEX_DIR"
        SMS_INDEX_FILE="$INDEX_DIR/$SMS_INDEX"
        if [ -f "$SMS_INDEX_FILE" ] && [ "$(cat "$SMS_INDEX_FILE")" = "$timestamp" ]; then
            exit 0
        fi
    fi
    read -r header
    headerString=$header
    if  [ "$header" -eq 0 ] || [ "$header" -eq 8 ]; then
        read -r id seqNum numParts
        headerString="$header $id $seqNum $numParts"
    fi
    read -r encoding

    msg="$(cat -)"
    echo "Saving $timestamp $number $headerString $msg"
    SMS_DATE_DIR="$PHONE_DIR/ByNumber/$number/"
    mkdir -p "$SMS_DATE_DIR"
    printf "%s\t%s\t%s\t%s\t%s\0\n" "$(date -Iseconds)" "$timestamp" "$headerString" "$encoding" "$msg" >> "$SMS_DATE_DIR/sms.txt"
    if [ -n "$SMS_INDEX" ]; then
        printf "%s" "$timestamp" > "$SMS_INDEX_FILE"
    fi
    echo "$number $msg" | tr "\r" "\n" | smsd -s
}
