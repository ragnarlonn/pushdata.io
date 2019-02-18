#include <Arduino.h>
// #include <Wire.h>
#include <U8g2lib.h>
#include <ESP8266WiFi.h>
#include "Pushdata_ESP8266_SSL.h"
#include "user_interface.h"

U8X8_SSD1306_128X32_UNIVISION_SW_I2C u8x8(/* clock=*/ SCL, /* data=*/ SDA, /* reset=*/ 16);   // Adafruit Feather ESP8266/32u4 Boards + FeatherWing OLED

Pushdata_ESP8266_SSL pd;

char ssid[] = "Goto 10";
char passwd[] = "datorbyxa";

//char ssid[] = "APA";
//char passwd[] = "deadbeef42";

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
  u8x8.draw2x2UTF8(0, 0, "NodeMCU");
  OLEDOutput("Starting...", "");
  pd.setEmail("nerdy_gnu123@example.com");
  while (pd.connectWiFi(ssid, passwd) != WL_CONNECTED) {
    OLEDOutput("Connecting...", "");
    delay(500);
    OLEDOutput("", "");
    delay(250);
  }
  OLEDOutput("Connected!", "");
  delay(1500);
}

int pos = 0;

void loop() {
  unsigned long now = millis();
  // Store a value every minute
  if (now % 60000 == 0) {
    char buf[12];
    char outbuf[16];
    WiFi.macAddress((uint8_t*)buf);
    sprintf(outbuf, "HW %02x%02x%02x%02x%02x%02x", buf[0], buf[1], buf[2], buf[3], buf[4], buf[5]);
    OLEDOutput("Sending", outbuf);
    Serial.print(now); Serial.println(" => time to send something!");
    pd.send((float)now);
    OLEDOutput("Sleeping", "");
  }
}