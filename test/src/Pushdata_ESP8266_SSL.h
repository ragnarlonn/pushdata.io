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
            ESP.wdtDisable();
            int ret = _send(tsname, value, NULL, 0);
            ESP.wdtEnable(5000);
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
            "MIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEAtbra1mhjjMaiZlIZIsky\n"
            "vewXvNuJNLE8RKZtstuRSLu5TEyIHwaeFb9oKe+bHDRVmi3a15qaedhlaTNNAx1Z\n"
            "o0pRg2c3yRgJr6XocO41jqGOP8WGw6008B9XjKpvL9NuEfpUHXeT6hlFsLk87av4\n"
            "S/MOTGMHnaXhOU8mRQAiw3vRTEY/zrx3hMFPGcFEmKcEWKlkLSnqFQwnNy/74iBg\n"
            "VQYN6MbtsmscaPB/luTVv1lX8+MBxXpQWwSSqKbDwUGZUEcE0S9lSQ416TlmTntx\n"
            "knFJTidG98mFIJ8ryZki1ZINY8NsIBan35JacNR8l6nS8enFPL3DpZMgKIIZCs0W\n"
            "AwIDAQAB\n"
            "-----END PUBLIC KEY-----";
            /*
            const char LetsEncryptRootCert[] = "-----BEGIN CERTIFICATE-----\n"
                "MIIFazCCA1OgAwIBAgIRAIIQz7DSQONZRGPgu2OCiwAwDQYJKoZIhvcNAQELBQAw\n"
                "TzELMAkGA1UEBhMCVVMxKTAnBgNVBAoTIEludGVybmV0IFNlY3VyaXR5IFJlc2Vh\n"
                "cmNoIEdyb3VwMRUwEwYDVQQDEwxJU1JHIFJvb3QgWDEwHhcNMTUwNjA0MTEwNDM4\n"
                "WhcNMzUwNjA0MTEwNDM4WjBPMQswCQYDVQQGEwJVUzEpMCcGA1UEChMgSW50ZXJu\n"
                "ZXQgU2VjdXJpdHkgUmVzZWFyY2ggR3JvdXAxFTATBgNVBAMTDElTUkcgUm9vdCBY\n"
                "MTCCAiIwDQYJKoZIhvcNAQEBBQADggIPADCCAgoCggIBAK3oJHP0FDfzm54rVygc\n"
                "h77ct984kIxuPOZXoHj3dcKi/vVqbvYATyjb3miGbESTtrFj/RQSa78f0uoxmyF+\n"
                "0TM8ukj13Xnfs7j/EvEhmkvBioZxaUpmZmyPfjxwv60pIgbz5MDmgK7iS4+3mX6U\n"
                "A5/TR5d8mUgjU+g4rk8Kb4Mu0UlXjIB0ttov0DiNewNwIRt18jA8+o+u3dpjq+sW\n"
                "T8KOEUt+zwvo/7V3LvSye0rgTBIlDHCNAymg4VMk7BPZ7hm/ELNKjD+Jo2FR3qyH\n"
                "B5T0Y3HsLuJvW5iB4YlcNHlsdu87kGJ55tukmi8mxdAQ4Q7e2RCOFvu396j3x+UC\n"
                "B5iPNgiV5+I3lg02dZ77DnKxHZu8A/lJBdiB3QW0KtZB6awBdpUKD9jf1b0SHzUv\n"
                "KBds0pjBqAlkd25HN7rOrFleaJ1/ctaJxQZBKT5ZPt0m9STJEadao0xAH0ahmbWn\n"
                "OlFuhjuefXKnEgV4We0+UXgVCwOPjdAvBbI+e0ocS3MFEvzG6uBQE3xDk3SzynTn\n"
                "jh8BCNAw1FtxNrQHusEwMFxIt4I7mKZ9YIqioymCzLq9gwQbooMDQaHWBfEbwrbw\n"
                "qHyGO0aoSCqI3Haadr8faqU9GY/rOPNk3sgrDQoo//fb4hVC1CLQJ13hef4Y53CI\n"
                "rU7m2Ys6xt0nUW7/vGT1M0NPAgMBAAGjQjBAMA4GA1UdDwEB/wQEAwIBBjAPBgNV\n"
                "HRMBAf8EBTADAQH/MB0GA1UdDgQWBBR5tFnme7bl5AFzgAiIyBpY9umbbjANBgkq\n"
                "hkiG9w0BAQsFAAOCAgEAVR9YqbyyqFDQDLHYGmkgJykIrGF1XIpu+ILlaS/V9lZL\n"
                "ubhzEFnTIZd+50xx+7LSYK05qAvqFyFWhfFQDlnrzuBZ6brJFe+GnY+EgPbk6ZGQ\n"
                "3BebYhtF8GaV0nxvwuo77x/Py9auJ/GpsMiu/X1+mvoiBOv/2X/qkSsisRcOj/KK\n"
                "NFtY2PwByVS5uCbMiogziUwthDyC3+6WVwW6LLv3xLfHTjuCvjHIInNzktHCgKQ5\n"
                "ORAzI4JMPJ+GslWYHb4phowim57iaztXOoJwTdwJx4nLCgdNbOhdjsnvzqvHu7Ur\n"
                "TkXWStAmzOVyyghqpZXjFaH3pO3JLF+l+/+sKAIuvtd7u+Nxe5AW0wdeRlN8NwdC\n"
                "jNPElpzVmbUq4JUagEiuTDkHzsxHpFKVK7q4+63SM1N95R1NbdWhscdCb+ZAJzVc\n"
                "oyi3B43njTOQ5yOf+1CceWxG1bQVs5ZufpsMljq4Ui0/1lvh+wjChP4kqKOJ2qxq\n"
                "4RgqsahDYVvTH9w7jXbyLeiNdd8XM2w9U/t7y0Ff/9yi0GE44Za4rF2LN9d11TPA\n"
                "mRGunUHBcnWEvgJBQl9nJEiU0Zsnvgc/ubhPgXRR4Xq37Z0j4r7g1SgEEzwxA57d\n"
                "emyPxgcYxn/eR44/KJ4EBs+lVDR3veyJm+kXQ99b21/+jh5Xos1AnX5iItreGCc=\n"
                "-----END CERTIFICATE-----\n";
                */
            if ((email == NULL || strlen(email) == 0) && (apikey == NULL || strlen(apikey) == 0)) {
                Serial.println("Pushdata_ESP8266_SSL: Error: you must set either an email or an api key");
                return 0;
            }
            DBGPRINTLN("Pushdata_ESP8266_SSL: BearSSL::WifiClientSecure init");
            BearSSL::WiFiClientSecure client;
            /*
            BearSSL::X509List x509;
            if (x509.append(LetsEncryptRootCert)) {
                DBGPRINTLN("Pushdata_ESP8266_SSL: parsing of x509 root cert succeeded");
            } else {
                DBGPRINTLN("Pushdata_ESP8266_SSL: parsing of x509 root cert failed");
            }
            client.setTrustAnchors(&x509);
            */
            DBGPRINTLN("Pushdata_ESP8266_SSL: BearSSLPublicKey init");
            BearSSL::PublicKey key;
            if (key.parse((uint8_t *)pubkey, strlen(pubkey))) {
                DBGPRINTLN("Pushdata_ESP8266_SSL: key.parse() succeeded");
            } else {
                DBGPRINTLN("Pushdata_ESP8266_SSL: key.parse() failed");
                while (1);
            }
            DBGPRINTLN("Pushdata_ESP8266_SSL: client.setKnownKey()");
            client.setKnownKey(&key);
            //client.setInsecure();
            yield();
            client.setCiphersLessSecure();
            DBGPRINTLN("Pushdata_ESP8266_SSL: client.connect()");
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
            DBGPRINTLN("Pushdata_ESP8266_SSL: Connected to pushdata.io:443");
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