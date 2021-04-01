#!/bin/sh
CONTACTS_FILE=${CONTACTS_FILE:-$HOME/contacts.json}
PHONE_DIR=${PHONE_DIR:-~/Phone}

getName() {
    number="$1"
    name=$(jq -er "to_entries[] | .key +\"\t\"+ .value.number" "$CONTACTS_FILE" | sed -E "s/[-\+()]*//g; s/([0-9]) ([0-9])/\1\2/g" | grep "$number"'$' | head -n1 | awk '{print $1}')
    if [ -z "$name" ]; then
        name=$(jq -er jq -er "to_entries[] | select (.value.numbers != null) | .key +\"\t\"+ (.value.numbers | .[])" "$CONTACTS_FILE" | sed -E "s/[-\+()]*//g; s/([0-9]) ([0-9])/\1\2/g"| head -n1 | awk '{print $1}')
    fi
    [ -n "$name" ] && echo "$name"

}
case "$1" in
    list-all)
        jq -r "keys[]" "$CONTACTS_FILE"
        ;;
    list)
        jq -r "to_entries[]|select (.value.group | index(\"FriendLike\")) | .key" "$CONTACTS_FILE"
        ;;
    list-sorted)
        ;;
    list-recents)
        #ls -pt $PHONE_DIR/**/.lastaccess | head | xargs -I{} $0 get-name {}
        ;;
    get-name)
        number=$(echo "$2"| sed -E "s/[^0-9]+//g")
        name=$(getName "$number" || getName "${number#1}")
        [ -z "$name" ] && echo "$number" || echo "$name"
        ;;
    get-number)
        (jq -er ".\"$2\".number" "$CONTACTS_FILE" || echo "$2") | tail -n 1| sed -E "s/[^0-9]+//g"
        ;;
esac
