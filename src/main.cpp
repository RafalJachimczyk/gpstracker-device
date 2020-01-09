
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>

#include <SoftwareSerial.h>
#include <pb_common.h>
#include <pb_encode.h>

#include <TinyGPS++.h>
#include <SimpleTimer.h>

#include <./SpatialTelemetry.pb.h>
#include "./credentials.h"

// SoftwareSerial SerialGPS(D7, D8); // RX, TX
SoftwareSerial SerialGPS(D7, D8);

WiFiClient client;
// TinyGsm modem(Serial);
// TinyGsmClient client(modem);

const char telemetryServer[] = "0.tcp.ngrok.io";
const int  telemetryPort = 18300;

TinyGPSPlus gps;
SimpleTimer timer;


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


        if (!client.connect(telemetryServer, telemetryPort)) {
            
            timer.setTimeout(2000L, []() {
                Serial.println("connection to telemetry failed");
            });
            return;
        }

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

        client.write(buffer, stream.bytes_written);
        client.stop();
        Serial.println("Written spatial telemetry...");


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