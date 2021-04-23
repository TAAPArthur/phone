#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <poll.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/file.h>
#include <sys/wait.h>
#include <unistd.h>
#include "ttyio.h"
#include "config.h"
#define LEN(A) (sizeof(A)/sizeof(A[0]))

#ifndef NDEBUG
#define DEBUG(X...) dprintf(2, X)
#endif

#define OUTPUT(X...) do {dprintf(1, X); DEBUG(X);} while(0)

const char MSG_ENDING_STR[] = {MSG_ENDING};
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
    if(write(ttyFD, s, strlen(s)) == -1) {
        perror("Failed to write data");
        exit(1);
    }
    setWaiting(1);
}


int spawn(const char* cmd) {
    DEBUG("Executing command %s\n", cmd);
    int pid = fork();
    if(!pid) {
        dup2(STDERR_FILENO, STDOUT_FILENO);
        execlp(SHELL, SHELL, "-c", cmd, NULL);
        perror("Failed exec");
        exit(1);
    }

    int status = 0;
    waitpid(pid, &status, 0);
    int exitCode = WIFEXITED(status) ? WEXITSTATUS(status) : WIFSIGNALED(status) ? WTERMSIG(status) : -1;
    DEBUG("cmd exited with status %d\n", exitCode);
    return exitCode;
}

int spawnResponse(Response* response, const char* arg) {
    char buffer[CMD_BUFFER];
    const char*cmd = response->cmd;

    if(response->flags & ADD_RESPONSE_FLAG) {
        if(response->flags & STRIP_LABEL)
            arg += strlen(response->response);
        sprintf(buffer, response->cmd, arg);
        cmd = buffer;
    }
    int exitCode = spawn(cmd);
    if(exitCode == 0) {
        if (response->successFunction)
            response->successFunction(arg);
    }
    unsetenv(SMS_INDEX_ENV_NAME);
    return exitCode;
}

void receiveSMSNotification(const char*s) {
    char messageArea[3]={0};
    int index;
    int ret = sscanf(s, "%*s \"%2s\",%d", messageArea, &index);
    DEBUG("Received SMS message ME:'%s' index: %d\n", messageArea, index);
    assert(ret==2);
    char buffer[16];
    sprintf(buffer,"AT+CMGR=%d%s", index, LN_ENDING);
    writeData(buffer);
}


void deleteSMS() {
    char buffer[16];
    if(getenv(SMS_INDEX_ENV_NAME)) {
        sprintf(buffer,"AT+CMGD=%s%s", getenv(SMS_INDEX_ENV_NAME), LN_ENDING);
        writeData(buffer);
    }
}
void readSMS(const char*s) {
    int index;
    int ret = sscanf(s, "%*s %d,", &index);
    if(ret >=0) {
        char buffer[4];
        sprintf(buffer, "%d", index);
        setenv(SMS_INDEX_ENV_NAME, buffer, 1);
    }
    else
        unsetenv(SMS_INDEX_ENV_NAME);
    setWaiting(1);
}

void processResponse(const char* response) {
    for(int i=0;i<LEN(responses);i++) {
        if(memcmp(responses[i].response, response, strlen(responses[i].response)) == 0 && (!(responses[i].flags & ONLY_IF_WAITING) || isWaiting())) {
            lastResponse = responses + i;
            DEBUG("Triggering response %d: %s\n", i, responses[i].response);
            if(responses[i].f) {
                responses[i].f(response);
                return;
            } else if(responses[i].cmd) {
                spawnResponse(&responses[i], response);
                return;
            }
        }
    }
    if(lastResponse && lastResponse->cmd && (lastResponse->flags & MULTI_LINE_FLAG)) {
        spawnResponse(lastResponse, response);
    }
    lastResponse = NULL;
}

int readLine(int fd, char*buffer) {
    int len = 512;
    int i;
    for(i=0;i<len;i++){
        int ret = read(fd, buffer+i, 1);

        if(ret == 0) {
            if(i == 0)
                return -1;
            else
                break;
        }
        else if(ret == -1) {
            perror("error reading?");
            break;
        }
        if(buffer[i] == '\n' || buffer[i] == '\r'){
            buffer[i]=0;
            break;
        }
    }
    buffer[i]=0;
    return i;
}
void onStart(int ttyFD) {
    // end any existing message prompt
    writeData(MSG_ENDING_STR);
    setWaiting(0);
    struct pollfd fds[] = {{ttyFD, POLLIN}};
    for(int i = 0; i < LEN(onStartCmds); i++) {
        writeData(onStartCmds[i]);
        writeData(LN_ENDING);
        while(isWaiting()){
            char buf[4096]={0};
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
    if(strncmp("MSG_TERM", buffer, 8) == 0) {
        buffer[strlen(buffer) + 1] = 0;
        buffer[strlen(buffer)] = MSG_ENDING;
    } else {
        sscanf(buffer, "LABEL=%255s", lastLabel);
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
void usage() {
    printf("ttyio [-chn] [device]\n");
}
int processArgs(const char* const* argv){
    int noLock = 0;
    int checkOnly = 0;
    for(; argv[0] && argv[0][0] == '-' ; argv++) {
        if(argv[0][1] == '-') {
            argv++;
            break;
        }
        switch(argv[0][1]){
            case 'h':
                usage();
                exit(0);
            case 'c':
                checkOnly = 1;
                break;
            case 'n':
                noLock = 1;
                break;
            default:
                usage();
                exit(1);
        }
    }
    const char* path = argv[0]?argv[0]:device;

    int fd = open(path, O_RDWR|O_NONBLOCK|O_CLOEXEC);
    if(fd ==-1) {
        perror("Failed open file");
        exit(2);
    }
    if(!noLock) {
        if(flock(fd, LOCK_EX | LOCK_NB) == -1){
            if(errno == EWOULDBLOCK)
                exit(3);
            else {
                perror("Failed to lock file");
                exit(2);
            }
        }
    }
    if(checkOnly)
        exit(0);
    for(int i = 0; i < LEN(onStartDeviceCmds); i++) {
        char buffer[255];
        snprintf(buffer, LEN(buffer), onStartDeviceCmds[i], path);
        spawn(buffer);
    }
    return fd;
}

void handEndOfFD(int fd) {
    if(fd == STDIN_FILENO) {
        if(!isWaiting())
            exit(status);
    } else {
        perror("Poll dead");
        exit(1);
    }
}
int __attribute__((weak)) main(int argc, const char * argv[]) {
    signal(SIGPIPE, SIG_IGN);
    ttyFD  = processArgs(argv + 1);
    DEBUG("Initializing\n");
    onStart(ttyFD);
    DEBUG("Starting\n");

    struct pollfd fds[] = {{ttyFD, POLLIN}, {STDIN_FILENO, POLLIN}};
    int numFDs = LEN(fds);
    while(poll(fds, numFDs, -1) >= 0) {
        char buf[4096] = {0};
        assert(numFDs);
        for(int i = 0; i < numFDs; i++) {
            if(fds[i].revents & POLLIN) {
                if(isWaiting() && ttyFD != fds[i].fd) {
                    DEBUG("Waiting on modem\n");
                    sleep(1);
                    break;
                }
                int ret = readLine(fds[i].fd, buf);
                if(ret == -1) {
                    DEBUG("Reached end of device %d", i);
                    handEndOfFD(fds[i].fd);
                    numFDs--;
                    break;
                }

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
                DEBUG("FD error");
                handEndOfFD(fds[i].fd);
            }
        }

        if(!isWaiting() && numFDs ==1)
            exit(0);
    }
}
