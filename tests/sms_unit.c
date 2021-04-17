#include <scutest/tester.h>
#include "../sms.h"
#include <assert.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>

char readBits(uint8_t *byte, int N);
char readBit(uint8_t *byte);
char writeBits(uint8_t *byte, uint8_t bits, int N);
char writeBit(uint8_t *byte, bool bit);


SCUTEST(read_write_single_bit) {
    uint8_t buffer = 0;
    const char bits[8] = { 0,0,1,1,0,1,1,1};
    for(int i=0; i < 8; i++)
        writeBit(&buffer, bits[i]);
    assert(buffer == 0b00110111);
    for(int i=7; i>=0; --i) {
        assert(readBit(&buffer) == bits[i]);
    }
}
SCUTEST(read_write_bits) {
    uint8_t buffer = 0;
    writeBits(&buffer, 0b10, 2);
    assert(buffer == 0b10);
    writeBits(&buffer, 0b01, 2);
    assert(buffer == 0b1001);
    assert(readBits(&buffer, 2) == 0b01);
    assert(buffer == 0b10);
    assert(readBits(&buffer, 2) == 0b10);
    assert(buffer == 0);
}


void decodeSept(const char*str, int len, char* buffer);
SCUTEST(test_decode_sept) {
    char buffer[255];
    const char*str="C8329BFD06DDDF723619";
    decodeSept(str, strlen(str), buffer);
    printf("%s", buffer);
    assert(strcmp("Hello world", buffer) == 0);
}
/*
SCUTEST(test_decode_padded_sept) {
    char buffer[255];


    const char*str="050003CC0202906536FB0DBABFE56C32";
    decodeSept(str, strlen(str), buffer);
    printf("%s\n", buffer);
    printf("%s\n", buffer+strlen(buffer)+1);
    assert(strcmp("Hello world", buffer) == 0);
}
*/

int encodeSept(const char*str, int len, char* buffer);
int encodeSeptWithPadding(const char*str, int len, int padding, char* buffer);
SCUTEST_ITER(test_encode_padded_sept, 2) {
    const char* msg="Hello world";
    char buffer[255]={0};
    encodeSeptWithPadding(msg, strlen(msg), _i, (char*)buffer);
    if(_i)
        assert(strcmp(buffer, "906536fb0dbabfe56c32")==0);
    else
        assert(strcmp(buffer, "c8329bfd06dddf723619")==0);


}

unsigned char readByte(const char**str);
void decodeSeptWithPadding(const char*str, int len, int padding, char* buffer);
SCUTEST_ITER(test_decode_padded_sept2, 2) {
    const char*str;
    if(_i)
        str="906536fb0dbabfe56c32";
    else
        str="c8329bfd06dddf723619";
    char buffer[255]={0};
    int padding = _i;
    decodeSeptWithPadding(str, strlen(str), padding?6:0, buffer);
    assert(strcmp(buffer, "Hello world")==0);
}

int main(int argc, char * argv[]) {
    return runUnitTests();
}


