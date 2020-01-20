#include "HttpsClient.h"

#include <SoftwareSerial.h>
#include <pb_common.h>
#include <pb_encode.h>

#include <TinyGPS++.h>
#include <SimpleTimer.h>

#include <./SpatialTelemetry.pb.h>


SoftwareSerial SerialGPS(D7, D8);
TinyGPSPlus gps;
SimpleTimer timer;


// Define the serial console for debug prints, if needed
// #define TINY_GSM_DEBUG SerialMon

// Set serial for debug console (to the Serial Monitor, default speed 115200)
#define SerialMon Serial

// Set serial for AT commands (to the module)
#define SerialAT Serial

String server = "us-central1-spatial-telemetry.cloudfunctions.net";
const int  port = 443;
const char resource[] = "/spatial-telemetry-receive";
const char contentType[] = "binary/octet-stream";

HttpsClient httpsClient(&server, port, &SerialMon, &SerialAT);

int timerId;

void updateGPS() {
  //read data from GPS module
  
  while (SerialGPS.available() > 0) {
    // Serial.println("Attempting to get GPS data.");
    gps.encode(SerialGPS.read());
  }
}

void writeSpatialTelemetry() {

    SerialMon.println("About to write spatial telemetry...");
    if (gps.location.isValid()) {

        uint8_t buffer[128];
        size_t message_length;
        bool status;

        SpatialTelemetry telemetry = SpatialTelemetry_init_zero;
        pb_ostream_t stream = pb_ostream_from_buffer(buffer, sizeof(buffer));

        int32 deviceId = 1;
        int32 timestamp = 1573153482;

        SerialMon.print("Lat: ");
        SerialMon.println(gps.location.lat());

        SerialMon.print("Lng: ");
        SerialMon.println(gps.location.lng());

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
            SerialMon.printf("Encoding failed: %s\n", PB_GET_ERROR(&stream));
            return;
        }

        

        SerialMon.print("[HTTPS] begin...\n");

        httpsClient.http->connectionKeepAlive();

        SerialMon.print("[HTTPS] POST...\n");
        int err = httpsClient.http->post(resource, contentType, message_length, buffer);

        if (err != 0) {
          SerialMon.println(F("failed to connect"));
          SerialMon.println(err);
        } else {

          int httpCode = httpsClient.http->responseStatusCode();

          // httpCode will be negative on error
          if (httpCode > 0) {
            // HTTP header has been send and Server response header has been handled
            SerialMon.printf("[HTTPS] POST... code: %d\n", httpCode);

            // file found at server
            if (httpCode == 200) {
              String responseBody = httpsClient.http->responseBody();
              SerialMon.println(responseBody);
            }
          } else {
            SerialMon.printf("[HTTPS] POST... failed, response code: %d\n", httpCode);
          }

          httpsClient.http->stop();
        }

    } else {
        SerialMon.println("Invalid GPS data");
    }

}

void setup() {

    
    // Initialize serial and wait for port to open:
    SerialMon.begin(9600);
    // Initialize GPS serial
    SerialGPS.begin(9600);

    httpsClient.ConnectNetwork(); 
    delay(1000);

    timerId = timer.setInterval(10000L, writeSpatialTelemetry);
}

void loop() {
    timer.run(); // Initiates BlynkTimer
    updateGPS();
}