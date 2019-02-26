#include <Arduino.h>
#include <ESP8266WiFi.h>
#include "Pushdata_ESP8266_SSL.h"

Pushdata_ESP8266_SSL pd;

char ssid[] = "MyWiFiNetwork";
char passwd[] = "MyWiFiPassword";
char macAddress[12];

void setup() {
  Serial.begin(115200);
  Serial.println();
  // Using nerdy_gnu123@example.com account email
  pd.setEmail("nerdy_gnu123@example.com");
  pd.addWiFi(ssid, passwd);
  char outbuf[20];
  WiFi.macAddress((uint8_t*)macAddress);
  sprintf(outbuf, "%02x:%02x:%02x:%02x:%02x:%02x", macAddress[0], macAddress[1], macAddress[2], macAddress[3], macAddress[4], macAddress[5]);
  Serial.print("Ethernet MAC address = "); Serial.println(outbuf);
  Serial.println("************************************************************"); 
  Serial.println("**** Transmitting data every minute to pushdata.io *********");
  Serial.println("**** Go to https://pushdata.io/nerdy_gnu123@example.com ****");
  Serial.print("**** and click on the time series \""); Serial.print(outbuf); Serial.println("\"   ****");
  Serial.println("************************************************************"); 
}

void loop() {
  // Call monitorWiFi() in every loop iteration to make sure the WiFi
  // connection is functional (reconnect if we were disconnected)
  if (pd.monitorWiFi() != WL_CONNECTED) 
    return;
  // Store a value every minute
  unsigned long now = millis();
  if (now % 60000 == 0) {
    char outbuf[16];
    sprintf(outbuf, "%02x:%02x:%02x:%02x:%02x:%02x", macAddress[0], macAddress[1], macAddress[2], macAddress[3], macAddress[4], macAddress[5]);
    Serial.print(now); Serial.println(" => time to send something!");
    pd.send((float)now);
  }
}