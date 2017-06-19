# ArduinoTW39
A telex Interface for arduino with ethernetshield.

The folder "etelex" contains/is the Arduino-Sketch.

Inside it, the file etelex.h contains most of the options, you may want tu configure.

More Information can be found at https://www.telexforum.de/viewtopic.php?f=29&t=589

Hardware
========
Depending on the Hardware, you may need to connect an additional snubber in paralell to the output.

SD-CARD
=======

If you want to use SD-Card Suport, the SD-Card should contain a folder pb.
Inside this folder you can place a text file for each dialable numer that contains the same informations as the tnl-server response would: (You can place arbitrary comments behind # signs)


*0.txt*
```
#ok
#781272 <-- sicherheitshalber nochmal die gefundene Nummer
#Fred, Braunschweig T100 <-- Name (rein informativ)
#1 <-- Typ (siehe unten)
#sonnibs.no-ip.org <-- Adresse, hier kann auch eine IP in der Form 95.90.186.198 stehen
#134 <-- Port-Nummer
#33 <-- Durchwahl
#+++ <-- Ende-Kennzeichnung
#
#Zum Typ:
#1 ist eine echte i-Telex-Station mit einem Hostnamen
#2 oder 5 ist eine echte i-Telex-Station mit einer IP-Adresse
#3 ist eine Station, die nur Ascii-Daten verarbeiten kann, so wie Frank's Wetterdienst-Server (mit Hostnamen)
#4 ist eine Station, die nur Ascii-Daten verarbeiten kann, mit IP-Adresse
#6 Email-Adresse (für dich nicht Sinnvoll und auch kaum verwendet)
ok
0
Lokaler Test# mit Kommentar
4
192.168.1.16
134
0
+++
```
