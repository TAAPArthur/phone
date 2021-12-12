#!/bin/sh -e

CALL_DIR=${PHONE_DIR:-/var/phone}
CALL_LOG=$CALL_DIR/call.log
cmd=
log() {
    printf "%s\t%s\t%s\n" "$(date +%FT%H:%M:%S%z)" "$1" "$2" >> "$CALL_LOG"
}

action=$1
shift
case $action in
    --ring)
        log RING "$1"
        ringd -s "$1"
        exit
    ;;
    --log-dial)
        log DIAL "$1"
        exit
        ;;
    -e)
        log END
        exit
    ;;
    -a)
        cmd="ATA" # answer call
        LOG_ACTION=ANSWER
    ;;
    -b|--drop-ball)
        cmd="AT+CHLD=0" # terminate held calls
        LOG_ACTION=CLEAN_HELD
    ;;
    -c|--connect)
        cmd="AT+CHLD=4" # Connects active calls and drops off
        LOG_ACTION=CONNECT
    ;;
    -f|--finish)
        cmd="AT+CHLD=1$1" # terminate active call (all if not specified) and accept other call
        LOG_ACTION=TERMINATE$1
    ;;
    -h)
        if [ "$1" = "--all" ]; then
            cmd="AT+CHUP" # hang up all calls
        else
            cmd="ATH" # hang up call
        fi
        LOG_ACTION=HANGUP
    ;;
    -j|--join)
        cmd="AT+CHLD=3" # Add held call to active call
        LOG_ACTION=JOIN
    ;;
    -o|--hold)
        cmd="AT+CHLD=2$1" # place active call (all but $1 if specified) on hold
        LOG_ACTION=HOLD$1
    ;;
    -t|--dialtone)
        cmd="AT+VTS=$1" # dial tones
        LOG_ACTION=DIALTONE
        LOG_PARAM=$1
    ;;
    -d|*)
        name=$1
        shift
        number=$(contacts get-number "$name" || echo "$name")
        echo "$number" | grep -q '^[0-9]+$'
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
