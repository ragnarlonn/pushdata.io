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
            DBGPRINT("Sending to TS "); DBGPRINTLN(tsname);
            return _send(tsname, value, NULL, 0);
        }
        // Store data in a specific TS
        int send(const char *tsname, float value) {
            return _send(tsname, value, NULL, 0);
        }
        // Store data in a specific TS and with tags
        // **tags = { "key1", "val1", "key2", "val2" ... }
        int send(const char *tsname, float value, const char **tags, int numtags) {
            return _send(tsname, value, tags, numtags);
        }
        // Function that does the actual sending
        int _send(const char *tsname, float value, const char **tags, int numtags) {
            static char pubkey[] = "-----BEGIN PUBLIC KEY-----"
"MIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEAtbra1mhjjMaiZlIZIsky"
"vewXvNuJNLE8RKZtstuRSLu5TEyIHwaeFb9oKe+bHDRVmi3a15qaedhlaTNNAx1Z"
"o0pRg2c3yRgJr6XocO41jqGOP8WGw6008B9XjKpvL9NuEfpUHXeT6hlFsLk87av4"
"S/MOTGMHnaXhOU8mRQAiw3vRTEY/zrx3hMFPGcFEmKcEWKlkLSnqFQwnNy/74iBg"
"VQYN6MbtsmscaPB/luTVv1lX8+MBxXpQWwSSqKbDwUGZUEcE0S9lSQ416TlmTntx"
"knFJTidG98mFIJ8ryZki1ZINY8NsIBan35JacNR8l6nS8enFPL3DpZMgKIIZCs0W"
"AwIDAQAB"
"-----END PUBLIC KEY-----";
            if ((email == NULL || strlen(email) == 0) && (apikey == NULL || strlen(apikey) == 0)) {
                Serial.println("Pushdata_ESP8266_SSL: Error: you must set either an email or an api key");
                return 0;
            }
            BearSSL::WiFiClientSecure client;
            BearSSL::PublicKey key(pubkey);
            client.setKnownKey(&key);
            //client.setInsecure();
            if (!client.connect("pushdata.io", 443)) {
                Serial.println("Pushdata_ESP8266: Error: failed to connect to pushdata.io:443");
                return 0;
            }
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
            DBGPRINT("packetBuf: len="); DBGPRINTLN(strlen(packetBuf)); DBGPRINTLN(packetBuf);
            DBGPRINT("Wrote "); DBGPRINT(httpPOST(&client, packetBuf)); DBGPRINTLN(" bytes");

Serial.println("request sent");
  while (client.connected()) {
    String line = client.readStringUntil('\n');
    Serial.println(line);
  }
  String line = client.readStringUntil('\n');
  Serial.println("reply was:");
  Serial.println("==========");
  Serial.println(line);
  Serial.println("==========");
  Serial.println("closing connection");

            return 1;
        }
    private:
        char apikey[21] = "";
        char email[51] = "";
        bool connecting = false;
        /*
        char pubkey[400] = "-----BEGIN PUBLIC KEY-----"
"MIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEAtbra1mhjjMaiZlIZIsky"
"vewXvNuJNLE8RKZtstuRSLu5TEyIHwaeFb9oKe+bHDRVmi3a15qaedhlaTNNAx1Z"
"o0pRg2c3yRgJr6XocO41jqGOP8WGw6008B9XjKpvL9NuEfpUHXeT6hlFsLk87av4"
"S/MOTGMHnaXhOU8mRQAiw3vRTEY/zrx3hMFPGcFEmKcEWKlkLSnqFQwnNy/74iBg"
"VQYN6MbtsmscaPB/luTVv1lX8+MBxXpQWwSSqKbDwUGZUEcE0S9lSQ416TlmTntx"
"knFJTidG98mFIJ8ryZki1ZINY8NsIBan35JacNR8l6nS8enFPL3DpZMgKIIZCs0W"
"AwIDAQAB"
"-----END PUBLIC KEY-----";
*/
//    char gurks[] = "hej";
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