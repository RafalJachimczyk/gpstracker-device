#include <Arduino.h>
#include <NMEAGPS.h>
#include <pb_common.h>
#include <pb_encode.h>

#include "HttpsClient.h"
#include "SpatialTelemetry.pb.h"

struct Position
{
  float lat;
  float lng;
  float voltage;
};

void writeSpatialTelemetry(HttpsClient *httpsClient, gps_fix *fix, HardwareSerial *SerialMon, HardwareSerial *SerialAT);