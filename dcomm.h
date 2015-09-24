/*
* File : dcomm.h
*/
#ifndef _DCOMM_H_
#define _DCOMM_H_
/* ASCII Const */
#define SOH 1
#define STX 2
#define ETX 3
/* Start of Header Character */ /* Start of Text Character */ /* End of Text Character */
#define ENQ 5
#define ACK 6
#define BEL 7
#define CR 13
#define LF 10
#define NAK 21
#define Endfile 26
#define ESC 27
/* XON/XOFF protocol */
/* Enquiry Character */
/* Acknowledgement */
/* Message Error Warning */ /* Carriage Return */
/* Line Feed */
/* Negative Acknowledgement */ /* End of file character */ /* ESC key */
#define XON (0x11)
#define XOFF (0x13)
/* Const */
#define BYTESIZE 256 /* The maximum value of a byte */ #define MAXLEN 1024 /* Maximum messages length */
typedef unsigned char Byte;
typedef struct QTYPE {
unsigned int count; unsigned int front; unsigned int rear; unsigned int maxsize; Byte *data;
} QTYPE;
typedef struct MESGB {
unsigned int soh; unsigned int stx; unsigned int etx; Byte checksum; Byte msgno;
Byte *data; } MESGB;
#endif