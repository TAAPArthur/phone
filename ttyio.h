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

void setStatus(int s);
void startReadingSMS();
void receiveSMSNotification(const char*s);
void readSMS(const char*s);

void markSuccess();
void markError();
void clearWaiting();

#define MULTI_LINE_FLAG 1
typedef struct {
    const char* response;
    void(*f)(const char*s);
    const char* cmd;
    int flags;
} Response;

Response responses[] = {
    {"OK", markSuccess},
    {">", clearWaiting},
    {"+CME ERROR: ", markError},
    {"ERROR", markError},
    {"RING"},
    {"NO CARRIER", .cmd = "call -e"},
    {"+CMTI: ", receiveSMSNotification},
    {"+CMGL: ", readSMS, .cmd = "save-sms", .flags = MULTI_LINE_FLAG},
    {"+CMGR: ", readSMS, .cmd = "save-sms", .flags = MULTI_LINE_FLAG},
};


const char* onStartCmds[] = {
    "AT+CLIP=1", // turn on caller id
    "AT+CMGL=4", // dump all stored messages
};
#endif

