#include "writeSpatialTelemetry.h"

const char resource[] = "/spatial-telemetry-receive";
const char contentType[] = "binary/octet-stream";

void writeSpatialTelemetry(HttpsClient *httpsClient, TinyGPSPlus *gps, HardwareSerial *SerialMon, HardwareSerial *SerialAT) {

    SerialMon->println("About to write spatial telemetry...");
    if (gps->location.isValid()) {

        uint8_t buffer[128];
        size_t message_length;
        bool status;

        SpatialTelemetry telemetry = SpatialTelemetry_init_zero;
        pb_ostream_t stream = pb_ostream_from_buffer(buffer, sizeof(buffer));

        int32 deviceId = 1;
        int32 timestamp = 1573153482;

        SerialMon->print("Lat: ");
        SerialMon->println(gps->location.lat());

        SerialMon->print("Lng: ");
        SerialMon->println(gps->location.lng());

        float lat = gps->location.lat();
        float lon = gps->location.lng();
        int32 gsmStrength = 25;

        telemetry.deviceId = deviceId;
        telemetry.timestamp = timestamp;
        telemetry.latitude = lat;
        telemetry.longitude = lon;
        telemetry.gsmStrength = gsmStrength;


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

    } else {
        SerialMon->println("Invalid GPS data");
    }

}