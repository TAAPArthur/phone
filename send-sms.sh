#!/bin/sh -e
CONTACTS_FILE=${CONTACTS_FILE:-$HOME/contacts.json}
name=$1
shift
number=$(jq .$name.number $CONTACTS_FILE | sed -E "s/[^0-9]+//g")
[ -z "$number" ]  && number=$name
echo "Sending message to $number"
sms $number $@ | mqsend phone
