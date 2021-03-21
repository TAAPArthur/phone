#!/bin/sh -e
CONTACTS_FILE=${CONTACTS_FILE:-$HOME/contacts.json}

case "$1" in
    list)
        jq -r "keys[]" $CONTACTS_FILE
        ;;
    get-name)
        jq -r "to_entries[]|select (.value.number | contains(\"$2\")) | .key" $CONTACTS_FILE
        ;;
    get-number)
        number=$( (jq -er ".$2.number" $CONTACTS_FILE || echo $2) | sed -E "s/[^0-9]+//g")
        [ -n "$number" ]
        echo $number
esac
