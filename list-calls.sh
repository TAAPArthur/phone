#!/bin/sh -e

BASE_FILTER=" [0-9]*,[0-9]*,"
case "$1" in
    -a) # active
        filter=${BASE_FILTER}0
        ;;
    -h) # held
        filter=${BASE_FILTER}1
        ;;
    -d) # dialing
        filter=${BASE_FILTER}2
        ;;
    -l) # alerting
        filter=${BASE_FILTER}3
        ;;
    -r|-i) # ringing/incoming
        filter=${BASE_FILTER}4
        ;;
    -w) # waiting
        filter=${BASE_FILTER}5
        ;;
    "")
        filter=""
        ;;
esac

# Fields id dir stat mode mpty number type alpha
wait-for-phone-response $$ | grep -E "$filter" &
pid=$!
echo "#LABEL=$$ AT+CLCC" | mqsend phone
wait $pid
