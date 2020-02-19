#include <SimpleTimer.h>

#include <NMEAGPS.h>
#include <GPSport.h>
#include <Streamers.h>

// Check configuration

#ifndef NMEAGPS_INTERRUPT_PROCESSING
  #error You must define NMEAGPS_INTERRUPT_PROCESSING in NMEAGPS_cfg.h!
#endif

#include "HttpsClient.h"
#include "lib/telemetry/writeSpatialTelemetry.h"

static NMEAGPS  gps;

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

unsigned int raw = 0;

float lat = 0;
float lng = 0;
float voltage = 0.0;

Position position {0,0};

//--------------------------

static void GPSisr( uint8_t c )
{
  gps.handle( c );

} // GPSisr

//--------------------------


void writeSpatialTelemetryProxy(void* args) {

  Position* locPos = (Position*)args;

  SerialMon.print("Lat var: ");
  SerialMon.printf("%.6f\n", locPos->lat);

  SerialMon.print("Lng var: ");
  SerialMon.printf("%.6f\n\n", locPos->lng);

  SerialMon.print("Lat: ");
  SerialMon.printf("%.6f\n", gps.fix().latitude());

  SerialMon.print("Lng: ");
  SerialMon.printf("%.6f\n\n", gps.fix().longitude());

  if(httpsClient.ConnectNetwork()) {
    SerialMon.println("###################: ConnectNetwork succeeded");

    if(writeSpatialTelemetry(&httpsClient, &gps.fix(), &SerialMon, &SerialAT)) {
      SerialMon.println("###################: POST succeeded");
    } else {
      SerialMon.println("###################: POST failed");
    }

    httpsClient.Disconnect();
  } else {
    SerialMon.println("###################: ConnectNetwork failed");
    if(httpsClient.modemRestart()) {
      SerialMon.println("###################: Modem restarted!");
    } else {
      SerialMon.println("###################: Failed restarting modem!");
    }
  }

}

void setup() {

/////////////
DEBUG_PORT.begin(9600);
  while (!DEBUG_PORT)
    ;

  DEBUG_PORT.print( F("NMEA_isr.INO: started\n") );
  DEBUG_PORT.print( F("fix object size = ") );
  DEBUG_PORT.println( sizeof(gps.fix()) );
  DEBUG_PORT.print( F("NMEAGPS object size = ") );
  DEBUG_PORT.println( sizeof(gps) );
  DEBUG_PORT.println( F("Looking for GPS device on " GPS_PORT_NAME) );

  trace_header( DEBUG_PORT );
  DEBUG_PORT.flush();

  gpsPort.attachInterrupt( GPSisr );
  gpsPort.begin( 9600 );
//////////
    

  // Initialize serial and wait for port to open:
  SerialMon.begin(9600);
  Serial.begin(9600);
  // httpsClient.ConnectNetwork();     

  timer.setInterval(10000L, writeSpatialTelemetryProxy, (void *)&position);

}

void loop() {
    timer.run(); // Initiates BlynkTimer
}