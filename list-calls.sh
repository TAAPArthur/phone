#!/bin/sh -e

filter="^[0-9]*,"
case "$1" in
    -a) # active
        filter=0
        ;;
    -h) # held
        filter=1
        ;;
    -d) # dialing
        filter=2
        ;;
    -l) # alerting
        filter=3
        ;;
    -r|-i) # ringing/incoming
        filter=4
        ;;
    -w) # waiting
        filter=5
        ;;
    "")
        filter=""
        ;;
esac

# Fields id dir stat mode mpty number type alpha
wait-for-phone-response $$ | grep "$filter" &
pid=$!
echo "#LABEL=$$ AT+CLCC" | mqsend phone
wait $pid
