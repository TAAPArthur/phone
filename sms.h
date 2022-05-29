#ifndef SMS_H
#define SMS_H

#ifdef DEBUG
#define TRACE(X...) dprintf(2, X)
#define TRACE(X...) dprintf(2, X)
#else
#define TRACE(X...)
#define TRACE(X...)
#endif

#define INTERNATIONAL_FORMAT 0x91
#define MAX_SMS_LEN 255 // 140
#define MAX_TOTAL_SMS_LEN (255*2 +2)
#define MAX_CONCAT_SMS_LEN 134
//#define UDH_CONCAT_SMS_HEADER_LEN (MAX_SMS_LEN -MAX_CONCAT_SMS_LEN )
#define UDH_CONCAT_SMS_16_HEADER_LEN 7
#define UDH_CONCAT_SMS_08_HEADER_LEN 6
#define CONCAT_SMS_08_BIT_REF_NUMBER 0x00
#define CONCAT_SMS_16_BIT_REF_NUMBER 0x08


#define MAX_PHONE_NUMBER_SIZE 128

typedef enum {
    GSM_7_BIT= 0x00,
    GSM_8_BIT= 0x04,
    GSM_UCS2 =0x08,
} GSMEncodingType;
typedef enum {
    SMS_DELIVER =0b00,
    SMS_SUBMIT_REPORT  =0b01,
    SMS_STATUS_REPORT=0b10,
} SMS_DELIVER_TYPE;

typedef enum {
    SMS_DELIVER_REPORT =0b00,
    SMS_SUBMIT  =0b01,
    SMS_COMMAND =0b10,
} SMS_RECEIVE_TYPE;
typedef struct {
    char len;
    char type;
    char number[10];
}PhoneNumber ;

typedef char bool ;
#endif
