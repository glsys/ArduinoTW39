#define _DEBUG
#define MYPORT 134
#define MYIPFALLBACK 192, 168, 1, 64
#define MYMACADRESS 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED
#define TLN_SERVER "sonnibs.no-ip.org"
#include <Arduino.h>


#include <SD.h>
#include <SPI.h>
#include <Ethernet.h>

#include "baudot.h"
#include "lookup.h"



// Enter a MAC address for your controller below.
// Newer Ethernet shields have a MAC address printed on a sticker on the shield
byte mac[] = { MYMACADRESS };

//Set the static IP address to use if the DHCP fails to assign
IPAddress ip(MYIPFALLBACK);

// Initialize the Ethernet client library
// with the IP address and port of the server

EthernetServer server(MYPORT);
EthernetClient client;

char tlnserver[] = TLN_SERVER;


const byte COMMUTATE_PIN = 2; // umpol pin for the relais
const byte RECIEVE_PIN = 3;     // recieve Pin when online
const byte SEND_PIN = 5; // send pin for the Mosfet

const int DATABIT_DURATION = 20; //milliseconds
const int STARTBIT_DURATION = 20; // milliseconds
const int STOPBIT_DURATION = 40; // milliseconds
const int SAMPLEPOS=60; // 60 von 120

byte state;


#define STATE_RUHEZUSTAND  0 //4ma  ump=0, ssr=0
#define STATE_ANFANGSSIGNAL  10 //4ma -> 40 ma -> opto==1 ca. 1000ms delay
#define STATE_WAHLAUFFORDERUNGSSIGNAL 20 //brk=1, 25ms delay
#define STATE_WARTE_AUF_ZIFFER  30  //brk=0, /warte  5000ms auf opto==0 -> 31 sonst -> 34 auf opto==0
#define STATE_WAHLIMPULS  31  //opto==0, warte auf opto==1 -> 32
#define STATE_WARTE_AUF_NAECHSTEN_WAHLIMPULS  32  //opto==1, warte auf 200ms auf opto==0 -> 31 sonst -> 33
#define STATE_WAHL_ENDE  34  // nummer zu kurz -> 90 sonst 40
#define STATE_LOOKUP  40  //nummer nicht gefunden -> 90 sonst 50

#define STATE_CONNECTING  50  // verbindungs timeout -> 90 sonst 100
#define STATE_BESETZT   90  //ump=1, SSR=1, delay 2000 -> 0
#define STATE_ONLINE   100  //ump=1, SSR=1, disconnect -> 0, opto==0 <2000s -> 110 sonst -> 0, recieve data via tcp ->120
#define STATE_RECIEVING   110
#define STATE_SENDING   120

#define STATE_DISCONNECT 200
byte currentDigit;
byte currentNumberPos;
#define NUMBER_BUFFER_LENGTH 30
char number[NUMBER_BUFFER_LENGTH];

byte recieve_buf; // recieve buffer
byte recieved_char; //

boolean recieving=false;

boolean debugTimings=false;
boolean new_char_recieved=false;



//unsigned long pin_val;
unsigned int bit_pos=0; // aktuelles bit in rcve, startbit=1

unsigned long last_timestamp;
unsigned long st_timestamp;

 void onlinepinchange(){
    if(recieving && bit_pos==0){
      if( digitalRead(RECIEVE_PIN)){ //startbit
        last_timestamp=millis();
        bit_pos++;

      }
    }
 }

void setup() {
  pinMode(RECIEVE_PIN, INPUT_PULLUP);
  pinMode(SEND_PIN, OUTPUT);
  pinMode(COMMUTATE_PIN, OUTPUT);

  digitalWrite(SEND_PIN, LOW);
  digitalWrite(COMMUTATE_PIN, LOW);


  Serial.begin(115200);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }

  pinMode(10, OUTPUT);                       // set the SS pin as an output (necessary!)
  digitalWrite(10, HIGH);                    // but turn off the W5100 chip!


  if (!SD.begin(4)) {
      PgmPrintln("SD initialization failed!");
  }else{
    PgmPrintln("Success SD init.");
  }



  PgmPrintln("...starting Network");


   if (Ethernet.begin(mac) == 0) {
    PgmPrintln("Failed to configure Ethernet using DHCP");
    // try to congifure using IP address instead of DHCP:
    Ethernet.begin(mac, ip);
  }
  // give the Ethernet shield a second to initialize:
  delay(1000);
  PgmPrint("My IP address: ");
  for (byte thisByte = 0; thisByte < 4; thisByte++) {
    // print the value of each byte of the IP address:
    Serial.print(Ethernet.localIP()[thisByte], DEC);
    PgmPrint(".");
  }
Serial.println();







  PgmPrintln("Welcome to eTelex!");
  PgmPrint("Free RAM: ");
  Serial.println(FreeRam()); 

  state=STATE_RUHEZUSTAND;

  bit_pos=0;
  last_timestamp=millis();
  st_timestamp=millis();
  new_char_recieved=false;

  currentDigit=0;
  currentNumberPos=0;
  attachInterrupt(digitalPinToInterrupt(RECIEVE_PIN), onlinepinchange, CHANGE);

}



void loop() {
  switch (state){

    case STATE_RUHEZUSTAND:
      digitalWrite(SEND_PIN, LOW);
      digitalWrite(COMMUTATE_PIN, LOW);
      if(digitalRead(RECIEVE_PIN)==0){ // AT
        delay(5);
        if(digitalRead(RECIEVE_PIN)==0){// immernoch AT
          delay(5);
          if(digitalRead(RECIEVE_PIN)==0){// immernoch AT
            last_timestamp=millis();
            state=STATE_ANFANGSSIGNAL;
#ifdef _DEBUG            
           PgmPrintln("STATE_ANFANGSSIGNAL");
#endif           
          }
        }
      }
      client = server.available();
      if (client) {
#ifdef _DEBUG
        PgmPrintln("server.available");
#endif        
        state=STATE_ONLINE;
        digitalWrite(COMMUTATE_PIN, HIGH);
        delay(1000);
      }
    break;

    case STATE_ANFANGSSIGNAL:
      digitalWrite(SEND_PIN, LOW);
      digitalWrite(COMMUTATE_PIN, LOW);
      if(millis()>last_timestamp+1000){
           state=STATE_WAHLAUFFORDERUNGSSIGNAL;
           last_timestamp=millis();
#ifdef _DEBUG
           PgmPrintln("STATE_WAHLAUFFORDERUNGSSIGNAL");
#endif
      }
    break;

    case STATE_WAHLAUFFORDERUNGSSIGNAL:
      digitalWrite(SEND_PIN, HIGH);
      digitalWrite(COMMUTATE_PIN, LOW);
      currentNumberPos=0;
      if(millis()>last_timestamp+25){
            digitalWrite(SEND_PIN, LOW);
            delay(10);
            last_timestamp=millis();
            state=STATE_WARTE_AUF_ZIFFER;
#ifdef _DEBUG
            PgmPrintln("STATE_WARTE_AUF_ZIFFER: 0");
#endif
      }
    break;

    case STATE_WARTE_AUF_ZIFFER:
      currentDigit=0;
      digitalWrite(SEND_PIN, LOW);
      digitalWrite(COMMUTATE_PIN, LOW);
      if(digitalRead(RECIEVE_PIN)==1){// wahlimpuls
            last_timestamp=millis();
            state=STATE_WAHLIMPULS;
      }
      //todo check number
      if(currentNumberPos>0 && millis()>last_timestamp+5000){
           last_timestamp=millis();
           state=STATE_WAHL_ENDE;
#ifdef _DEBUG
           PgmPrintln("STATE_WAHL_ENDE");
#endif
      }

    break;

    case STATE_WAHLIMPULS:
      digitalWrite(SEND_PIN, LOW);
      digitalWrite(COMMUTATE_PIN, LOW);
      if(digitalRead(RECIEVE_PIN)==0){// wahlimpuls_ende
            currentDigit++;
            last_timestamp=millis();
            state=STATE_WARTE_AUF_NAECHSTEN_WAHLIMPULS;
//            PgmPrintln("STATE_WARTE_AUF_NAECHSTEN_WAHLIMPULS");
      }
      if(millis()>last_timestamp+30000){
            last_timestamp=millis();
            state=STATE_RUHEZUSTAND;
#ifdef _DEBUG
           PgmPrintln("Dial Start Timeout");
           PgmPrintln("STATE_RUHEZUSTAND");
#endif
      }

    break;

    case STATE_WARTE_AUF_NAECHSTEN_WAHLIMPULS:
      digitalWrite(SEND_PIN, LOW);
      digitalWrite(COMMUTATE_PIN, LOW);
      if(digitalRead(RECIEVE_PIN)==1){// wahlimpuls
            last_timestamp=millis();
            state=STATE_WAHLIMPULS;
      }
      if(millis()>last_timestamp+200){
            last_timestamp=millis();
            state=STATE_WARTE_AUF_ZIFFER;
            if(currentDigit==0){
#ifdef _DEBUG
              PgmPrintln("kein impuls");
              PgmPrintln("STATE_RUHEZUSTAND");
#endif
              state=STATE_RUHEZUSTAND;
            }
            if(currentDigit==10){
              currentDigit=0;
            }
           number[currentNumberPos++]='0'+currentDigit;
           if(currentNumberPos==NUMBER_BUFFER_LENGTH){
#ifdef _DEBUG
             PgmPrintln("number to long!");
#endif
             currentNumberPos=NUMBER_BUFFER_LENGTH-1;
           }
#ifdef _DEBUG
           PgmPrint("ZIFFER:");
           Serial.println(currentDigit);
           PgmPrint("STATE_WARTE_AUF_ZIFFER:");
           Serial.println(currentNumberPos);
#endif
      }

    break;


   case STATE_WAHL_ENDE:
      digitalWrite(SEND_PIN, LOW);
      digitalWrite(COMMUTATE_PIN, LOW);
      number[currentNumberPos]=0;
#ifdef _DEBUG
     PgmPrint("gewählte Nummer war: ");
     Serial.println(number);
#endif
      state=STATE_LOOKUP;

      recieving=true;
    break;


case STATE_LOOKUP:
  if(lookupNumber(number)){//970012 //729512
#ifdef _DEBUG
    PgmPrint("lookup ok: ");
    Serial.print(lookup_host);
    PgmPrint(":");
    Serial.print(lookup_port);
    PgmPrint("*");
    Serial.print(lookup_durchwahl);
    PgmPrintln("*");
    PgmPrintln("STATE_CONNECTING");
#endif
  state=STATE_CONNECTING;
  }else{
#ifdef _DEBUG
    PgmPrintln("kein impuls");
    PgmPrintln("STATE_RUHEZUSTAND");
#endif
    state=STATE_RUHEZUSTAND;
  }


    break;

    case STATE_CONNECTING:
#ifdef _DEBUG
  PgmPrintln("connecting to remote");
#endif
  if (client.connect(lookup_host.c_str(), lookup_port)) {
#ifdef _DEBUG
    PgmPrintln("connected to remote");
#endif
    if(lookup_durchwahl>0){
#ifdef _DEBUG
      Serial.print('*');
      Serial.print(lookup_durchwahl);
      Serial.print('*');
      Serial.println();
#endif
      client.print('*');
      client.print(lookup_durchwahl);
      client.print('*');
    }
    state=STATE_ONLINE;
    recieving=true;
    digitalWrite(COMMUTATE_PIN, HIGH);
    delay(1000);
  }else{
#ifdef _DEBUG
    PgmPrintln("connecting to remote failed");
#endif
    state=STATE_DISCONNECT;
  }
    
    break;
    case STATE_ONLINE:
      digitalWrite(COMMUTATE_PIN, HIGH);
      if(!client.connected()){ // hat der client getrennt ?
        client.stop();
#ifdef _DEBUG
        PgmPrintln("Client disconnected!");
#endif
        state=STATE_DISCONNECT;
      }
      if(!digitalRead(RECIEVE_PIN)){ // wollen wir trennen
        st_timestamp=millis();
      }else if(millis()>st_timestamp+3000){
#ifdef _DEBUG
        PgmPrintln("We want to disconnect!");
#endif
        state=STATE_DISCONNECT;
      }
    break;
    case STATE_DISCONNECT:
#ifdef _DEBUG
      PgmPrintln("Disconnecting!");
      PgmPrintln("STATE_RUHEZUSTAND");
#endif
      client.stop();
      state=STATE_RUHEZUSTAND;
      digitalWrite(SEND_PIN, LOW);
      digitalWrite(COMMUTATE_PIN, LOW);
      delay(1000);
    break;

/*
#define STATE_WAHLIMPULS  31;//opto==0, warte auf opto==1 -> 32
#define STATE_WARTE_AUF_NAECHSTEN_WAHLIMPULS  32;//opto==1, warte auf 200ms auf opto==0 -> 31 sonst -> 33
#define STATE_WAHL_ENDE  34;// nummer zu kurz -> 90 sonst 40
#define STATE_LOOKUP  40;//nummer nicht gefunden -> 90 sonst 50
#define STATE_CONNECTING  50;// verbindungs timeout -> 90 sonst 100
#define STATE_BESETZT   90;//ump=1, SSR=1, delay 2000 -> 0
#define STATE_ONLINE   100;//ump=1, SSR=1, disconnect -> 0, opto==0 <2000s -> 110 sonst -> 0, recieve data via tcp ->120
#define STATE_RECIEVING   110;
#define STATE_SENDING   120;
*/

  }





    if(bit_pos==0){
      //wait for startbit (done in pinchangeinterrupt)
    }else{
      unsigned long timestamp=millis();
      long diff=timestamp-last_timestamp;
      if((bit_pos>0) && (bit_pos<6) && (diff>= (STARTBIT_DURATION+(DATABIT_DURATION*(bit_pos-1))+(DATABIT_DURATION*SAMPLEPOS/120)))){

#ifdef _DEBUGTIMINGS
    Serial.print(bit_pos,DEC);
    PgmPrint("\t");
    Serial.print(last_timestamp,DEC);
    PgmPrint("\t");
    Serial.print(timestamp,DEC);
    PgmPrint("\t");
    Serial.print(diff,DEC);
    PgmPrint("\t");
    Serial.print((STARTBIT_DURATION+(DATABIT_DURATION*(bit_pos-1))+(DATABIT_DURATION*SAMPLEPOS/120)));
    PgmPrint("\n");
#endif

      recieve_buf=recieve_buf<<1;
      bit_pos++;
       if(digitalRead(RECIEVE_PIN)){
       }else{
        recieve_buf=recieve_buf+1;
      }
      if(bit_pos==6){
        new_char_recieved=true;
        recieved_char=recieve_buf;
        recieve_buf=0;
        bit_pos=0;
      }
    }
    }



  if (new_char_recieved) {
    char c=baudotToAscii(recieved_char);
    Serial.print(c);

    if(client.connected()){
#ifdef itelex
  if(c!=BAUDOT_CHAR_UNKNOWN && c!=BAUDOT_MODE_BU && c!=BAUDOT_MODE_ZI && c!=0){
    client.print(c);
  }
#else
  client.print(c);
#endif
    }
    new_char_recieved=false;
  }


   if (Serial.available() > 0) {
    if(state==STATE_ONLINE){
        sendAsciiAsBaudot(Serial.read());
    }else{
      state=STATE_ONLINE;
      recieving=true;
    }
   }

  if (client.available()) {
        char c = client.read();
        sendAsciiAsBaudot(c);
        Serial.print(c);
  }
  Ethernet.maintain();
}

void sendAsciiAsBaudot(char ascii) {
  byte modeswtichcode;
  byte baudot = asciiToBaudot(ascii,&modeswtichcode);

  if(baudot!=0){
    if(modeswtichcode!=BAUDOT_MODE_UNDEFINED){
      byte_send(modeswtichcode);
#ifdef _DEBUG_2
      PgmPrint("Switching mode to ");
      Serial.print(modeswtichcode);
      PgmPrint("\n");
#endif      
    }
    byte_send(baudot);
  }
} // end char_send()

void byte_send(byte _byte) {
  recieving=false;
  state=STATE_SENDING;


  digitalWrite(SEND_PIN, HIGH);
  delay(STARTBIT_DURATION);


  for (int _bit = 4; _bit >= 0; _bit--) {
    digitalWrite(SEND_PIN, !bitRead(_byte, _bit));
    delay(DATABIT_DURATION);
  } // end for loop

  digitalWrite(SEND_PIN, LOW);
  delay(STOPBIT_DURATION);

  recieving=true;
  state=STATE_ONLINE;
} // end byte_send()
