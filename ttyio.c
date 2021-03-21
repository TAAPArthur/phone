#include <poll.h>
#include <signal.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <string.h>
#include "ttyio.h"
#define LEN(A) (sizeof(A)/sizeof(A[0]))

#ifndef NDEBUG
#define DEBUG(X...) dprintf(2, X)
#endif
int ttyFD;
int status = SUCCESS;
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
void writeData(const char* s) {
    write(ttyFD, s, strlen(s));
    setWaiting(1);
}

static int subCmdFD[2];

void startReadingSMS() {
    readingSMS = 1;
    if(pipe(subCmdFD) ==-1){
        perror("pipe");
    }

    if(!fork()) {
        dup2(subCmdFD[0], STDIN_FILENO);
        close(subCmdFD[1]);
        close(subCmdFD[0]);
        execl(SMS_READ_CMD, SMS_READ_CMD);
    }
    close(subCmdFD[0]);
}

void commitSMSRead() {
    readingSMS = 0;
    close(subCmdFD[1]);
    wait(NULL);
}

void receiveSMSNotification(const char*s) {
    char messageArea[3]={0};
    char index[4];
    int ret = sscanf(s, "%*s \"%2s\",%3s", messageArea, &index);
    setenv(SMS_INDEX_ENV_NAME, index, 1);
    DEBUG("Received SMS message %d, ME:'%s' index: %s\n",ret, messageArea, index);
    char buffer[16];
    sprintf(buffer,"AT+CMGR=%s%s", index, LN_ENDING);
    writeData(buffer);
    startReadingSMS();
}
void readSMS(const char*s) {
    if(readingSMS) {
        int len =0 ;
        int ret= sscanf(s, "%*s %*d,,%d", &len);
        if(ret==0)
            ret = sscanf(s, "%*s %*d,%*d,%d", &len);
        DEBUG("%d, %d\n",ret, len);
    }
}

void processResponse(char* response) {
    for(int i=0;i<LEN(responses);i++) {
        if(memcmp(responses[i].response, response, strlen(responses[i].response)) == 0) {
            if(readingSMS)
                commitSMSRead();
            if(responses[i].f) {
                responses[i].f(response);
                return;
            }
        }
    }
    if(readingSMS) {
       if(-1 == write(subCmdFD[1], response, strlen(response))){
           perror("Write");
       }
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
void onStart(int ttyFD) {
    char buf[4096]={0};
    struct pollfd fds[] = {{ttyFD, POLLIN}};
    for(int i = 0; i < LEN(onStartCmds); i++) {
        writeData(onStartCmds[i]);
        writeData(LN_ENDING);
        while(isWaiting()){
            int ret = poll(fds, 1, -1);
            if(ret = -1) {
                perror("Poll");
                exit(1);
            }
            int chars = readLine(ttyFD, buf);
            if (chars) {
                DEBUG("'%s' (%ld)\n",  buf, strlen(buf));
                processResponse(buf);
            }
        }
    }
}
int main(int args, const char* argv[]) {

    signal(SIGPIPE, SIG_IGN);
    ttyFD = open(argv[1]?argv[1]:device, O_RDWR|O_NONBLOCK);
    if(ttyFD==-1) {
        exit(2);
    }
    DEBUG("Initializing\n");
    onStart(ttyFD);
    DEBUG("Starting\n");

    struct pollfd fds[] = {{ttyFD, POLLIN}, {STDIN_FILENO, POLLIN}};
    int numFDs = LEN(fds);
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
                        DEBUG("'%s' (%ld)\n",  buf, strlen(buf));
                        processResponse(buf);
                        if(isWaiting() && numFDs ==1)
                            exit(2);
                    } else if(!isWaiting()) {
                        if(strncmp(buf, "DONE:", 5)==0) {
                            DEBUG("DEBUG2: '%s' (%ld)\n",  buf+5, strlen(buf));
                            buf[strlen(buf)] = MSG_ENDING;
                            writeData(buf+5);
                        }
                        else {
                            DEBUG("DEBUG: '%s' (%ld) %d\n",  buf, strlen(buf), fds[i].revents);
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
                    DEBUG("End of stdin: '%s' %d\n\n", buf, chars);
                    numFDs--;
                    break;
                }
            }
        }
    }
}
