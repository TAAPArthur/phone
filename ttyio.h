#ifndef PHONE_TTYIO_H
#define PHONE_TTYIO_H

#define SUCCESS 0
#define ERROR 2
#define IN_PROGRESS 4

#define SMS_INDEX_ENV_NAME "SMS_INDEX"

#define MSG_ENDING 0x1A

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
#define ONLY_IF_WAITING 8
#define USE_LAST_INPUT 16
typedef struct {
    const char* response;
    void(*f)(const char*s);
    const char* cmd;
    void(*successFunction)(const char*s);
    int flags;
} Response;
#endif

