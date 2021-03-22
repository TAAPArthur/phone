#include <scutest/tester.h>
#include <assert.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>

char* processMetadata(char*buffer);
extern char lastLabel[];
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

int main(int argc, char * argv[]) {
    return runUnitTests();
}
