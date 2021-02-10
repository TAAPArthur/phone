#!/bin/sh
SMS_MESSAGE_DIR=${SMS_MESSAGE_DIR:-~/SMS}
while IFS=$'\t' read -r -d $'\0' timestamp number msg; do
    SMS_DIR=$SMS_MESSAGE_DIR/$number/
    if [ ! -z "$SMS_INDEX" ]; then
        SMS_INDEX_FILE=$SMS_DIR/.index/$SMS_INDEX
        mkdir -p $SMS_INDEX_FILE
        if [ $(cat $SMS_INDEX_FILE) = "$timestamp" ]; then
            continue
        fi
    fi
    SMS_DATE_DIR="$SMS_DIR/$(date -I)"
    mkdir -p $SMS_DATE_DIR
    echo -en "$(date -Iseconds)\t$timestamp\t$msg\0" >> $SMS_DATE_DIR/sms.txt
    [ -z "$SMS_INDEX" ] || echo -n $timestamp > $SMS_INDEX_FILE
done
