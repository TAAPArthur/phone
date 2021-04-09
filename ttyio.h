#ifndef PHONE_TTYIO_H
#define PHONE_TTYIO_H

#define SUCCESS 0
#define ERROR 2
#define IN_PROGRESS 4

#define SMS_INDEX_ENV_NAME "SMS_INDEX"

#define MSG_ENDING 0x1A
const char MSG_ENDING_STR[] = {MSG_ENDING};
const char LN_ENDING[] ="\r\n";
const char * device = "/dev/ttyUSB2";

#define SHELL "/bin/sh"

void setStatus(int s);
void startReadingSMS();
void receiveSMSNotification(const char*s);
void readSMS(const char*s);
void deleteSMS();

void markSuccess();
void markError();
void clearWaiting();
#define CMD_BUFFER 255

#define MULTI_LINE_FLAG 1
#define STRIP_LABEL 2
#define ADD_RESPONSE_FLAG 4
typedef struct {
    const char* response;
    void(*f)(const char*s);
    const char* cmd;
    void(*successFunction)(const char*s);
    int flags;
} Response;

Response responses[] = {
    {"OK", markSuccess},
    {">", clearWaiting, .cmd = "save-sms %s", .flags = MULTI_LINE_FLAG | ADD_RESPONSE_FLAG },
    {"+CME ERROR: ", markError},
    {"ERROR", markError},
    {"RING"},
    {"+CLIP:", .cmd = "ringd -s %s", .flags = STRIP_LABEL | ADD_RESPONSE_FLAG},
    {"NO CARRIER", .cmd = "call -e %s"},
    {"+CMTI: ", receiveSMSNotification},
    {"+CMGL: ", readSMS, .cmd = "save-sms %s", .successFunction = deleteSMS, .flags = MULTI_LINE_FLAG | ADD_RESPONSE_FLAG },
    {"+CMGR: ", readSMS, .cmd = "save-sms %s", .successFunction = deleteSMS, .flags = MULTI_LINE_FLAG | ADD_RESPONSE_FLAG },
};


const char* onStartCmds[] = {
    "AT+CLIP=1", // turn on caller id
    "AT+CMGL=4", // dump all stored messages
    "ATE1", // echo input back
};
#endif

