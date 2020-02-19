#include <SimpleTimer.h>
#include <avr/wdt.h>

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

volatile int counter;      // Count number of times ISR is called.
volatile int countmax = 3; // Arbitrarily selected 3 for this example.
                          // Timer expires after about 24 secs if
                          // 8 sec interval is selected below.


void watchdogClear() {
  counter = 0;
  wdt_reset();
}

void watchdogEnable()
{
 counter=0;
 cli();                              // disable interrupts

 MCUSR = 0;                          // reset status register flags

                                     // Put timer in interrupt-only mode:                                        
 WDTCSR |= 0b00011000;               // Set WDCE (5th from left) and WDE (4th from left) to enter config mode,
                                     // using bitwise OR assignment (leaves other bits unchanged).
 WDTCSR =  0b01000000 | 0b100001;    // set WDIE (interrupt enable...7th from left, on left side of bar)
                                     // clr WDE (reset enable...4th from left)
                                     // and set delay interval (right side of bar) to 8 seconds,
                                     // using bitwise OR operator.

 sei();                              // re-enable interrupts
 //wdt_reset();                      // this is not needed...timer starts without it

 // delay interval patterns:
 //  16 ms:     0b000000
 //  500 ms:    0b000101
 //  1 second:  0b000110
 //  2 seconds: 0b000111
 //  4 seconds: 0b100000
 //  8 seconds: 0b100001

}

ISR(WDT_vect) // watchdog timer interrupt service routine
{
 counter+=1;

 if (counter < countmax)
 {
   wdt_reset(); // start timer again (still in interrupt-only mode)
 }
 else             // then change timer to reset-only mode with short (16 ms) fuse
 {
   
   MCUSR = 0;                          // reset flags

                                       // Put timer in reset-only mode:
   WDTCSR |= 0b00011000;               // Enter config mode.
   WDTCSR =  0b00001000 | 0b000000;    // clr WDIE (interrupt enable...7th from left)
                                       // set WDE (reset enable...4th from left), and set delay interval
                                       // reset system in 16 ms...
                                       // unless wdt_disable() in loop() is reached first

   //wdt_reset(); // not needed
 }
}


static void GPSisr( uint8_t c )
{
  gps.handle( c );

} // GPSisr

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
      // Clears the watchdog timer
      watchdogClear();
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

  // Clears the watchdog reset registry flag to prevent reboot loop
  // has to be called immediately
  MCUSR = 0x00; //cleared for next reset detection
  wdt_disable();

  // GPS Port and interrupt setup
  gpsPort.attachInterrupt( GPSisr );
  gpsPort.begin( 9600 );
   
  // Initialize serial and wait for port to open:
  SerialMon.begin(9600);
  Serial.begin(9600);

  // Used to connect in the setup
  // httpsClient.ConnectNetwork();     

  SerialMon.println("###################: Atmega644 started!");

  timer.setInterval(10000L, writeSpatialTelemetryProxy, (void *)&position);

}

void loop() {
    timer.run(); // Initiates BlynkTimer
    watchdogEnable(); // set up watchdog timer in interrupt-only mode
}