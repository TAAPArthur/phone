#include <scutest/tester.h>
#include <assert.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
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
    int ret = read(fds[0], buffer, sizeof(buffer));
    assert(strcmp(buffer,"AT+CMGR=10\r\n")==0);
}

int spawnResponse(Response* response, const char* arg) ;
SCUTEST(test_spawn_response) {
    Response success = {"", .cmd="exit 0"};
    Response failure = {"", .cmd="exit 1"};
    assert(0 == spawnResponse(&success, ""));
    assert(1 == spawnResponse(&failure, ""));
}

int main(int argc, char * argv[]) {
    return runUnitTests();
}
