#!/bin/sh

SMS_MESSAGE_DIR=${SMS_MESSAGE_DIR:-~/SMS}
sms -d | {
    read -r _ timestamp
    read -r _ number
    msg="$(cat -)"
    echo "Saving $timestamp $number $msg"
    SMS_DIR=$SMS_MESSAGE_DIR/$number/
    if [ -n "$SMS_INDEX" ]; then
        SMS_INDEX_DIR=$SMS_DIR/.index/
        mkdir -p $SMS_INDEX_DIR
        SMS_INDEX_FILE=$SMS_DIR/.index/$SMS_INDEX
        if [ -f $SMS_INDEX_FILE ] && [ "$(cat $SMS_INDEX_FILE)" = "$timestamp" ]; then
            exit 0
        fi
    fi
    SMS_DATE_DIR="$SMS_DIR/$(date -I)"
    mkdir -p $SMS_DATE_DIR
    printf "%s\t%s\t%s\0" "$(date -Iseconds)" "$timestamp" "$msg" >> $SMS_DATE_DIR/sms.txt
    [ -z "$SMS_INDEX" ] || printf "%s" $timestamp > $SMS_INDEX_FILE
}
