#!/bin/sh -e

PHONE_DIR=${PHONE_DIR:-~/Phone}
sms -d "$1" | {
    read -r date time tz
    timestamp="$date $time $tz"
    read -r number
    echo "Timestamp $timestamp Number: $number"
    [ -n "$number" ]
    msg="$(cat -)"
    echo "Saving $timestamp $number $msg"
    if [ -n "$SMS_INDEX" ]; then
        INDEX_DIR="$PHONE_DIR/.index"
        mkdir -p "$INDEX_DIR"
        SMS_INDEX_FILE="$INDEX_DIR/$SMS_INDEX"
        if [ -f "$SMS_INDEX_FILE" ] && [ "$(cat "$SMS_INDEX_FILE")" = "$timestamp" ]; then
            exit 0
        fi
    fi
    SMS_DATE_DIR="$PHONE_DIR/$number/SMS/$(date -I)"
    mkdir -p "$SMS_DATE_DIR"
    printf "%s\t%s\t%s\0\n" "$(date -Iseconds)" "$timestamp" "$msg" >> "$SMS_DATE_DIR/sms.txt"
    if [ -n "$SMS_INDEX" ]; then
        printf "%s" "$timestamp" > "$SMS_INDEX_FILE"
    fi
}
