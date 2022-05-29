#!/bin/sh -e

PHONE_DIR=${PHONE_DIR:-/var/phone}
[ -e "$PHONE_DIR" ] || mkdir -p "$PHONE_DIR"
sms -d "$1" | {
    read -r date time tz
    timestamp="$date $time $tz"
    read -r number
    [ -n "$number" ]
    [ "$number" = "1${number#1}" ] && number="${number#1}"

    read -r header
    headerString=$header
    if  [ "$header" -eq 0 ] || [ "$header" -eq 8 ]; then
        read -r id seqNum numParts
        headerString="$header $id $seqNum $numParts"
    fi
    read -r encoding

    msg="$(tr "\n" "\v")"
    echo "Saving $timestamp $number $headerString $msg"
    SMS_DIR="$PHONE_DIR/ByNumber/$number/"
    mkdir -p "$SMS_DIR"
    printf "%s\t%s\t%s\t%s\t%s\0\n" "$(date +%FT%H:%M:%S%z)" "$timestamp" "$headerString" "$encoding" "$msg" >> "$SMS_DIR/sms.txt"
    SENT=0
    [ "$date" = 0 ] && SENT=1
    echo "$number $SENT $msg" | smsd -s
} || {
    echo "Encounter error; saving encoded message"
    echo "$1" >> "$PHONE_DIR/error.log"
}
