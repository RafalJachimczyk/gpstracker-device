#include "writeSpatialTelemetry.h"

const char resource[] = "/spatial-telemetry-receive";
const char contentType[] = "binary/octet-stream";

// Voltage monitoring


void writeSpatialTelemetry(HttpsClient *httpsClient, gps_fix *fix, HardwareSerial *SerialMon, HardwareSerial *SerialAT) {

    // SerialMon->println("About to write spatial telemetry...");
    // if (gps->location.isValid()) {

        uint8_t buffer[128];
        size_t message_length;
        bool status;

        SpatialTelemetry telemetry = SpatialTelemetry_init_zero;
        pb_ostream_t stream = pb_ostream_from_buffer(buffer, sizeof(buffer));

        int deviceId = 1;
        int timestamp = 1573153482;


        float lat = fix->latitude();
        float lng = fix->longitude();

        SerialMon->print("Lat var: ");
        SerialMon->printf("%.6f\n", lat);

        SerialMon->print("Lng var: ");
        SerialMon->printf("%.6f\n\n", lng);

        int gsmStrength = httpsClient->GetGsmStrength();

        telemetry.deviceId = deviceId;
        telemetry.timestamp = timestamp;
        telemetry.latitude = lat;
        telemetry.longitude = lng;
        telemetry.gsmStrength = gsmStrength;
        telemetry.voltage = 1;


        status = pb_encode(&stream, SpatialTelemetry_fields, &telemetry);
        message_length = stream.bytes_written;

        if (!status) {
            SerialMon->printf("Encoding failed: %s\n", PB_GET_ERROR(&stream));
            return;
        }

        SerialMon->print("[HTTPS] begin...\n");

        httpsClient->http->connectionKeepAlive();

        SerialMon->print("[HTTPS] POST...\n");
        int err = httpsClient->http->post(resource, contentType, message_length, buffer);

        if (err != 0) {
          SerialMon->println(F("failed to connect"));
          SerialMon->println(err);
        } else {

          int httpCode = httpsClient->http->responseStatusCode();

          // httpCode will be negative on error
          if (httpCode > 0) {
            // HTTP header has been send and Server response header has been handled
            SerialMon->printf("[HTTPS] POST... code: %d\n", httpCode);

            // file found at server
            if (httpCode == 200) {
              String responseBody = httpsClient->http->responseBody();
              SerialMon->println(responseBody);
            }
          } else {
            SerialMon->printf("[HTTPS] POST... failed, response code: %d\n", httpCode);
          }

          httpsClient->http->stop();
        }

    // } else {
        // SerialMon->println("Invalid GPS data");
    // }

}