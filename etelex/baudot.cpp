#include "baudot.h"
#include "etelex.h"

byte baudot_recieving_mode=BAUDOT_MODE_UNDEFINED;
byte baudot_sending_mode=BAUDOT_MODE_UNDEFINED;


char baudotToAscii(byte baudot){
    baudot_sending_mode=BAUDOT_MODE_UNDEFINED;

  if (baudot == BAUDOT_MODE_ZI){
    baudot_recieving_mode=BAUDOT_MODE_ZI;
//    Serial.print("ZI \n");
  }else if (baudot == BAUDOT_MODE_BU){
    baudot_recieving_mode=BAUDOT_MODE_BU;
//    Serial.print("BU \n");
  }

 if (baudot_recieving_mode==BAUDOT_MODE_BU){
      return baudot_bu[baudot];
 }
 if (baudot_recieving_mode==BAUDOT_MODE_ZI){
      return baudot_zi[baudot];
 }
 return BAUDOT_CHAR_UNKNOWN;
}
byte asciiToBaudot(char ascii_in,byte *modeswtichcode){
  baudot_recieving_mode=BAUDOT_MODE_UNDEFINED;
  *modeswtichcode=BAUDOT_MODE_UNDEFINED;
  char ascii=tolower(ascii_in);

  if(baudot_sending_mode==BAUDOT_MODE_ZI){
    for (byte i = 0 ; i < 32 ; i++){
      if (ascii == baudot_zi[i]){
        return i;
      }
    }
    for (byte i = 0 ; i < 32 ; i++){
      if (ascii == baudot_bu[i]){
        *modeswtichcode=BAUDOT_MODE_BU;
        baudot_sending_mode=BAUDOT_MODE_BU;
        return i;
      }
    }
  }//baudot_sending_mode==BAUDOT_MODE_ZI

  for (byte i = 0 ; i < 32 ; i++){
    if (ascii == baudot_bu[i]){
      if(baudot_sending_mode!=BAUDOT_MODE_BU){ // maybe still undefined
        *modeswtichcode=BAUDOT_MODE_BU;
        baudot_sending_mode=BAUDOT_MODE_BU;
      }
      return i;
    }
  }
  for (byte i = 0 ; i < 32 ; i++){
    if (ascii == baudot_zi[i]){
      *modeswtichcode=BAUDOT_MODE_ZI;
       baudot_sending_mode=BAUDOT_MODE_ZI;
      return i;
    }
  }
  return BAUDOT_CHAR_UNKNOWN;
}
