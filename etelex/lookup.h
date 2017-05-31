#ifndef __LOOKUP_H__
#define __LOOKUP_H__
#include <Arduino.h>

#include <Ethernet.h>

extern String lookup_host;
extern int lookup_port;
extern byte lookup_durchwahl;

extern bool lookupNumber(String number);

#endif //ndef __LOOKUP_H__
