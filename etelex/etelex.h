#ifndef __ETELEX_H__
#define __ETELEX_H__

#define MYPORT 134
#define MYIPFALLBACK 192, 168, 1, 64
#define MYMACADRESS 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED
#define TLN_SERVER "sonnibs.no-ip.org"

#define MYSSID "Wondernet2"
#define MYWIFIPASSWORD "xxxxxxxxxx"
#define MYHOSTNAME "etelex"


#define _DEBUG
#undef _DEBUGTIMINGS
#define ITELEX
#define USEIRQ


#ifdef ESP8266
#define ESP
#endif


#ifdef ESP
#include <ESP8266WiFi.h>

//const char* ssid = MYSSID;
//const char* password = MYWIFIPASSWORD;


#else // NOT ESP
#include <Ethernet.h>
#define USE_SDCARD
#endif


#ifdef USE_SDCARD
#include <SD.h>
#include <SPI.h>
#else
/** Store and print a string in flash memory.*/
#define PgmPrint(x) SerialPrint_P(PSTR(x))
/** Store and print a string in flash memory followed by a CR/LF.*/
#define PgmPrintln(x) SerialPrintln_P(PSTR(x))

extern  void SerialPrint_P(PGM_P str);
extern  void SerialPrintln_P(PGM_P str);
#endif



#endif

