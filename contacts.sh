#!/bin/sh -e
CONTACTS_FILE=${CONTACTS_FILE:-$HOME/contacts.json}

case "$1" in
    list)
        jq -r "keys[]" $CONTACTS_FILE
        ;;
    get-name)
        number=$(echo $2| sed -E "s/[^0-9]+//g")
        jq -er "to_entries[]|select (.value.number | gsub(\"[^0-9]\";\"\") | (select (. == \"$number\" or . == \"${number#1}\"))) | .key" $CONTACTS_FILE || echo $number
        ;;
    get-number)
        number=$( (jq -er ".\"$2\".number" $CONTACTS_FILE || echo $2) | sed -E "s/[^0-9]+//g")
        [ -n "$number" ]
        echo $number
esac
