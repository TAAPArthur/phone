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

const char* SMS_READ_CMD = "/bin/save-sms";

void setStatus(int s);
void startReadingSMS();
void receiveSMSNotification(const char*s);
void readSMS(const char*s);

static inline void markSuccess(){setStatus(SUCCESS);}
static inline void markError(){setStatus(ERROR);}
static inline void clearWaiting(){setStatus(IN_PROGRESS);}

typedef struct {
    const char* response;
    void(*f)(const char*s);
} Response;

Response responses[] = {
    {"OK", markSuccess},
    {">", clearWaiting},
    {"+CME ERROR: ", markError},
    {"ERROR", markError},
    {"RING"},
    {"+CMGL: ", startReadingSMS},
    {"+CMTI: ", receiveSMSNotification},
    {"+CMGR: ", readSMS}
};

const char* onStartCmds[] = {
    MSG_ENDING_STR, // end any existing message prompt
    "AT+CMGL=4", // dump all stored messages
};
#endif

