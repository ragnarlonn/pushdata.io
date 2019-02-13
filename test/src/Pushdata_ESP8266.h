#ifndef PUSHDATA_ESP8266_H
#define PUSHDATA_ESP8266_H

#include <Arduino.h>


#ifdef ESP8266
#include "ESP8266WiFi.h"
#include "WiFiUDP.h"
#include "WiFiClientSecure.h"

WiFiUDP UDP;

class Pushdata_ESP8266 {
    public:
        int begin() {
            return UDP.begin(8844);
        }
        void setAPIKey(const char *key) {
            strncpy(apikey, key, 20);
        }
        // Send data without a TS name, to be tagged with device MAC address
        int send(float value) {
            static char tsname[20];
            uint8_t mac[6];
            char microseconds[10];
            WiFi.macAddress(mac);
            itoa(micros64(), microseconds, 10);
            sprintf(tsname, "___%s_%02x%02x%02x%02x%02x%02x__", microseconds, 
                    mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
            Serial.print("Sending to TS "); Serial.println(tsname);
            return send(tsname, value, NULL, 0);
        }
        // Store data in a specific TS
        int send(const char *tsname, float value) {
            return send(tsname, value, NULL, 0);
        }
        // Store data in a specific TS and with tags
        // **tags = { "key1", "val1", "key2", "val2" ... }
        int send(const char *tsname, float value, const char **tags, int numtags) {
            static char packetBuf[200];
            memset((void *)packetBuf, 0, 200);
            sprintf(packetBuf, "{\"name\":\"%s\",\"points\":[{\"value\":%f}]", tsname, value);
            if (numtags > 0) {
                int l;
                strcat(packetBuf, ",\"tags\":{");
                for (int i = 0; i < numtags; i++) {
                    l = strlen(packetBuf);
                    if ((strlen(tags[i*2])+strlen(tags[i*2+1])+8) >= 200) {
                        return 1;
                    }
                    sprintf(packetBuf+l, "\"%s\":\"%s\",", tags[i*2], tags[i*2+1]);
                }
                // remove extraneous comma
                packetBuf[strlen(packetBuf)-1] = '\0';
                // add closing seagull
                strcat(packetBuf, "}");
            }
            // final closing brace
            strcat(packetBuf, "}");
            Serial.print("packetBuf: len="); Serial.println(strlen(packetBuf)); Serial.println(packetBuf);
            if (UDP.beginPacket("35.171.228.197", 8844) != 1) {
                Serial.println("Error resolving pushdata.io");
                while(1);
            }
            Serial.println("Survived UDP.beginPacket()");
            Serial.print("Wrote "); Serial.print(UDP.write(packetBuf)); Serial.println(" bytes");
            Serial.print("UDP.endPacket() returned "); Serial.println(UDP.endPacket())
            ;
            return 0;
        }
    private:
        char apikey[21] = "";

};

#endif

#endif