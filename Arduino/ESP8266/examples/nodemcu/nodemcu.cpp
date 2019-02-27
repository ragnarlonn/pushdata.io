#include <Arduino.h>
#include <ESP8266WiFi.h>
#include "Pushdata_ESP8266_SSL.h"

Pushdata_ESP8266_SSL pd;

char ssid[] = "MyWiFiNetwork";
char passwd[] = "MyWiFiPassword";

void setup() {
  // Using nerdy_gnu123@example.com account email - check it out at https://pushdata.io/nerdy_gnu123@example.com
  pd.setEmail("nerdy_gnu123@example.com");
  pd.addWiFi(ssid, passwd);
}

void loop() {
  // Call monitorWiFi() in every loop iteration to make sure the WiFi
  // connection is functional (reconnect if we were disconnected)
  if (pd.monitorWiFi() != WL_CONNECTED) 
    return;
  // Store the value 4711 once every minute. 
  // Note that we do not configure a name for the time series here - the Pushdata library will then use 
  // the ethernet MAC address of the NodeMCU as the time series name. You can change the primary name 
  // to something else in the Pushdata UI, but the MAC address name will always remain, and function as 
  // an alias name (so your device can continue sending data to it without any kind of reconfiguration)
  if (millis() % 60000 == 0) {
    pd.send(4711);
  }
}