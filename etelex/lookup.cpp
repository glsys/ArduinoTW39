#include "lookup.h"
#include "etelex.h"


//#define _DEBUG

char sd_c;
#ifdef ESP
WiFiClient tln_client;
#else
EthernetClient tln_client;
#endif

String lookup_host;
int lookup_port;
byte lookup_durchwahl;
//char linebuffer[40];
String linebuffer;

#ifdef USE_SDCARD
bool lookupSD(String number){
#ifdef _DEBUG
  PgmPrintln("lookupTlnSrv");
#endif
  pinMode(10, OUTPUT);                       // set the SS pin as an output (necessary!)
  digitalWrite(10, HIGH);                    // but turn off the W5100 chip!
String filename="pb/"+number+".txt";
File  myFile = SD.open(filename);
  if (myFile) {
    int pos=0; 
    linebuffer="";
     // read from the file until there's nothing else in it:
     while (myFile.available()) {
        sd_c=myFile.read();
        if(sd_c=='#'){
          while (myFile.available()) {
            sd_c=myFile.read();
            if(sd_c=='\n'){
              break;
            }
          }
        }
        if(sd_c!='\r' && sd_c!='\n'){
          linebuffer+=sd_c;
          //Serial.write(sd_c);          
        } 
        if(sd_c=='\n'){
          linebuffer.trim();
          if(linebuffer.length()>0){
            if(pos==0){
#ifdef _DEBUG
              PgmPrint("Status: ");
              Serial.println(linebuffer);
#endif

              if(linebuffer!="ok"){
#ifdef _DEBUG
                PgmPrintln("Status not ok");
#endif
                myFile.close(); 
                return false;
              }
            }
            if(pos==1){
#ifdef _DEBUG
              PgmPrint("Nummer: ");
              Serial.println(linebuffer);
#endif              
              if(linebuffer!=number){
#ifdef _DEBUG
                PgmPrintln("number not the same");
#endif
                myFile.close(); 
                return false;
              }

            }
            if(pos==2){
#ifdef _DEBUG
              PgmPrint("Name: ");
              Serial.println(linebuffer);
#endif
            }
            if(pos==3){
#ifdef _DEBUG
              PgmPrint("Typ: ");
              Serial.println(linebuffer);
#endif
            }
            if(pos==4){
#ifdef _DEBUG              
              PgmPrint("Host: ");
              Serial.println(linebuffer);
#endif
              lookup_host=linebuffer;
            }
            if(pos==5){
#ifdef _DEBUG
              PgmPrint("Port: ");
              Serial.println(linebuffer);
#endif
              lookup_port=linebuffer.toInt();
            }
            if(pos==6){
#ifdef _DEBUG              
              PgmPrint("Durchwahl: ");
              Serial.println(linebuffer);
#endif
              lookup_durchwahl=linebuffer.toInt();
              myFile.close(); 
              return true;
              
            }
            
            if(linebuffer=="+++"){
              pos=0;
            }else{
              pos++;                          
            }
            linebuffer="";
          }
        }
     }
     // close the file:
    myFile.close();
   } else {
     // if the file didn't open, print an error:
#ifdef _DEBUG
      PgmPrintln("error opening file");
#endif      
   }

  return false;
}
#endif



/*
#Zum Typ:
#1 ist eine echte i-Telex-Station mit einem Hostnamen
#2 oder 5 ist eine echte i-Telex-Station mit einer IP-Adresse
#3 ist eine Station, die nur Ascii-Daten verarbeiten kann, so wie Frank's Wetterdienst-Server (mit Hostnamen)
#4 ist eine Station, die nur Ascii-Daten verarbeiten kann, mit IP-Adresse
#6 Email-Adresse (für dich nicht Sinnvoll und auch kaum verwendet)
*/

bool lookupTlnSrv(String number){
PgmPrintln("lookupTlnSrv");
    String IP="";
    if (tln_client.connect(tlnserver, 11811)) {
#ifdef _DEBUG
      PgmPrintln("connected to tln Server");
#endif
      String query="q";
      query+=number;
      tln_client.println(query);
      while(!tln_client.available()){;}
      


    int pos=0; 
    linebuffer="";
     while(tln_client.available()){
        sd_c=tln_client.read();
        if(sd_c!='\r' && sd_c!='\n'){
          linebuffer+=sd_c;    
        } 
        if(sd_c=='\n'){
          linebuffer.trim();
          if(linebuffer.length()>0){
            if(pos==0){
#ifdef _DEBUG
              PgmPrint("Status: ");
              Serial.println(linebuffer);
#endif
              if(linebuffer!="ok"){
#ifdef _DEBUG
                PgmPrintln("Status not ok");
#endif
                tln_client.stop();
                return false;
              }
            }
            if(pos==1){
#ifdef _DEBUG
              PgmPrint("Nummer: ");
              Serial.println(linebuffer);
#endif
              if(linebuffer!=number){
#ifdef _DEBUG
                PgmPrintln("number not the same");
#endif
                tln_client.stop();
                return false;
              }

            }
            if(pos==2){
#ifdef _DEBUG
              PgmPrint("Name: ");
              Serial.println(linebuffer);
#endif
            }
            if(pos==3){
#ifdef _DEBUG
              PgmPrint("Typ: ");
              Serial.println(linebuffer);
#endif
            }
            if(pos==4){
#ifdef _DEBUG
              PgmPrint("Host: ");
              Serial.println(linebuffer);
#endif
              lookup_host=linebuffer;
            }
            if(pos==5){
#ifdef _DEBUG
              PgmPrint("Port: ");
              Serial.println(linebuffer);
#endif
              lookup_port=linebuffer.toInt();
            }
            if(pos==6){
#ifdef _DEBUG
              PgmPrint("Durchwahl: ");
              Serial.println(linebuffer);
#endif
              lookup_durchwahl=linebuffer.toInt();
              tln_client.stop();
              return true;
              
            }
            
            if(linebuffer=="+++"){
              pos=0;
            }else{
              pos++;                          
            }
            linebuffer="";
          }
        }
     }       
     tln_client.stop();
    }
#ifdef _DEBUG
PgmPrintln("");
#endif
  return(false);
}
bool lookupHardcoded(String number){
  PgmPrintln("lookupHardcoded");
  if(number=="0"){
    lookup_host="192.168.1.16";
    lookup_port=134;
    lookup_durchwahl=0;
    return true;
  }
  if(number=="1"){
    lookup_host="www.glsys.de";
    lookup_port=134;
    lookup_durchwahl=0;
    return true;
  }
  return false;
}

bool lookupNumber(String number){
#ifdef _DEBUG
  Serial.println();
  PgmPrint("lookupNumber: ");
  Serial.println(number);
#endif
#ifdef USE_SDCARD  
  if(lookupSD(number)) return true;
#else
  if(lookupHardcoded(number)) return true;
#endif
  return lookupTlnSrv(number);
}






#ifndef USE_SDCARD
 void SerialPrint_P(PGM_P str) {
  for (uint8_t c; (c = pgm_read_byte(str)); str++) Serial.write(c);
}
 void SerialPrintln_P(PGM_P str) {
  SerialPrint_P(str);
  Serial.println();
}
#endif  //USE_SDCARD
