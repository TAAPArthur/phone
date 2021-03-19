#!/bin/sh -e
number=$(contacts get-number $1)
echo "Sending message to $number"
sms $number $@ | mqsend phone
