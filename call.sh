#!/bin/sh -e

CALL_DIR=${PHONE_DIR:-~/Phone}
CALL_LOG=$CALL_DIR/call.log
cmd=
log() {
    printf "%s\t%s\t%s\n" "$(date -Iseconds)" "$1" "$2" >> "$CALL_LOG"
}
ring() {
    read -r number _ _ _ alpha valid
    log RING "$number"
    [ "$valid" = "0" ]
    if notify-send -r "phone-$number" "Call from $(contacts get-name "$number" || echo "$number") $alpha"; then
        $0 -a
    else
        $0 -h
    fi
}

action=$1
shift
case $action in
    end)
        log END
        rm /tmp/.ringing
        exit
    ;;
    --is-ringing)
        [ -e /tmp/.ringing ]
        exit
        ;;
    ring)
        touch /tmp/.ringing
        ring
        exit
    ;;
    -a)
        cmd="ATA" # answer call
        LOG_ACTION=ANSWER
        ;;
    -h)
        if [ "$1" = "--all" ]; then
            cmd="AT+CHUP" # hang up all calls
        else
            cmd="HUP" # hang up call
        fi
        LOG_ACTION=HANGUP
        ;;
    -t)
        cmd="AT+VTS=$1" # dial tones
        LOG_ACTION=DIALTONE
        LOG_PARAM=$1
        ;;
    -d|*)
        name=$1
        shift
        number=$(contacts get-number "$name")
        cmd="ATD$number$*;"
        LOG_ACTION=DIAL
        LOG_PARAM=$number
    ;;
esac


wait-for-phone-response $$ &
pid=$!
echo "#LABEL=$$ $cmd" | mqsend phone
wait $pid
if [ -n "$LOG_ACTION" ]; then
    log $LOG_ACTION "$LOG_PARAM"
fi
