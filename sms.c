#!/bin/tcc -run
#include <stdio.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <fcntl.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include "sms.h"
#include <sys/types.h>
#include <stdint.h>
typedef char bool ;
#ifndef NDEBUG
#define DEBUG(X...) dprintf(2, X)
#define TRACE(X...) dprintf(2, X)
#else
#define DEBUG(X...)
#define TRACE(X...)
#endif

char FIELD_SEPARATOR = ' ';
char LN_TERMINATOR = '\n';

void die(const char*msg){
    perror(msg);
    exit(1);
}
void _convert(const char*c, char len, unsigned char* buffer) {
    for(int i=0;i<len;i+=2) {
        sscanf(c+i, "%2hhx", buffer+i/2);
    }
}
char readBinaryEncodedOctetSigned(const char**str, bool asSigned) {
    char buffer[2] = {str[0][1], str[0][0]};
    if(asSigned) {
        if (buffer[0] >= '8' && buffer[0] < 'A')
            buffer[0] -= 8;
        else if (buffer[0] >= 'A')
            buffer[0] = buffer[0] - 'A' + '9' - 7;
    }
    unsigned char c=0;

    sscanf(buffer, "%2hhd", &c);
    (*str)+=2;
    return c;
}
unsigned char readByte(const char**str) {
    unsigned char c=0;
    _convert(*str, 2, &c);
    (*str)+=2;
    return c;
}
void writeByte(char byte, char**ptr) {
    sprintf(*ptr, "%02hhx", byte);
    *ptr+=2;
}
void decodeSept(const char*str, int len, char* buffer) {
    const int numOctets = len*8/7;
    unsigned char mask = -1;
    unsigned char carry = 0;
    int n=0;
    for(int i=0;i<numOctets;i++) {
        if(i%8 == 7 && i) {
            buffer[n++] = carry;
            carry = 0;
            mask = -1;
            continue;
        }
        mask = mask >> 1;
        unsigned char rawByte = readByte(&str);

        buffer[n++] = ((rawByte& mask) << (i%8)) | carry ;
        carry = (rawByte & ~(mask)) >> (7-i%8);
    }
    buffer[n]=0;
    assert(!str[0]);
}
int encodeSept(const char*str, int len, char* buffer) {
    unsigned char mask = -1;
    int n=0;
    for(int i=0;i<len && str[i];i++) {
        if(i%8 == 7) {
            mask = -1;
            continue;
        }
        unsigned char rawByte = str[i];
        unsigned char nextByte = str[i+1];
        sprintf(buffer+n, "%02hhx", ((rawByte& mask) >> (i%8)) | (0xFF & (nextByte << (7-i%8))));
        n+=2;
        mask = mask << 1;
    }
    DEBUG("N: %d len:%d\n", n, len);
    DEBUG("%ld %ld\n", strlen(str), strlen(buffer));
    buffer[n]=0;
    return strlen(buffer);
}


char buffer[1000]={0};
int swapNibble(const char*c, int len, char*buffer) {
    // convert from swap nibble
    int i;
    for(i=0; i<len;i+=2) {
        buffer[i] = c[i+1];
        buffer[i+1] = c[i];
    }
    return i;
}

char* readTime(const char**c, char*result) {
    for(int i=0;i<7;i++)
        result[i]=readBinaryEncodedOctetSigned(c, i==6);
    return result;
}
char* readPhoneNumber(const char**c, int len, char*result) {
    *c+=swapNibble(*c, len, result);
    if(result[len]=='F') {
        result[len] = 0;
    }
    return result;
}
int encodePhoneNumber(const char*c, int len, char* buffer) {
    swapNibble(c, len, buffer);
    if(len %2 ) {
        buffer[len-1] = 'F';
    }
    return len + len %2 ;
}


char readBits(uint8_t *byte, int N) {
    char bits = *byte & N;
    *byte >>=N;
    return bits;
}
char readBit(uint8_t *byte) {
    return readBits(byte, 1);
}

char writeBits(uint8_t *byte, uint8_t bits, int N) {
    *byte <<= N;
    return *byte|=bits;
}
char writeBit(uint8_t *byte, bool bit) {
    return writeBits(byte, bit, 1);
}

void readPhoneAddress(const char**c, char*number) {
    char lengthOfPhoneNumber=readByte(c);
    readByte(c);// number type
    readPhoneNumber(c, lengthOfPhoneNumber, number);
    assert(strlen(number)==lengthOfPhoneNumber);
}

void decodeUCS2(const char*str, int len, char* buffer) {

    for(int i=0;i<len;i++) {
        readByte(&str);
        char c = readByte(&str);
        buffer[i] = c;
    }
    buffer[len]=0;
}
void decodeUserMessage(const char**c, uint8_t type, char* buffer) {
    uint8_t dataLength=readByte(c);
    GSMEncodingType  dateEncoding = GSM_7_BIT;
    GSMEncodingType  TYPES[]={GSM_7_BIT, GSM_8_BIT, GSM_UCS2 };
    if(type <= 127 || type >= 240)
        dateEncoding = TYPES[type/4 %4];
    printf("%s\n", *c);
    printf("%ld\n", strlen(*c));
    int f=open("hex.dump", O_CREAT|O_RDWR, 0666);
    perror("open");
    assert(f!=-1);
    switch(dateEncoding ) {
        case GSM_7_BIT:
            decodeSept(*c, dataLength, buffer);
            break;
        case GSM_8_BIT:
            for(int i=0;*c && i<102;i++){
                char a= readByte(c);
                write(f, &a, 1);
                //printf("%c", a);
                printf("'%s' (%ld)\n",*c, strlen(*c));
            }
            break;

            for(int i=0;*c;i++){
                buffer[i] = readByte(c);
            }
            break;
        case GSM_UCS2:
            decodeUCS2(*c, dataLength, buffer);
            break;
        default:
            die("Unsupported gsm type");
    }
}
int setenvInt(const char*name, int value) {
    char buffer[5]={0};
    sprintf(buffer, "%d", value);
    return setenv(name, buffer, value);
}

char*writeConcatenatedSMS(char*ptr, int refNumber, int totalSize, int index,  int*messagesLeftAfterThis ){
    int numParts = (totalSize -1) / MAX_CONCAT_SMS_LEN +1;
    *messagesLeftAfterThis = numParts - index - 1;

    writeByte(messagesLeftAfterThis?MAX_SMS_LEN:UDH_CONCAT_SMS_HEADER_LEN + totalSize %MAX_CONCAT_SMS_LEN,  &ptr);
    writeByte(UDH_CONCAT_SMS_HEADER_LEN, &ptr);
    writeByte(CONCAT_SMS_8_BIT_REF_NUMBER, &ptr);
    writeByte(UDH_CONCAT_SMS_HEADER_LEN-2, &ptr);
    writeByte(0, &ptr); // leading bits of ref number used mainly to avoid having to force 7-bit aligment
    writeByte(refNumber, &ptr);
    writeByte(numParts, &ptr);
    writeByte(index, &ptr);
    DEBUG("Total parts %d %d\n", numParts, index );
    return ptr;
}

void readConcatenatedSMS(const char**c, int lengthOfRestOfHeader ){
    int csmsRefNumber = readByte(c);
    if(lengthOfRestOfHeader ==4) {
        csmsRefNumber = (csmsRefNumber <<8) |readByte(c);
    }
    readByte(c); // total parts
    readByte(c); // seqNumber
    //printf("%c%d %d %d", FIELD_SEPARATOR, csmsRefNumber, seqNumber, totalParts);
}

void readUserHeader(const char**c){
    readByte(c);// header len
    int infoElementIdentifier = readByte(c);
    int lengthOfRestOfHeader = readByte(c);
    switch(infoElementIdentifier) {
        case CONCAT_SMS_8_BIT_REF_NUMBER:
        case CONCAT_SMS_16_BIT_REF_NUMBER:
            readConcatenatedSMS(c, lengthOfRestOfHeader);
            break;
        default:
            *c+=lengthOfRestOfHeader;
    }
}
void decodeUserHeaderAndMessage(const char**c, bool userDataHeaderPresent,char type){
    char message[MAX_SMS_LEN] = {0};
    if(userDataHeaderPresent) {
        readUserHeader(c);
    }
    decodeUserMessage(c,type, message);
    printf("%c%s%c", FIELD_SEPARATOR, message, LN_TERMINATOR);
}

void printTimeNumber(char timeParts[7], char*number){
    if(timeParts)
        printf("Time:%c%02d/%02d/%02d %d:%d:%d %d%c", FIELD_SEPARATOR, timeParts[0], timeParts[1], timeParts[2], timeParts[3], timeParts[4], timeParts[5], timeParts[6], LN_TERMINATOR);
    else
        printf("Time:%c0%c", FIELD_SEPARATOR, LN_TERMINATOR);
    printf("Number:%c%s%c", FIELD_SEPARATOR, number, LN_TERMINATOR);
}

void decodeSMSDeliver(const char**c, uint8_t firstByte) {
    readBit(&firstByte); // more messages to send
    readBits(&firstByte, 2); //Loop prevention
    readBit(&firstByte); //Status report
    bool userDataHeaderPresent = readBit(&firstByte);
    readBit(&firstByte); //replyPath
    assert(firstByte == 0);
    char number[MAX_PHONE_NUMBER_SIZE]={0};
    readPhoneAddress(c, number);
    readByte(c);// TP-PID Protocol identifier
    char dataCoding = readByte(c);
    char timeParts[7];
    readTime(c, timeParts);
    printTimeNumber(timeParts, number);
    decodeUserHeaderAndMessage(c, userDataHeaderPresent, dataCoding);
}

void decodeSMSSubmit(const char**c, uint8_t firstByte) {
    readBit(&firstByte); // reject duplicates
    char validityPeriod = readBits(&firstByte, 2); // validity period
    readBit(&firstByte); //Status report
    bool userDataHeaderPresent = readBit(&firstByte);
    readBit(&firstByte); //replyPath
    readByte(c); // messageRef
    char number[MAX_PHONE_NUMBER_SIZE ];
    readPhoneAddress(c, number);
    readByte(c);// TP-PID Protocol identifier
    char dataCoding = readByte(c);
    if(validityPeriod ) {
        // skip validity info
        if(validityPeriod ==2)
            readByte(c);
        else
            *c+=2*7;
    }
    printTimeNumber(NULL, number);
    decodeUserHeaderAndMessage(c, userDataHeaderPresent, dataCoding);
}

void decodeSMSMessage(const char*c) {
    char smscLen=readByte(&c);
    c+=smscLen*2;
    uint8_t pduType=readByte(&c);
    char messageTypeIndicator = readBits(&pduType, 2);
    DEBUG("PDU_TYPE %x\n", pduType);
    switch(messageTypeIndicator) {
        case SMS_DELIVER:
            decodeSMSDeliver(&c, pduType);
            break;
        case SMS_SUBMIT:
            decodeSMSSubmit(&c, pduType);
            break;
    }
}

void encodeUserMessage(char*c, const char*msg, int msgLen, GSMEncodingType type) {
    DEBUG("%d\n", type);
    switch(type) {
        case GSM_7_BIT:
            encodeSept(msg, msgLen, c);
            break;
        case GSM_8_BIT:
            for(int i=0;i<msgLen && msg[i];i++) {
                sprintf(c+i*2, "%02hhx", (char)msg[i]);
            }
            break;
        default:
            die("Unsupported gsm type for encoding");
    }
}
int getMaxMessageSizePerType(GSMEncodingType type) {
    switch(type) {
        case GSM_7_BIT: return 160;
        case GSM_8_BIT: return 140;
        default:
        case GSM_UCS2: return 70;
    }
}
void encodeSMSMessage(const char*number, const char*msg, int type) {
    char buffer[MAX_TOTAL_SMS_LEN ] = {0};
    const int realMessageLen = strlen(msg);
    DEBUG("Message len: %d\n", realMessageLen);
    bool splitMessage = realMessageLen > MAX_SMS_LEN;
    char *ptr=buffer;
    writeByte(0x00, &ptr); // SMSC info

    uint8_t pduType = 0;
    writeBit(&pduType , 0); // reply path; use same path as originator
    writeBit(&pduType , splitMessage); // user data field present
    writeBit(&pduType , 1); // status report request
    writeBits(&pduType , 0, 2); // validity info
    writeBit(&pduType, 1); // Reject duplicates
    writeBits(&pduType , SMS_SUBMIT, 2);
    writeByte(pduType, &ptr);
    DEBUG("0x%x\n",pduType);

    writeByte(0x00, &ptr); // tp-message-ref; will this be auto generated?

    // destination address
    writeByte(strlen(number), &ptr); // address length
    writeByte(0x91, &ptr); // typeof address
    ptr+=encodePhoneNumber(number, strlen(number), ptr);

    writeByte(0x00, &ptr); // tp-pid
    writeByte(type, &ptr); // Data coding scheme
    if(splitMessage) {
        DEBUG("Using concatenated sms messages; current buffer: \n%s\n", buffer);
        int refNumber=rand()%255;
        int i=0;
        int remainder;
        do{
            DEBUG("Message len: %d %d\n", realMessageLen, type);
            char*userMessageStart= writeConcatenatedSMS(ptr,refNumber, realMessageLen, i, &remainder);
            encodeUserMessage(userMessageStart, msg + i*MAX_CONCAT_SMS_LEN ,MAX_CONCAT_SMS_LEN , type);
            DEBUG("Total PDU size %ld\n", strlen(buffer));
            assert(strlen(buffer) <= MAX_TOTAL_SMS_LEN);
            printf("AT+CMGS=%ld\n%s%c\n", (strlen(buffer)-2)/2, buffer, 0x1A);
            i++;
        }while(remainder);
    } else{
        DEBUG("Using regular sms messages; current buffer: %ld\n%s \n",strlen(buffer), buffer);
        writeByte(strlen(msg), &ptr);
        encodeUserMessage(ptr, msg, realMessageLen, type);
        DEBUG("Total PDU size %ld\n", strlen(buffer));
        printf("AT+CMGS=%ld\n%s%c\n", (strlen(buffer)-2)/2, buffer, 0x1A);
    }
}
void usage(const char* programName) {
    fprintf(stderr, "Usage: %s [-78] number or %s [-f field_sep] [-z]  \n", programName, programName);
}

int main(int argc, char * argv[]) {
    int alphabet = GSM_7_BIT;
    int decode = 0;
    int opt;
    while((opt=getopt(argc, argv, "f:zhd78"))!=-1) {
        switch(opt) {
            case 'f':
                FIELD_SEPARATOR = optarg[0];
               break;
            case 'z':
                LN_TERMINATOR = 0;
                break;
            case 'd':
                decode = 1;
               break;
            case '7':
               alphabet = GSM_7_BIT;
               break;
            case '8':
               alphabet = GSM_8_BIT;
               break;
            case 'h':
                usage(argv[0]);
                break;
            default:
                usage(argv[0]);
                exit(1);
                break;
        }
    }
    char buffer[4096]={0};
    int ret = read(STDIN_FILENO, buffer, sizeof(buffer)-1);
    if(ret ==-1) {
        perror("failed to read");
    }

    if(decode)
        decodeSMSMessage(buffer);
    else {
        const char* number = argv[optind];
        if(!number) {
            usage(argv[0]);
            exit(1);
        }
        encodeSMSMessage(number, buffer, alphabet);
    }
    exit(0);
}
