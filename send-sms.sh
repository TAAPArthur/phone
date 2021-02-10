#!/bin/sh -e
CONTACTS_FILE=${CONTACTS_FILE:-$HOME/contacts.json}
name=$1
NUMBER=$(jq .$name.number $CONTACTS_FILE | sed -E "s/[^0-9]+//g")
sms $NUMBER | mbus phone
