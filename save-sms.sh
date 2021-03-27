#!/bin/sh -e

sms -d $1 | {
    read -r date time tz
    timestamp="$date $time $tz"
    read -r number
    echo "Timestamp $timestamp Number: $number"
    [ -n "$number" ]
    SMS_MESSAGE_DIR=${PHONE_DIR:-~/Phone}/$number/SMS
    msg="$(cat -)"
    echo "Saving $timestamp $number $msg"
    SMS_DIR=$SMS_MESSAGE_DIR/$number
    if [ -n "$SMS_INDEX" ]; then
        SMS_INDEX_DIR=$SMS_DIR/.index
        mkdir -p $SMS_INDEX_DIR
        SMS_INDEX_FILE=$SMS_DIR/.index/$SMS_INDEX
        if [ -f $SMS_INDEX_FILE ] && [ "$(cat $SMS_INDEX_FILE)" = "$timestamp" ]; then
            echo "AT+CMGD=$SMS_INDEX" | mqsend phone
            exit 0
        fi
    fi
    SMS_DATE_DIR="$SMS_DIR/$(date -I)"
    mkdir -p $SMS_DATE_DIR
    printf "%s\t%s\t%s\n\0" "$(date -Iseconds)" "$timestamp" "$msg" >> $SMS_DATE_DIR/sms.txt
    if [ -n "$SMS_INDEX" ]; then
        printf "%s" $timestamp > $SMS_INDEX_FILE
        echo "AT+CMGD=$SMS_INDEX" | mqsend phone
    fi
}
