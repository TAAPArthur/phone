#!/bin/sh
CONTACTS_FILE=${CONTACTS_FILE:-$HOME/contacts.json}
PHONE_DIR=${PHONE_DIR:-~/Phone}
SED_PARSER="s/[-\+()]*//g; s/([0-9]) ([0-9])/\1\2/g"


getAltName() {
    name="$(jq -er "to_entries[] | select (.value.numbers != null) | .key + \".\" + (.value.numbers | to_entries[] |
        .key + \"\t\" + .value )" "$CONTACTS_FILE" | sed -E "$SED_PARSER" | grep "$1" | head -n1 | cut -f1 )"
    [ -n "$name" ] && echo "$name"
}
getDefaultName() {
    name="$(jq -er "to_entries[] | .key +\"\t\"+ .value.number" "$CONTACTS_FILE" | sed -E "$SED_PARSER" | grep "$1"'$' | head -n1 | cut -f1)"
    [ -n "$name" ] && echo "$name"
}
getNameOrAltName() {
    name="$(getDefaultName "$1")"
    if [ -z "$name" ]; then
        name="$(getAltName "$1")"
    fi
    [ -n "$name" ] && echo "$name"

}

getName() {
    number="$1"
    name="$(getNameOrAltName "$number" || getNameOrAltName "${number#1}")"
    [ -z "$name" ] && echo "$number" || echo "$name"
}
case "$1" in
    list-all)
        jq -r "keys[]" "$CONTACTS_FILE"
        ;;
    list)
        jq -r "to_entries[]|select (.value.group | index(\"FriendLike\")) | .key" "$CONTACTS_FILE"
        ;;
    list-recents)
        cd "$PHONE_DIR" || exit
        find ByNumber/ -type f -exec stat -c "%Y %n" {} \; | sort -r | head -n10 | cut -d"/" -f2 | {
            while read -r number; do
                getName "$number"
            done
        }
        ;;
    link)
        mkdir -p "$PHONE_DIR/ByName"
        cd "$PHONE_DIR/ByNumber" || exit
        find . -type d | {
            while read -r number; do
                [ "$number" = "." ] && continue
                number=$(echo "$number"| sed -E "s/[^0-9]+//g")
                for name in $(getDefaultName "$number") $(getAltName "$number"); do
                    [ -n "$name" ] && ln -sf "../ByNumber/$number" "../ByName/$name"
                done
            done
        }

        ;;
    get-name)
        number=$(echo "$2" | sed -E "s/[^0-9]+//g")
        getName "$number"
        ;;
    get-number)
        (jq -er ".\"$2\".number" "$CONTACTS_FILE" || echo "$2") | tail -n 1| sed -E "s/[^0-9]+//g"
        ;;
esac
