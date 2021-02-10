#!/bin/tcc -run
#define _XOPEN_SOURCE
#include <poll.h>
#include <signal.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#define LEN(A) (sizeof(A)/sizeof(A[0]))

const char LN_ENDING[] ="\r\n";
const char MSG_ENDING=0x1A;
const char * device = "/dev/ttyUSB2";
int ttyFD;
#define SUCCESS 0
#define WAITING 3
#define ERROR 2
#define IN_PROGRESS 4
#define UNDEFINED 255
int status = UNDEFINED;
int waiting = 0;
int isWaiting() {
    return waiting;
}
int readingSMS;
void setWaiting(int i) {
    waiting = i;
}
void setStatus(int s) {
    setWaiting(0);
    status = s;
    readingSMS = 0;
}
void markSuccess(){setStatus(SUCCESS);}
void markError(){setStatus(ERROR);}
void clearWaiting(){setStatus(IN_PROGRESS);}
void writeData(const char* s) {
    write(ttyFD, s, strlen(s));
}

int subCmdFDs[2];

void startReadingSMS() {
    readingSMS = 1;
    pipe(subCmdFDs);
    if(!fork()) {
        dup2(subCmdFDs[0], STDIN_FILENO);
        close(subCmdFDs[1]);
        close(subCmdFDs[0]);
        execl("/bin/sh", "/bin/sh", "-c", "cat -");
    }
    close(subCmdFDs[0]);
}

void commitSMSRead() {
    printf("Committing SMS read\n");
    readingSMS = 0;
    close(subCmdFDs[1]);
}

void receiveSMSNotification(const char*s) {
    char messageArea[3]={0};
    int index = 0 ;
    int ret= sscanf(s, "%*s \"%2s\",%d", messageArea, &index);
    printf("%d, ME:'%s' index: %d\n",ret, messageArea, index);
    char buffer[16];
    sprintf(buffer,"AT+CMGR=%d%s", index, LN_ENDING);
    writeData(buffer);
    startReadingSMS();
}
void readSMS(const char*s) {
    if(readingSMS) {
        int len =0 ;
        int ret= sscanf(s, "%*s %*d,,%d", &len);
        if(ret==0)
            ret = sscanf(s, "%*s %*d,%*d,%d", &len);
        printf("%d, %d\n",ret, len);
    }
}
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

void processResponse(char* response) {
    for(int i=0;i<LEN(responses);i++) {
        if(memcmp(responses[i].response, response, strlen(responses[i].response)) == 0) {
            if(readingSMS)
                commitSMSRead();
            if(responses[i].f) {
                printf("Running command %d\n", i);
                responses[i].f(response);
                return;
            }
        }
    }
    if(readingSMS)
       if(-1== write(subCmdFDs[1], response, strlen(response))){
           perror("Write");
       }
}


int readLine(int fd, char*buffer) {
    int len = 255;
    int i;
    for(i=0;i<len;i++){
        int ret = read(fd, buffer+i, 1);

        if(ret == 0)
            break;
        else if(ret == -1) {
            break;
        }
        if(buffer[i] == '\n' || buffer[i] == '\r'){
            buffer[i]=0;
            break;
        }
    }
    return i;
}
int main(int args, const char* argv[]) {

    signal(SIGPIPE, SIG_IGN);
    ttyFD = open(argv[1]?argv[1]:device, O_RDWR|O_NONBLOCK);
    if(ttyFD==-1) {
        exit(2);
    }
    struct pollfd fds[] = {{ttyFD, POLLIN}, {STDIN_FILENO, POLLIN}};
    int numFDs = LEN(fds);
    printf("Starting\n");
    while(poll(fds, numFDs, -1) >= 0) {
        char buf[4096]={0};
        for(int i = 0; i < LEN(fds); i++) {
            if(fds[i].revents & POLLIN) {
                if(isWaiting() && ttyFD != fds[i].fd) {
                    sleep(1);
                    continue;
                }
                int chars = readLine(fds[i].fd, buf);
                if (chars) {
                    if(ttyFD == fds[i].fd) {
                        printf("'%s' (%ld)\n",  buf, strlen(buf));
                        processResponse(buf);
                        if(isWaiting() && numFDs ==1)
                            exit(status);
                    } else if(!isWaiting()) {
                        if(strncmp(buf, "DONE:", 5)==0) {
                            printf("DEBUG2: '%s' (%ld)\n",  buf+5, strlen(buf));
                            buf[strlen(buf)] = MSG_ENDING;
                            writeData(buf+5);
                        }
                        else {
                            printf("DEBUG: '%s' (%ld) %d\n",  buf, strlen(buf), fds[i].revents);
                            writeData(buf);
                            writeData(LN_ENDING);
                            setWaiting(1);
                        }
                    }
                }
                break;
            }
            else if(fds[i].revents & (POLLERR | POLLNVAL | POLLHUP)) {
                if(fds[i].fd == ttyFD) {
                    perror("Poll dead");
                    exit(1);
                } else {
                    if(!isWaiting())
                        exit(status);
                    int chars = readLine(fds[i].fd, buf);
                    printf("End of stdin: '%s' %d\n\n", buf, chars);
                    numFDs--;
                    break;
                }
            }
        }
    }
}
