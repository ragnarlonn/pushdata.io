#ifndef PUSHDATA_ESP8266_H
#define PUSHDATA_ESP8266_H

#include <Arduino.h>

#define DEBUG
// #undef DEBUG

#ifdef DEBUG
#define DBGPRINTHEADER Serial.print("Pushdata_8266_SSL: ")
#define DBGPRINT(X) (Serial.print(X))
#define DBGPRINTLN(X) (Serial.println(X))
#define DBGPRINTH(X) (DBGPRINTHEADER && Serial.print(X))
#define DBGPRINTHLN(X) (DBGPRINTHEADER && Serial.println(X))
#else
#define DBGPRINT(X)
#define DBGPRINTLN(X)
#define DBGPRINTH(X)
#define DBGPRINTHLN(X)
#endif

#ifdef ESP8266
#include "ESP8266WiFi.h"
#include "WiFiClientSecure.h"
#include "BearSSLHelpers.h"

class Pushdata_ESP8266_SSL {
    public:
        void setEmail(const char *emailAddress) {
            strncpy(email, emailAddress, 50);
        }
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
        void setApiKey(const char *key) {
            strncpy(apikey, key, 20);
        }
        // Send data without a TS name, to be tagged with device MAC address
        int send(float value) {
            static char tsname[20];
            uint8_t mac[6];
            WiFi.macAddress(mac);
            sprintf(tsname, "---%02x%02x%02x%02x%02x%02x__", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
            DBGPRINTH("Sending to TS "); DBGPRINTLN(tsname);
            //ESP.wdtDisable();
            int ret = _send(tsname, value, NULL, 0);
            //ESP.wdtEnable(5000);
            return ret;
        }
        // Store data in a specific TS
        int send(const char *tsname, float value) {
            ESP.wdtDisable();
            int ret = _send(tsname, value, NULL, 0);
            ESP.wdtEnable(5000);
            return ret;
        }
        // Store data in a specific TS and with tags
        // **tags = { "key1", "val1", "key2", "val2" ... }
        int send(const char *tsname, float value, const char **tags, int numtags) {
            ESP.wdtDisable();
            int ret = _send(tsname, value, tags, numtags);
            ESP.wdtEnable(5000);
            return ret;
        }
        // Function that does the actual sending
        int _send(const char *tsname, float value, const char **tags, int numtags) {
            const char pubkey[] = "-----BEGIN PUBLIC KEY-----\n"
                "MIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEA1hXt5g1q0NbHAQwG4w6T\n"
                "w9YtviEJytQjM+fBsMeUEol1d8qOqVgF6aiOthYJM1yKytKQ488tAIXdx7w4dM77\n"
                "lLKLKXjEkeIruVf0L5RRLSMH6NzIAVOpImggQWs5VU+nmmj6TOK7pQNqi9SveJMZ\n"
                "L/XgI10GzjbJU5Bv5sKZr7OY99Ot6DTGyyreK2wLw5GHTuKlh/yRypDiw0G1d9nO\n"
                "E9tNdzCtJzSJJfd/ZZ32fpu0tXzRKyD5lWeR4Qu0qTjCYajZXDxMrBd3DbrROrEf\n"
                "rl0u7L4Hiu6xxENObqS7PEBtdgM3WqnjH6GUc9UNH+1JSnpGOPffpXYHQ0aGUr52\n"
                "FwIDAQAB\n"
                "-----END PUBLIC KEY-----";
            if ((email == NULL || strlen(email) == 0) && (apikey == NULL || strlen(apikey) == 0)) {
                Serial.println("Pushdata_ESP8266_SSL: Error: you must set either an email or an api key");
                return 0;
            }
            DBGPRINTHLN("BearSSL::WifiClientSecure init");
            BearSSL::WiFiClientSecure client;
            DBGPRINTHLN("BearSSLPublicKey init");
            BearSSL::PublicKey key;
            if (key.parse((uint8_t *)pubkey, strlen(pubkey))) {
                DBGPRINTHLN("key.parse() succeeded");
            } else {
                DBGPRINTHLN("key.parse() failed");
                while (1);
            }
            DBGPRINTHLN("client.setKnownKey()");
            client.setKnownKey(&key);
            //client.setInsecure();
            yield();
            //client.setCiphersLessSecure();
            DBGPRINTHLN("client.connect()");
            unsigned long startConnect = millis();
            client.setDefaultNoDelay(true);
            if (!client.connect("pushdata.io", 443)) {
                Serial.println("Pushdata_ESP8266: Error: failed to connect to pushdata.io:443");
                char buf[200];
                int err = client.getLastSSLError(buf, 199);
                buf[199] = '\0';
                Serial.println("Last SSL error was:");
                Serial.println(buf);
                Serial.print("ERRCODE: "); Serial.println(err);
                return 0;
            }
            DBGPRINTH("connect took "); DBGPRINT(millis()-startConnect); DBGPRINTLN(" ms");
            DBGPRINTHLN("Connected to pushdata.io:443");
            static char packetBuf[250];
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
            // final closing brace
            strcat(packetBuf, "}");
            DBGPRINTH("packetBuf: len="); DBGPRINTLN(strlen(packetBuf)); DBGPRINT("   "); DBGPRINTLN(packetBuf);
            DBGPRINTH("Wrote "); DBGPRINT(httpPOST(&client, packetBuf)); DBGPRINTLN(" bytes");
            DBGPRINTHLN("request sent, reading response:");
            // Wait max 2s for response
            unsigned long startRecv = millis();
            client.setTimeout(1000); // should be the default, but we're paranoid
            while (client.connected()) {
                yield();
                String line = client.readStringUntil('\n');
                DBGPRINT("   "); DBGPRINTLN(line);
                if (line == "\r\n") {
                    // End of HTTP response encountered
                    break;
                }
                if ((millis() - startRecv) >= 2000) {
                    Serial.println("Pushdata_ESP8266_SSL: timed out reading HTTP response");
                    break;
                }
            }
            client.stop();
            return 1;
        }
    private:
        char apikey[21] = "";
        char email[51] = "";
        bool connecting = false;
        int _httpPOST(BearSSL::WiFiClientSecure *client, const char *queryParam, const char *payload) {
            int written = 0;
            written += client->print(String("POST /api/timeseries?") + queryParam + " HTTP/1.1\r\n");
            written += client->print("Host: pushdata.io\r\n");
            written += client->print("User-Agent: Pushdata_ESP8266_SSL\r\n");
            written += client->print("Content-Type: application/json\r\n");
            written += client->print(String("Content-Length: ") + strlen(payload) + "\r\n");
            written += client->print("Connection: close\r\n\r\n");
            written += client->println(payload);
            return written;
        }
        int httpPOST(BearSSL::WiFiClientSecure *client, const char *payload) {
            if (apikey != NULL && strlen(apikey) > 0) {
                return _httpPOST(client, (String("apikey=") + apikey).c_str(), payload);
            }
            if (email != NULL && strlen(email) > 0) {
                return _httpPOST(client, (String("email=") + email).c_str(), payload);
            }
            return 0;
        }

};

#endif

#endif