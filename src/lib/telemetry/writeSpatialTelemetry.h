#include <Arduino.h>
#include <SoftwareSerial.h>
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

bool writeSpatialTelemetry(HttpsClient *httpsClient, gps_fix *fix, float current, float voltage, HardwareSerial *SerialMon, SoftwareSerial *SerialAT);