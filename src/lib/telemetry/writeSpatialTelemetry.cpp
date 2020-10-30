#include "writeSpatialTelemetry.h"

const char resource[] = "/spatial-telemetry-receive";
const char contentType[] = "binary/octet-stream";

// Voltage monitoring


bool writeSpatialTelemetry(HttpsClient *httpsClient, gps_fix *fix, float current, float voltage, HardwareSerial *SerialMon, SoftwareSerial *SerialAT) {

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

        // SerialMon->print("Lat var: ");
        // SerialMon->printf("%.6f\n", lat);

        // SerialMon->print("Lng var: ");
        // SerialMon->printf("%.6f\n\n", lng);

        int gsmStrength = httpsClient->GetGsmStrength();

        telemetry.deviceId = deviceId;
        telemetry.timestamp = timestamp;
        telemetry.latitude = lat;
        telemetry.longitude = lng;
        telemetry.gsmStrength = gsmStrength;
        telemetry.voltage = voltage;
        telemetry.current = current;


        status = pb_encode(&stream, SpatialTelemetry_fields, &telemetry);
        message_length = stream.bytes_written;

        if (!status) {
            SerialMon->printf("Encoding failed: %s\n", PB_GET_ERROR(&stream));
            return false;
        }

        SerialMon->print("[HTTPS] begin...\n");

        httpsClient->http->connectionKeepAlive();

        SerialMon->print("[HTTPS] POST...\n");
        int err = httpsClient->http->post(resource, contentType, message_length, buffer);
        SerialMon->printf("[HTTPS] POST message_length: %d\n", message_length);

        if (err != 0) {
          
            SerialMon->println(F("[HTTPS] POST - failed to connect"));
            SerialMon->println(err);
            return false;

        } else {
            SerialMon->println("[HTTPS] POST - connected, reading response ...");
            int httpCode = httpsClient->http->responseStatusCode();
            int responseSize = httpsClient->http->contentLength();
            SerialMon->printf("[HTTPS] POST - code: %d\n", httpCode);
            // httpCode will be negative on error
            if (httpCode > 0) {
                // HTTP header has been send and Server response header has been handled
                // String responseBody = httpsClient->http->responseBody();
                // SerialMon->printf("[HTTPS] POST - response body: %s\n", responseBody);
                httpsClient->http->stop();

                if (httpCode == 200) {
                  return true;
                }

            } else {
                SerialMon->printf("[HTTPS] POST - failed, response code: %d\n", httpCode);
                httpsClient->http->stop();   
            }

            return false;
        }

    // } else {
        // SerialMon->println("Invalid GPS data");
    // }

}