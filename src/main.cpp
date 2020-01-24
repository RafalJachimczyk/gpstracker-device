#include <ESP8266WiFi.h>
#include <SoftwareSerial.h>
#include <SimpleTimer.h>

#define RX_PIN 13
#define TX_PIN 15

#include <NMEAGPS.h>
#include <GPSport.h>
#include <Streamers.h>

#include "HttpsClient.h"
#include "lib/telemetry/writeSpatialTelemetry.h"

static NMEAGPS  gps;
static gps_fix  fix;

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

void updateGPS() {
  //read data from GPS module
  
  while (gps.available( gpsPort )) {
    fix = gps.read();
    // doSomeWork();
  

    pinMode(A0, INPUT);
    raw = analogRead(A0);
    voltage = raw / 1023.0;
    voltage = voltage * 4.2;
    position.voltage = voltage;

    // byte gpsData = SerialGPS.read();
    // SerialMon.write(gpsData);
        // SerialMon.print("Lat: ");
        // SerialMon.printf("%.5f", gps.location.lat());

        // SerialMon.print("Lng: ");
        // SerialMon.printf("%.5f", gps.location.lng());
  }
}

// void printGPS(void* args) {

//         Position* locPos = (Position*)args;
//         SerialMon.print("Lat var: ");
//         SerialMon.printf("%.6f\n", locPos->lat);

//         SerialMon.print("Lng var: ");
//         SerialMon.printf("%.6f\n\n", locPos->lng);

//         SerialMon.print("Lat: ");
//         SerialMon.printf("%.6f\n", gps.location.lat());

//         SerialMon.print("Lng: ");
//         SerialMon.printf("%.6f\n\n", gps.location.lng());
// }

void writeSpatialTelemetryProxy(void* args) {

        Position* locPos = (Position*)args;

        SerialMon.print("Lat var: ");
        SerialMon.printf("%.6f\n", locPos->lat);

        SerialMon.print("Lng var: ");
        SerialMon.printf("%.6f\n\n", locPos->lng);

        SerialMon.print("Lat: ");
        SerialMon.printf("%.6f\n", fix.latitude());

        SerialMon.print("Lng: ");
        SerialMon.printf("%.6f\n\n", fix.longitude());


  httpsClient.ConnectNetwork(); 
  writeSpatialTelemetry(&httpsClient, &fix, &SerialMon, &SerialAT);
  httpsClient.Disconnect();
}

void setup() {

    WiFi.disconnect(); 
    WiFi.mode(WIFI_OFF);
    WiFi.forceSleepBegin();

/////////////
DEBUG_PORT.begin(9600);
  while (!DEBUG_PORT)
    ;

  // DEBUG_PORT.print( F("NMEA.INO: started\n") );
  // DEBUG_PORT.print( F("  fix object size = ") );
  // DEBUG_PORT.println( sizeof(gps.fix()) );
  // DEBUG_PORT.print( F("  gps object size = ") );
  // DEBUG_PORT.println( sizeof(gps) );
  // DEBUG_PORT.println( F("Looking for GPS device on " GPS_PORT_NAME) );

  #ifndef NMEAGPS_RECOGNIZE_ALL
    #error You must define NMEAGPS_RECOGNIZE_ALL in NMEAGPS_cfg.h!
  #endif

  #ifdef NMEAGPS_INTERRUPT_PROCESSING
    #error You must *NOT* define NMEAGPS_INTERRUPT_PROCESSING in NMEAGPS_cfg.h!
  #endif

  #if !defined( NMEAGPS_PARSE_GGA ) & !defined( NMEAGPS_PARSE_GLL ) & \
      !defined( NMEAGPS_PARSE_GSA ) & !defined( NMEAGPS_PARSE_GSV ) & \
      !defined( NMEAGPS_PARSE_RMC ) & !defined( NMEAGPS_PARSE_VTG ) & \
      !defined( NMEAGPS_PARSE_ZDA ) & !defined( NMEAGPS_PARSE_GST )

    DEBUG_PORT.println( F("\nWARNING: No NMEA sentences are enabled: no fix data will be displayed.") );

  #else
    if (gps.merging == NMEAGPS::NO_MERGING) {
      // DEBUG_PORT.print  ( F("\nWARNING: displaying data from ") );
      // DEBUG_PORT.print  ( gps.string_for( LAST_SENTENCE_IN_INTERVAL ) );
      // DEBUG_PORT.print  ( F(" sentences ONLY, and only if ") );
      // DEBUG_PORT.print  ( gps.string_for( LAST_SENTENCE_IN_INTERVAL ) );
      // DEBUG_PORT.println( F(" is enabled.\n"
      //                       "  Other sentences may be parsed, but their data will not be displayed.") );
    }
  #endif

  // DEBUG_PORT.print  ( F("\nGPS quiet time is assumed to begin after a ") );
  // DEBUG_PORT.print  ( gps.string_for( LAST_SENTENCE_IN_INTERVAL ) );
  // DEBUG_PORT.println( F(" sentence is received.\n"
  //                       "  You should confirm this with NMEAorder.ino\n") );

  trace_header( DEBUG_PORT );
  DEBUG_PORT.flush();

  gpsPort.begin( 9600 );
//////////
    

    // httpsClient.ConnectNetwork(); 

    // Initialize serial and wait for port to open:
    SerialMon.begin(9600);
    // Initialize GPS serial

    timer.setInterval(1000L, writeSpatialTelemetryProxy, (void *)&position);
    //timer.setInterval(1000L, printGPS, (void *)&position);
}

void loop() {
    timer.run(); // Initiates BlynkTimer
    updateGPS();
}