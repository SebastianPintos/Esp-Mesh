/*
  Copyright 2017 Andreas Spiess

  Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"),
  to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense,
  and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE
  FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
  WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

  This software is based on the work of Ray Burnette: https://www.hackster.io/rayburne/esp8266-mini-sniff-f6b93a
*/

#include <ESP8266WiFi.h>
#include <ArduinoJson.h>
// #include <credentials.h>
#include <set>
#include <string>
#include "./functions.h"

#define disable 0
#define enable  1
#define SENDTIME 30000
#define MAXDEVICES 40
#define MINRSSI -100
#define JBUFFER 15+ (MAXDEVICES * 40)


// uint8_t channel = 1;
unsigned int channel = 1;
int clients_known_count_old, aps_known_count_old;
unsigned long sendEntry, deleteEntry;


String device[MAXDEVICES];
int nbrDevices = 0;
int usedChannels[15];
boolean sendMQTT = false;


void setup() {
  Serial.begin(9600);
  Serial.printf("\n\nSDK version:%s\n\r", system_get_sdk_version());
  Serial.println(F("Human detector by Andreas Spiess. ESP8266 mini-sniff by Ray Burnette http://www.hackster.io/rayburne/projects"));
  Serial.println(F("Based on the work of Ray Burnette http://www.hackster.io/rayburne/projects"));

  wifi_set_opmode(STATION_MODE);            // Promiscuous works only with station mode
  wifi_set_channel(channel);
  wifi_promiscuous_enable(disable);
  wifi_set_promiscuous_rx_cb(promisc_cb);   // Set up promiscuous callback
  wifi_promiscuous_enable(enable);
  channel = 1;
  
  wifi_set_channel(channel);
}




void loop() {

  nothing_new++;                          // Array is not finite, check bounds and adjust if required
  if (nothing_new > 200) {                // monitor channel for 200 ms
    nothing_new = 0;
    channel++;
    if (channel == 15){ 
      channel = 1;
      wifi_set_channel(channel);
    }
  }
  delay(1);  // critical processing timeslice for NONOS SDK! No delay(0) yield()
  
  if ((millis() - sendEntry > SENDTIME) || exceededMaxAPs() || exceededMaxClients()) {
    sendEntry = millis();
    sendMQTT = true;
  }
  if (sendMQTT) {
    showDevices();
    sendDevices();
    cleanAll();
    
  }
  
}



void cleanAll() {
  memset(aps_known, 0, sizeof(aps_known));
  memset(clients_known, 0, sizeof(clients_known));
  clients_known_count = 0;
  aps_known_count = 0;
}


void showDevices() {
  Serial.println("");
  Serial.println("");
  Serial.println("-------------------Device DB-------------------");
  Serial.printf("%4d Devices + Clients.\n", aps_known_count + clients_known_count); // show count

  // show Beacons
  for (int u = 0; u < aps_known_count; u++) {
    Serial.printf( "%4d ", u); // Show beacon number
    Serial.print("B ");
    Serial.print(formatMac1(aps_known[u].bssid));
    Serial.print(" RSSI ");
    Serial.print(aps_known[u].rssi);
    Serial.print(" channel ");
    Serial.print(aps_known[u].channel);
    Serial.println();

  }

  // show Clients
  for (int u = 0; u < clients_known_count; u++) {
    Serial.printf("%4d ", u); // Show client number
    Serial.print("C ");
    Serial.print(formatMac1(clients_known[u].station));
    Serial.print(" RSSI ");
    Serial.print(clients_known[u].rssi);
    Serial.print(" channel ");
    Serial.print(clients_known[u].channel);
    Serial.println();
  }
}

void sendDevices() {
  String deviceMac;


  StaticJsonBuffer<JBUFFER>  jsonBuffer;
  
  JsonObject& root = jsonBuffer.createObject();
  JsonArray& mac = root.createNestedArray("MAC");
  JsonArray& rssi = root.createNestedArray("RSSI");
  JsonArray& milis = root.createNestedArray("MILIS");
  JsonArray& ch = root.createNestedArray("CH");
  JsonArray& type = root.createNestedArray("TYPE");
  JsonArray& ssidOrStationMac = root.createNestedArray("STATION/SSID");
  // add Beacons
  for (int u = 0; u < aps_known_count; u++) {
    deviceMac = formatMac1(aps_known[u].bssid);
    if (aps_known[u].rssi > MINRSSI) {
      mac.add(deviceMac);
      rssi.add(aps_known[u].rssi);
      milis.add(aps_known[u].lastDiscoveredTime);
      ch.add(aps_known[u].channel);
      type.add("B");
      ssidOrStationMac.add(aps_known[u].ssid);

    }
  }

  // Add Clients
  for (int u = 0; u < clients_known_count; u++) {
    deviceMac = formatMac1(clients_known[u].station);
    if (clients_known[u].rssi > MINRSSI) {
      mac.add(deviceMac);
      rssi.add(clients_known[u].rssi);
      milis.add(clients_known[u].lastDiscoveredTime);
      ch.add(clients_known[u].channel);
      if (clients_known[u].channel == -2) {
        type.add("R");
      }
      else {
        type.add("C");
      }
      ssidOrStationMac.add(formatMac1(clients_known[u].ap));


    }
  }

  Serial.println();
  Serial.printf("Devices Above RSSI Threshold: %02d\n", mac.size());
  root.prettyPrintTo(Serial);
  sendEntry = millis();
  jsonBuffer.clear();
}
