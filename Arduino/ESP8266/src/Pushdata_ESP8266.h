#ifndef PUSHDATA_ESP8266_H
#define PUSHDATA_ESP8266_H

#include <Arduino.h>

#define DEBUG
// #undef DEBUG

#ifdef DEBUG
#define DBGPRINT(x) Serial.print(x)
#define DBGPRINTLN(x) Serial.println(x)
#else
#define DBGPRINT(x)
#define DBGPRINTLN(x)
#endif

#ifdef ESP8266
#include "ESP8266WiFi.h"
#include "WiFiUDP.h"

WiFiUDP UDP;

class Pushdata_ESP8266 {
    public:
        int connectWiFi(const char *ssid, const char *passwd) {
            if (!connecting) {
                WiFi.mode(WIFI_STA);
                if (WiFi.status() != WL_CONNECTED) {
                    connecting = true;
                    if (passwd && strlen(passwd)>0) {
                        WiFi.begin(ssid, passwd);
                    } else {
                        WiFi.begin(ssid);
                    }
                }
            } 
            return WiFi.status();
        }
        void setAPIKey(const char *key) {
            strncpy(apikey, key, 20);
        }
        void setEmail(const char *emailAddress) {
            strncpy(email, emailAddress, 50);
        }
        // Send data without a TS name, to be tagged with device MAC address
        int send(float value) {
            static char tsname[20];
            uint8_t mac[6];
            WiFi.macAddress(mac);
            sprintf(tsname, "---%02x%02x%02x%02x%02x%02x__", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
            DBGPRINT("Sending to TS "); DBGPRINTLN(tsname);
            return send(tsname, value, NULL, 0);
        }
        // Store data in a specific TS
        int send(const char *tsname, float value) {
            return send(tsname, value, NULL, 0);
        }
        // Store data in a specific TS and with tags
        // **tags = { "key1", "val1", "key2", "val2" ... }
        int _send(const char *tsname, float value, const char **tags, int numtags) {
            if ((apikey == NULL || strlen(apikey) || (email == NULL || strlen(email) == 0)) {
                Serial.println("Pushdata_ESP8266: Error: you must set email and api key");
                return 0;
            }
            if (UDP.begin(8844) != 1) {
                Serial.println("Pushdata_ESP8266: Error: failed to initialise UDP");
                return 0;
            }
            static char packetBuf[400];
            memset((void *)packetBuf, 0, 250);
            sprintf(packetBuf, "{\"name\":\"%s\",\"points\":[{\"value\":%f}]", tsname, value);
            if (numtags > 0) {
                int l;
                strcat(packetBuf, ",\"tags\":{");
                for (int i = 0; i < numtags; i++) {
                    l = strlen(packetBuf);
                    if ((strlen(tags[i*2])+strlen(tags[i*2+1])+8) >= 200) {
                        Serial.println("Pushdata_ESP8266: Error: packet size exceeded 200 bytes including tags");
                        return 0;
                    }
                    sprintf(packetBuf+l, "\"%s\":\"%s\",", tags[i*2], tags[i*2+1]);
                }
                // remove extraneous comma
                packetBuf[strlen(packetBuf)-1] = '\0';
                // add closing seagull
                strcat(packetBuf, "}");
            }
            // Add email
            if (apikey[0] != '\0') {
                sprintf(packetBuf+strlen(packetBuf), ",\"apikey\":\"%s\"", apikey);
            }
            // XXX TODO: add a field w randomized content, for increased security

            // XXX TODO: encrypt packets contents using pushdata.io public key

            // final closing brace
            strcat(packetBuf, "}");
            DBGPRINT("packetBuf: len="); DBGPRINTLN(strlen(packetBuf)); DBGPRINTLN(packetBuf);
            if (UDP.beginPacket("pushdata.io", 8844) != 1) {
                Serial.println("Pushdata_ESP8266: Error: Failed to resolve pushdata.io");
                while(1);
            }
            DBGPRINT("Wrote "); DBGPRINT(UDP.write(packetBuf, strlen(packetBuf))); DBGPRINTLN(" bytes");
            DBGPRINT("UDP.endPacket() returned "); DBGPRINTLN(UDP.endPacket());
            return 1;
        }
    private:
        char apikey[21] = "";
        char email[51] = "";
        bool connecting = false;

};

#endif

#endif