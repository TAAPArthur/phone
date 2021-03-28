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

#define OUTPUT(X...) dprintf(1, X)
int ttyFD;
int status = SUCCESS;
int waiting = 0;
char lastLabel[255];
static Response* lastResponse;
int isWaiting() {
    return waiting;
}
int readingSMS;
void setWaiting(int i) {
    waiting = i;
}
void setStatus(int s) {
    lastResponse = NULL;
    setWaiting(0);
    status = s;
    readingSMS = 0;
}
void signalLastProcess(int error) {
    if(lastLabel[0]) {
        OUTPUT("%s TERM %d\n", lastLabel, error);
        lastLabel[0] = 0;
    }
}
void echoResponse(const char* response) {
    if(lastLabel[0]) {
        OUTPUT("%s MSG %s\n", lastLabel, response);
    }
}

void markSuccess(){
    signalLastProcess(0);
    setStatus(SUCCESS);
}
void markError(){
    signalLastProcess(1);
    setStatus(ERROR);
}
void clearWaiting(){setStatus(IN_PROGRESS);}
void writeData(const char* s) {
    write(ttyFD, s, strlen(s));
    setWaiting(1);
}

void spawn(const char* cmd, const char* arg) {
    DEBUG("Executing command %s with arg '%s'\n", cmd, arg);
    if(!fork()) {
        int ret = execlp(cmd, cmd, arg, NULL);
        perror("Failed exec");
        exit(1);
    }
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
}
void readSMS(const char*s) {
    char index[4];
    int ret= sscanf(s, "%*s %d,", &index);
    setenv(SMS_INDEX_ENV_NAME, index, 1);
    setWaiting(1);
}

void processResponse(const char* response) {
    for(int i=0;i<LEN(responses);i++) {
        if(memcmp(responses[i].response, response, strlen(responses[i].response)) == 0) {
            lastResponse = responses + i;
            DEBUG("Triggering response %d: %s\n", i, responses[i].response);
            if(responses[i].f) {
                responses[i].f(response);
                return;
            } else if(responses[i].cmd) {
                spawn(responses[i].cmd, response);
                return;
            }
        }
    }
    if(lastResponse && lastResponse->cmd && (lastResponse->flags & MULTI_LINE_FLAG)) {
        spawn(lastResponse->cmd, response);
    }
}

int readLine(int fd, char*buffer) {
    int len = 512;
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
    // end any existing message prompt
    writeData(MSG_ENDING_STR);
    setWaiting(0);
    char buf[4096]={0};
    struct pollfd fds[] = {{ttyFD, POLLIN}};
    for(int i = 0; i < LEN(onStartCmds); i++) {
        writeData(onStartCmds[i]);
        writeData(LN_ENDING);
        while(isWaiting()){
            int ret = poll(fds, 1, -1);
            if(ret == -1) {
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
static void processMetadataHelper(char* buffer){
    int ret;
    if(strncmp("MSG_TERM", buffer, 8) == 0) {
        buffer[strlen(buffer) + 1] = 0;
        buffer[strlen(buffer)] = MSG_ENDING;
    } else {
        sscanf(buffer, "LABEL=%255s", &lastLabel);
    }
}
char* processMetadata(char* buffer){
    if(buffer[0]=='#') {
        buffer++;
        if(buffer[0] == '#')
            return buffer;
        processMetadataHelper(buffer);
        buffer = strstr(buffer, " ");
        if(buffer)
            buffer++;
    }
    return buffer;
}
int __attribute__((weak)) main(int argc, char * argv[]) {

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
                    break;
                }
                readLine(fds[i].fd, buf);
                const char* data = processMetadata(buf);
                if (data && data[0]) {
                    if(ttyFD == fds[i].fd) {
                        DEBUG("RESPONSE: '%s' (%ld)\n",  data, strlen(data));
                        echoResponse(data);
                        processResponse(data);
                    } else if(!isWaiting()) {
                        DEBUG("INPUT   : '%s' (%ld)\n",  data, strlen(data));
                        writeData(data);
                        writeData(LN_ENDING);
                        setWaiting(1);
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

        if(!isWaiting() && numFDs ==1)
            exit(0);
    }
}
