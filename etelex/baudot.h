#ifndef __BAUDOT_H__
#define __BAUDOT_H__
#include <Arduino.h>

#define itelex

#define BAUDOT_MODE_UNDEFINED 0
#define BAUDOT_MODE_BU 31
#define BAUDOT_MODE_ZI 27


#ifdef itelex
 const char baudot_bu[] = "#t\ro hnm\nlrgipcvezdbsyfxawj\017uqk\016";
 const char baudot_zi[] = "#5\r9 #,.\n)4#80:=3+@?'6#/-2%\01771(\016";
//   const char baudot_zi[] = "#5\r9 #,.\n)4#80:=3+#?'6#/-2%\01771(\016";
 #define BAUDOT_CHAR_UNKNOWN 0
#else
 const char baudot_bu[] = "\000t\ro hnm\nlrgipcvezdbsyfxawj\017uqk\016";
 const char baudot_zi[] = "\0005\r9 #,.\n)4#80:=3+\005?'6#/-2\007\01771(\016";
 #define BAUDOT_CHAR_UNKNOWN 26
#endif


extern char baudotToAscii(byte baudot);
extern byte asciiToBaudot(char ascii,byte *modeswtichcode);

#endif //ndef __BAUDOT_H__
