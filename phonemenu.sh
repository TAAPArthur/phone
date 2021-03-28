#!/bin/sh -e

quick_text() {
    action=$(dmenu) <<EOF
Test
Driving
Here
Will call you back later
Help requested
nm
EOF
    send-sms "$1" "$action"
}
contacts_menu() {
name=$(contacts list-recent | dmenu)
action=$( dmenu) <<EOF
call
text
cancel
hangup
answer
EOF
case "$action" in
    call)
        call "$name"
        ;;
    answer)
        call -a
        ;;
    hangup)
        call -h
        ;;
    text)
        quick_text "$name"
        ;;
    *)
        exit 0
        ;;
esac
}
cmd=$(dmenu) <<EOF
suspend
contact
camera
lock
cancel
EOF
case "$cmd" in
    suspend)
        /bin/suspend
        ;;
    contact)
        contacts_menu
        ;;
    cancel)
        exit 0
        ;;
    *)
        exit 1
esac
