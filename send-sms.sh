#!/bin/sh -e

number=$(contacts get-number "$1")
echo "Sending message to $number"
shift

wait-for-phone-response $$ &
pid=$!
{
    printf "%s" "#LABEL=$$ "
    sms "$number" "$@"
} | mqsend phone
wait $pid
