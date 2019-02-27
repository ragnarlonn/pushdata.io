#include <Arduino.h>
// #include <Wire.h>
#include <U8g2lib.h>
#include <ESP8266WiFi.h>
#include "Pushdata_ESP8266_SSL.h"

U8X8_SSD1306_128X32_UNIVISION_SW_I2C u8x8(/* clock=*/ SCL, /* data=*/ SDA, /* reset=*/ 16);   // Adafruit Feather ESP8266/32u4 Boards + FeatherWing OLED

Pushdata_ESP8266_SSL pd;

char ssid[] = "MyWiFiNetwork";
char passwd[] = "MyWiFiPassword";
char macAddress[12];

void OLEDOutput(const char *s1, const char *s2) {
  u8x8.clearLine(2);
  u8x8.clearLine(3);
  if (strlen(s1) > 0) {
    u8x8.drawUTF8(0, 2, s1);
  }
  if (strlen(s2) > 0) {
    u8x8.drawUTF8(0, 3, s2);
  }
}

void setup() {
  Serial.begin(115200);
  Serial.println();
  u8x8.begin();
  u8x8.setPowerSave(0);
  u8x8.setFont(u8x8_font_chroma48medium8_r);
  u8x8.draw2x2UTF8(0, 0, "Pushdata");
  OLEDOutput("Starting...", "");
  // Fallback to using nerdy_gnu123@example.com email
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
  delay(1500);
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
    OLEDOutput("Sending", outbuf);
    Serial.print(now); Serial.println(" => time to send something!");
    pd.send((float)now);
    OLEDOutput("Sleeping", "");
  }
}