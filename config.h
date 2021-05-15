#ifndef CONFIG_TTYIO_H
#define CONFIG_TTYIO_H

#include "ttyio.h"

const char LN_ENDING[] ="\r\n";
const char * device = "/dev/ttyUSB2";

Response responses[] = {
    {"OK", markSuccess},
    {">", clearWaiting, .cmd = "save-sms %s", .flags = MULTI_LINE_FLAG | ADD_RESPONSE_FLAG },
    {"+CME ERROR: ", markError},
    {"ERROR", markError},
    {"RING", .cmd = "call --ring"},
    {"+CLIP:", .cmd = "ringd -s %s", .flags = STRIP_LABEL | ADD_RESPONSE_FLAG},
    {"NO CARRIER", markError, .flags = ONLY_IF_WAITING },
    {"NO CARRIER", .cmd = "call -e"},
    {"+CMTI: ", receiveSMSNotification},
    {"+CMGL: ", readSMS, .cmd = "save-sms %s", .successFunction = deleteSMS, .flags = MULTI_LINE_FLAG | ADD_RESPONSE_FLAG },
    {"+CMGR: ", readSMS, .cmd = "save-sms %s", .successFunction = deleteSMS, .flags = MULTI_LINE_FLAG | ADD_RESPONSE_FLAG },
};

const char* onStartDeviceCmds[] = {
    "stty -F %s -icanon -echo"
};

const char* onStartCmds[] = {
    "AT+CLIP=1", // turn on caller id
    "AT+CMGL=4", // dump all stored messages
    "ATE1", // echo input back
};
#endif
