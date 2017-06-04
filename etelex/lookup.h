#ifndef __LOOKUP_H__
#define __LOOKUP_H__
#include <Arduino.h>



extern String lookup_host;
extern int lookup_port;
extern byte lookup_durchwahl;
extern char tlnserver[];
extern bool lookupNumber(String number);

#endif //ndef __LOOKUP_H__
