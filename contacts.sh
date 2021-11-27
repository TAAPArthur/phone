#!/bin/sh -e
CONTACTS_DIR=${CONTACTS_DIR:-$HOME/Contacts}
PHONE_DIR=${PHONE_DIR:-/var/phone}
LOCAL_PHONE_DIR=${LOCAL_PHONE_DIR:-~/Phone}

getNameFromNumber() {
    grep -h -v "^#" "$CONTACTS_DIR"/contacts* | sed -E "s/\+?\s*\(?\s*([0-9]+)\s*-?\)?/\1/g" | grep -E -e "\|$1(\||\$)" -e "\|${1#1}(\||\$)" -e "\|1$1(\||\$)" | head -n1 | cut -d"|" -f 1 | grep .
}

getNumberFromName() {
    grep "^$1|" "$CONTACTS_DIR"/contacts* | cut -d"|" -f2 | sed -E "s/[^0-9]+//g" | head -n1 | grep .
}

getEmailFromName() {
    grep "^$1|" "$CONTACTS_DIR"/contacts* |  head -n1 | grep -Po "[^|@]+@[^|@]+"
}

case "$1" in
    list)
        cat "$CONTACTS_DIR"/contacts* | cut -d"|" -f 1
        ;;
    list-recents)
        cd "$PHONE_DIR" || exit
        find ByNumber/ -type f -exec stat -c "%Y %n" {} \; | sort -r -n | head -n10 | cut -d"/" -f2 | {
            while read -r number; do
                getNameFromNumber "$number" || echo "$number"
            done
        }
        ;;
    recent-sms)
        grep -R "" "$PHONE_DIR/ByNumber" | sort -bdr -k2,3 | head -n"${1:-10}" | {
            while IFS=: read -r file data; do
                number="$(basename "$(dirname "$file")")"
                printf "%s %s\n" "$(getNumberFromName "$number")" "$data"
            done
        }
        ;;
    last)
        cd "$PHONE_DIR/ByNumber"
        {
        for dir in *; do
            [ -d "$dir" ]
            line=$(tail -n1 "$dir/sms.txt")
            printf "%s %s\n" "$(getNameFromNumber "$dir" || echo "$dir")" "$line"
        done
        } | sort -bdr -k2,3
        ;;
    link)
        mkdir -p "$LOCAL_PHONE_DIR/ByName"
        [ -d "$LOCAL_PHONE_DIR/ByNumber" ] || ln -sf "$PHONE_DIR/ByNumber" "$LOCAL_PHONE_DIR/ByNumber"
        [ -f "$LOCAL_PHONE_DIR/call.log" ] || ln -sf "$PHONE_DIR/call.log" "$LOCAL_PHONE_DIR/call.log"
        find "$LOCAL_PHONE_DIR/ByNumber/" -type d | {
            while read -r dir; do
                number=$(basename "$dir" | sed -E "s/[^0-9]+//g")
                [ -z "$number" ] && continue
                for name in $(getNameFromNumber "$number" || true); do
                    if [ -n "$name" ] && ! [ "$LOCAL_PHONE_DIR/ByNumber/$number" -ef "$LOCAL_PHONE_DIR/ByName/$name" ]; then
                        ln -sf "../ByNumber/$number" "$LOCAL_PHONE_DIR/ByName/$name"
                    fi
                done
            done
        }

        ;;
    get-name)
        number=$(echo "$2" | sed -E "s/[^0-9]+//g")
        getNameFromNumber "$number"
        ;;
    get-number)
        getNumberFromName "$2"
        ;;
    get-email)
        getEmailFromName "$2"
        ;;
    get-sms-file)
        echo "$LOCAL_PHONE_DIR/ByName/$2/sms.txt"
        ;;
esac
