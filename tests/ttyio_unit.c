#define SCUTEST_DEFINE_MAIN
#define SCUTEST_IMPLEMENTATION
#include <scutest/scutest.h>

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "../ttyio.h"

char* processMetadata(char*buffer);
extern char lastLabel[];
extern int ttyFD;
void receiveSMSNotification(const char*s);
SCUTEST(test_process_metadata) {
    char buffer[] = "#LABEL=123456 ATA";
    char*data=processMetadata(buffer);
    assert(strcmp(lastLabel, "123456") == 0);
    assert(strcmp(data, "ATA") == 0);
}

SCUTEST(test_process_metadata_noop) {
    char buffer[] = "ATA";
    char*data=processMetadata(buffer);
    assert(strcmp(data, buffer) == 0);
}

SCUTEST(test_receive_sms) {
    int fds[2];
    pipe(fds);
    ttyFD = fds[1];
    receiveSMSNotification("+CMTI: \"ME\",10");
    char buffer[16] ={0};
    read(fds[0], buffer, sizeof(buffer));
    assert(strcmp(buffer,"AT+CMGR=10\r\n")==0);
}

int spawnResponse(Response* response, const char* arg) ;
SCUTEST(test_spawn_response) {
    Response success = {"", .cmd="exit 0"};
    Response failure = {"", .cmd="exit 1"};
    assert(0 == spawnResponse(&success, ""));
    assert(1 == spawnResponse(&failure, ""));
}


int processArgs(const char*const * argv);
SCUTEST(test_argument_check, .exitCode=3) {
    processArgs((const char*[]){"/dev/null", NULL});
    processArgs((const char*[]){"-c", "/dev/null", NULL});
}

SCUTEST(test_argument_check_only) {
    processArgs((const char*[]){"-c", "/dev/null", NULL});
    exit(10);
}

SCUTEST(test_argument_bad_device, .exitCode=2) {
    processArgs((const char*[]){"-c", "/this_file_does_not_exit...probabl", NULL});
}

SCUTEST(test_argument_no_lock) {
    processArgs((const char*[]){"-n", "/dev/null", NULL});
    processArgs((const char*[]){"-n", "-c", "/dev/null", NULL});
    processArgs((const char*[]){"-n", "/dev/null", NULL});
}


SCUTEST(test_argument_lock_different_device) {
    processArgs((const char*[]){"/dev/null", NULL});
    processArgs((const char*[]){"/dev/zero", NULL});
}
