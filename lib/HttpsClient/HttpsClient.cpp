#include "HttpsClient.h"

HttpsClient::HttpsClient(String* server, int port, HardwareSerial *SerialMon, HardwareSerial *SerialAT) {
  _SerialMon = SerialMon;
  _SerialAT = SerialAT;

  _modem = new TinyGsm(*_SerialAT);

  _client = new TinyGsmClientSecure(*_modem);
  
  http = new HttpClient(*_client, *server, port); 
}

void HttpsClient::ConnectNetwork() {

  _SerialMon->println("Wait...");

  // Set GSM module baud rate
  // TinyGsmAutoBaud(SerialAT,GSM_AUTOBAUD_MIN,GSM_AUTOBAUD_MAX);
  _SerialAT->begin(9600);
  delay(10);

  // Restart takes quite some time
  // To skip it, call init() instead of restart()

  uint32_t free = system_get_free_heap_size();
  _SerialMon->println("Hello World!");
  _SerialMon->printf("Free memory: %d", free);

  _SerialMon->println("Initializing modem...");
  delay(10);
  _modem->testAT();
  
  //_modem->init();
  delay(100);
  String modemInfo = _modem->getModemInfo();
  _SerialMon->print("Modem Info: ");
  _SerialMon->println(modemInfo);

  #if TINY_GSM_USE_GPRS
    // Unlock your SIM card with a PIN if needed
    if ( GSM_PIN && _modem->getSimStatus() != 3 ) {
      _modem->simUnlock(GSM_PIN);
    }
  #endif

  if (!_modem->hasSSL()) {
    _SerialMon->println(F("SSL is not supported by this modem"));
    while(true) { delay(1000); }
  }    

  #if TINY_GSM_USE_WIFI
    // Wifi connection parameters must be set before waiting for the network
    _SerialMon.print(F("Setting SSID/password..."));
    if (!_modem->networkConnect(wifiSSID, wifiPass)) {
      _SerialMon.println(" fail");
      delay(10000);
      return;
    }
    _SerialMon.println(" success");
  #endif

  #if TINY_GSM_USE_GPRS && defined TINY_GSM_MODEM_XBEE
    // The XBee must run the gprsConnect function BEFORE waiting for network!
    _modem->gprsConnect(apn, gprsUser, gprsPass);
  #endif

  _SerialMon->print("Waiting for network...");
  if (!_modem->waitForNetwork()) {
    _SerialMon->println(" fail");
    delay(10000);
    return;
  }
  _SerialMon->println(" success");

  if (_modem->isNetworkConnected()) {
    _SerialMon->println("Network connected");
  }

  #if TINY_GSM_USE_GPRS
    // GPRS connection parameters are usually set after network registration
      _SerialMon->print(F("Connecting to "));
      _SerialMon->print(apn);
      if (!_modem->gprsConnect(apn, gprsUser, gprsPass)) {
        _SerialMon->println(" fail");
        delay(10000);
        return;
      }
      _SerialMon->println(" success");

      if (_modem->isGprsConnected()) {
        _SerialMon->println("GPRS connected");
      }
  #endif
}

void HttpsClient::StopRequest() {
  http->stop();
  _SerialMon->println(F("Server disconnected"));
}

void HttpsClient::Disconnect() {
  #if TINY_GSM_USE_WIFI
    _modem.networkDisconnect();
    _SerialMon.println(F("WiFi disconnected"));
  #endif
  #if TINY_GSM_USE_GPRS
      _modem->gprsDisconnect();
      _SerialMon->println(F("GPRS disconnected"));
  #endif
}
