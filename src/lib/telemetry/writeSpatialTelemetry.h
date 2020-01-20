#include <Arduino.h>
#include <TinyGPS++.h>
#include <pb_common.h>
#include <pb_encode.h>

#include "HttpsClient.h"
#include "SpatialTelemetry.pb.h"


void writeSpatialTelemetry(HttpsClient *httpsClient, TinyGPSPlus *gps, HardwareSerial *SerialMon, HardwareSerial *SerialAT);