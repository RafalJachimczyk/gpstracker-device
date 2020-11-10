#include <SimpleTimer.h>
#include <avr/wdt.h>
#include <Wire.h>
#include <Maxim_DS2782.h>

#define NMEAGPS_INTERRUPT_PROCESSING

#include <NMEAGPS.h>
#include <GPSport.h>
// #include <Streamers.h>
#include <SoftwareSerial.h>

int timerWriteSpatialTelemetryProxy;
int timerUpdateGpsStatusIndicators;


// Check configuration

#ifndef NMEAGPS_INTERRUPT_PROCESSING
  #error You must define NMEAGPS_INTERRUPT_PROCESSING in NMEAGPS_cfg.h!
#endif

#include "HttpsClient.h"
#include "lib/telemetry/writeSpatialTelemetry.h"

static NMEAGPS  gps;
gps_fix gpsFix;

SimpleTimer timer;

static const uint8_t i2c_address = 0x34;
static const float rsns_ohm = 0.1;
Maxim_DS2782 ds2782(&Wire, i2c_address, rsns_ohm, &SerialMon);

// Define the serial console for debug prints, if needed
// #define TINY_GSM_DEBUG SerialMon

// Set serial for debug console (to the Serial Monitor, default speed 115200)
#define SerialMon Serial
// Set serial for AT commands (to the module)
SoftwareSerial SerialAT(23, 22);

String server = "us-central1-spatial-telemetry.cloudfunctions.net";
const int  port = 443;

HttpsClient httpsClient(&server, port, &SerialMon, &SerialAT);

unsigned int raw = 0;

float lat = 0;
float lng = 0;
float voltage = 0.0;
float current = 0.0;

Position position {0,0};

volatile int counter;      // Count number of times ISR is called.
volatile int countmax = 10; // Arbitrarily selected 3 for this example.
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

bool modemRestart() {
    if(httpsClient.modemRestart()) {
      SerialMon.println("###################: Modem restarted!");
      return true;
    } else {
      SerialMon.println("###################: Failed restarting modem!");
      return false;
    }
}

bool isGpsFixValid() {
    return gpsFix.valid.location;
}

void updateGpsStatusIndicators() {
    SerialMon.printf("Is GPS fix valid: %s", isGpsFixValid() ? "true" : "false");
    SerialMon.println();

    if(isGpsFixValid()) {
      // SerialMon.println("Valid!");
      digitalWrite(PB4, HIGH);
    } else {
      // SerialMon.println("Invalid!");
      digitalWrite(PB4, LOW);
    }  
}

void writeSpatialTelemetryProxy(void* args) {

  if(isGpsFixValid()) {
    voltage = ds2782.readVoltage();
    current = ds2782.readCurrent();


    if(httpsClient.ConnectNetwork()) {
        SerialMon.println("###################: ConnectNetwork succeeded");

        if(writeSpatialTelemetry(&httpsClient, &gpsFix, current, voltage, &SerialMon, &SerialAT)) {
        SerialMon.println("###################: POST succeeded");
        // Clears the watchdog timer
        watchdogClear();
        } else {
        SerialMon.println("###################: POST failed");
        }

        httpsClient.Disconnect();
    } else {
        SerialMon.println("###################: ConnectNetwork failed");
        modemRestart();
    }
  }
}

void setup() {

  // Clears the watchdog reset registry flag to prevent reboot loop
  // has to be called immediately
  MCUSR = 0x00; //cleared for next reset detection
  wdt_disable();


 //Disable JTAG
  MCUCR = (1<<JTD);
  MCUCR = (1<<JTD);

  // enable Modem
  pinMode(20, OUTPUT);
  digitalWrite(20, HIGH);
  delay(1000);
  digitalWrite(20, LOW);
  delay(1000);

  // PB4 LED
  pinMode(PB4, OUTPUT);

  // GPS Port and interrupt setup
  gpsPort.attachInterrupt( GPSisr );
  gpsPort.begin(9600);
   
  // Initialize serial and wait for port to open:
  SerialMon.begin(9600);
  SerialAT.begin(2400);

  SerialMon.println("###################: Atmega644 started!");
  modemRestart();
  
  timerUpdateGpsStatusIndicators = timer.setTimeout(1500L, updateGpsStatusIndicators);
  timerUpdateGpsStatusIndicators = timer.setInterval(30000L, updateGpsStatusIndicators);

  timerWriteSpatialTelemetryProxy = timer.setInterval(60000L, writeSpatialTelemetryProxy, (void *)&position);

  watchdogEnable(); // set up watchdog timer in interrupt-only mode
}

void loop() {
    timer.run(); // Initiates Timer
    
    if(gps.available()) {
      if(gps.fix().valid.location && gps.fix().valid.time && gps.fix().valid.date) {
        gpsFix = gps.read();
      }
    }
}