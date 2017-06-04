#ifndef __ETELEX_H__
#define __ETELEX_H__

#define MYPORT 134
#define MYIPFALLBACK 192, 168, 1, 64
#define MYMACADRESS 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED
#define TLN_SERVER "sonnibs.no-ip.org"

#define USE_SDCARD
#define _DEBUG
#define ITELEX



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

