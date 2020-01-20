#include <SoftwareSerial.h>
#include <TinyGPS++.h>
#include <SimpleTimer.h>

#include "HttpsClient.h"
#include "lib/writeSpatialTelemetry/writeSpatialTelemetry.h"

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
  writeSpatialTelemetry(&httpsClient, &gps, &SerialMon, &SerialAT);
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