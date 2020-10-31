#include "writeSpatialTelemetry.h"

const char resource[] = "/spatial-telemetry-receive";
const char contentType[] = "binary/octet-stream";

bool writeSpatialTelemetry(HttpsClient *httpsClient, gps_fix *fix, float current, float voltage, HardwareSerial *SerialMon, SoftwareSerial *SerialAT) {

    uint8_t buffer[128];
    size_t message_length;
    bool status;

    SpatialTelemetry telemetry = SpatialTelemetry_init_zero;
    pb_ostream_t stream = pb_ostream_from_buffer(buffer, sizeof(buffer));

    int deviceId = 1;
    int timestamp = 1573153482;

    float lat = fix->latitude();
    float lng = fix->longitude();

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

    SerialMon->print("[HTTPS] POST...\n");
    int err = httpsClient->http->post(resource, contentType, message_length, buffer);
    SerialMon->printf("[HTTPS] POST message_length: %d\n", message_length);

    if (err != 0) {
        
        SerialMon->println(F("[HTTPS] POST - failed to connect"));
        SerialMon->println(err);
        httpsClient->http->stop();
        return false;

    } else {
        httpsClient->http->stop();
        return true;
    }

}