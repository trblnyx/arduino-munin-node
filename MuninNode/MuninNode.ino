/*
 Arduino Munin Node

 (CC) 2014, 'Aztec Eagle' Turbo <turbo@ier.unam.mx>
 (CC) 2014, Instituto de Energías Renovables, UNAM
  
 A simple munin node that shows the value of each analog inputs
 using an Arduino Ethernet shield, or an Arduino Ethernet board.

 (The Arduino Ethernet board with PoE it's a beauty!)

 Supported by UNAM DGAPA Proyecto PAPIME PE101814 
   Conjunto de Prácticas de Instrumentación y Control basadas en 
   Hardware y Software Libre para Energías Renovables
 */

#define DEBUG 1
#define FS(X) String(F(X))

#include <SPI.h>
#include <Ethernet.h>

// Change as your needs!
String nodename = "arduino";
byte mac[] = {0x90, 0xA2, 0xDA, 0xF0, 0x0F, 0x00};
byte ip[] = {192,168,209,111};
byte ns[] = {192,168,209,254};
byte gw[] = {192,168,209,254};
byte nm[] = {255,255,255,0};

// Initialize the Ethernet server library
// with the IP address and port you want to use
// (port 4949 is default for munin):
EthernetServer server(4949);

void setup() {
#if DEBUG
  // Open serial communications and wait for port to open:
  Serial.begin(9600);
  while (!Serial) ;// wait for serial port to connect. Needed for Leonardo only
  Serial.println("Initializing...");
#endif
  // start the Ethernet connection and the server:
  Ethernet.begin(mac,ip,ns,gw,nm);
  server.begin();
#if DEBUG
  Serial.print("Server is at ");
  Serial.println(Ethernet.localIP());
#endif
  // setup your stuff
  pinMode(13, OUTPUT);
}

void loop() {
  // listen for incoming clients
  EthernetClient client = server.available();
  if (client) {
    digitalWrite(13, HIGH); // turn the LED on
#if DEBUG
    Serial.println("New client");
#endif
    client.print(FS("# munin node at ") + nodename + '\n');

    while (client.connected()) {
      if (client.available()) {
#if DEBUG
        Serial.println("readString...");
#endif
        String command = client.readString();
#if DEBUG
        Serial.println(command);
#endif
        if (command.startsWith(FS("quit"))) break;
        if (command.startsWith(FS("version"))) {
          client.print(FS("munins node on ") + nodename + FS(" version: 1.0.0\n"));
          continue;
        }
        if (command.startsWith(FS("list"))) {
          client.print(FS("a0 a1 a2 a3 a4 a5\n"));
          continue;
        }
        if (command.startsWith(FS("config a"))) {
          char ch = command.charAt(8);
          if (ch>='0' && ch<='5') {
              client.print(FS("graph_title Analog Input A") + String(ch - '0') + '\n');
              // client.print(FS("graph_args -l 0 --upper-limit 1024\n"));
              // client.print(FS("graph_scale no\n"));
              // client.print(FS("graph_category arduino\n"));
              client.print(FS("sensor.label Digital value\n.\n"));
          } else client.print(FS("# Unknown service\n.\n"));
          continue;
        }
        if (command.startsWith(FS("fetch a"))) {
          char ch = command.charAt(7);
          if (ch>='0' && ch<='5') {
            // read the input on analog pin 0:
            int sensorValue = analogRead(A0 + ch - '0');
            // float voltage = 5.0 * sensorValue / 1024;
#if DEBUG
            Serial.print("ADC.value ");
            Serial.println(sensorValue);
#endif
            client.print(FS("sensor.value ")
                       + String(sensorValue) 
                       + FS("\n.\n"));
            continue;
          } else client.print(FS("# Unknown service\n.\n")); 
          continue;
        }
        // no command catched
        client.print(FS("# Unknown command. Try list, config, fetch, version or quit\n"));
      }
    }
    // give the web browser time to receive the data
    delay(1);
    // close the connection:
    client.stop();
#if DEBUG
    Serial.println("Client disconnected");
#endif
    digitalWrite(13, LOW); // turn the LED off
  }
}

