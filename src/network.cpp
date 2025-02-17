// network.cpp

#include "network.hpp"

#include <Arduino_JSON.h>
#include <HTTPClient.h>
#include <WiFi.h>



namespace paper {

static const char* GEO_API_REQUEST  = "http://ip-api.com/json/?fields=status,message,country,regionName,city,timezone,offset";



int network_connect(const char* ssid, const char* password)
{
    static constexpr int TIMEOUT_MS = 15000;

    Serial.printf("NETWORK > Connecting to network '%s'", ssid);
    WiFi.begin(ssid, password);

    {
        uint32_t start = millis();

        while (WiFi.status() != WL_CONNECTED) {
            delay(500);
            Serial.print(".");

            if (millis() - start >= TIMEOUT_MS) {
                Serial.print("\nError: Could not connect to network.\n");
                return -1;
            }
        }

        Serial.print(" done.\n");
    }

    return 0;
}

void network_disconnect()
{
    WiFi.disconnect();
    Serial.print("NETWORK > Disconnected.\n");
}

String network_http_request(const char* server_request)
{
    static constexpr int TIMEOUT_MS = 15000;

    WiFiClient client;
    HTTPClient http;

    http.begin(client, server_request);
    http.setTimeout(TIMEOUT_MS);

    // Send HTTP POST request
    int response_code = http.GET();

    String payload = "{}"; 

    if (response_code > 0)
        payload = http.getString();
    else
        Serial.printf("Error: HTTP response %d.\n", response_code);

    // Free resources
    http.end();

    return payload;
}

int network_api_utc_dst_offset()
{
    Serial.print("NETWORK > Requesting location...\n");

    String response = network_http_request(GEO_API_REQUEST);
    JSONVar object = JSON.parse(response);

    // JSON.typeof(jsonVar) can be used to get the type of the var
    if (JSON.typeof(object) == "undefined") {
        Serial.println("Error: JSON parsing failed.");
        Serial.println(response);
        return 0;
    }

    if (!object.hasOwnProperty("offset") || JSON.typeof(object["offset"]) != "number") {
        Serial.println("Error: JSON missing required element.");
        Serial.println(response);
        return 0;
    }

    const int offset = int(object["offset"]);

    Serial.print("NETWORK > ");
    Serial.print((const String&) object["city"]);
    Serial.print(", ");
    Serial.print((const String&) object["regionName"]);
    Serial.print(", ");
    Serial.println((const String&) object["country"]);
    Serial.flush();

    Serial.print("NETWORK > ");
    Serial.print((const String&) object["timezone"]);
    Serial.printf(" (UTC offset: %d s)\n", offset);

    return offset;
}

} // namespace paper