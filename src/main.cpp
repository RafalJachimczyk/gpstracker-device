
#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h> 
#include <ESP8266HTTPClient.h>

#include <SoftwareSerial.h>
#include <pb_common.h>
#include <pb_encode.h>

#include <TinyGPS++.h>
#include <SimpleTimer.h>

#include <./SpatialTelemetry.pb.h>
#include "./credentials.h"

SoftwareSerial SerialGPS(D7, D8);

TinyGPSPlus gps;
SimpleTimer timer;

const char fingerprint[] PROGMEM = "2A 97 72 75 5B BF BD EB 41 E8 1D D0 59 81 F0 7A 57 98 75 88";

int timerId;

void updateGPS() {
  //read data from GPS module
  
  while (SerialGPS.available() > 0) {
    // Serial.println("Attempting to get GPS data.");
    gps.encode(SerialGPS.read());
  }
}

void writeSpatialTelemetry() {

    Serial.println("About to write spatial telemetry...");
    if (gps.location.isValid()) {

        uint8_t buffer[128];
        size_t message_length;
        bool status;

        SpatialTelemetry telemetry = SpatialTelemetry_init_zero;
        pb_ostream_t stream = pb_ostream_from_buffer(buffer, sizeof(buffer));

        int32 deviceId = 1;
        int32 timestamp = 1573153482;

        Serial.print("Lat: ");
        Serial.println(gps.location.lat());

        Serial.print("Lng: ");
        Serial.println(gps.location.lng());

        float lat = gps.location.lat();
        float lon = gps.location.lng();
        int32 gsmStrength = 25;

        telemetry.deviceId = deviceId;
        telemetry.timestamp = timestamp;
        telemetry.latitude = lat;
        telemetry.longitude = lon;
        telemetry.gsmStrength = gsmStrength;


        status = pb_encode(&stream, SpatialTelemetry_fields, &telemetry);
        message_length = stream.bytes_written;

        if (!status) {
            Serial.printf("Encoding failed: %s\n", PB_GET_ERROR(&stream));
            return;
        }

        std::unique_ptr<BearSSL::WiFiClientSecure>client(new BearSSL::WiFiClientSecure);

        client->setFingerprint(fingerprint);

        HTTPClient https;

        Serial.print("[HTTPS] begin...\n");
        if (https.begin(*client, "https://us-central1-spatial-telemetry.cloudfunctions.net/spatial-telemetry-receive")) {  // HTTPS

          Serial.print("[HTTPS] POST...\n");
          // start connection and send HTTP header
          int httpCode = https.POST(buffer, message_length);

          // httpCode will be negative on error
          if (httpCode > 0) {
            // HTTP header has been send and Server response header has been handled
            Serial.printf("[HTTPS] POST... code: %d\n", httpCode);

            // file found at server
            if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
              String payload = https.getString();
              Serial.println(payload);
            }
          } else {
            Serial.printf("[HTTPS] POST... failed, response code: %d, error: %s\n", httpCode,  https.errorToString(httpCode).c_str());
          }

          https.end();
        } else {
          Serial.printf("[HTTPS] Unable to connect\n");
        }
        
        // client.write(buffer, stream.bytes_written);
        // client.stop();
        // Serial.println("Written spatial telemetry...");


    } else {
        Serial.println("Invalid GPS data");
    }



}

void setupWiFi()
{
  WiFi.mode(WIFI_STA); // added 300716
  WiFi.begin(ssid, password);
  Serial.println();
  Serial.print("Connecting");
    
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
   }

  Serial.println();
  
  Serial.print("Connected, IP address: ");
  Serial.println(WiFi.localIP());
}

void setup() {

    
    // Initialize serial and wait for port to open:
    Serial.begin(9600);

    // Initialize GPS serial
    SerialGPS.begin(9600);

    setupWiFi();

    timerId = timer.setInterval(10000L, writeSpatialTelemetry);

    // timer.setInterval(1000L, myGPSEvent);
}

void loop() {
    timer.run(); // Initiates BlynkTimer
    updateGPS();
}