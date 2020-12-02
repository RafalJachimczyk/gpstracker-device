#include "Accelerometer.h"

Accelerometer::Accelerometer() {

};

byte Accelerometer::readRegister(byte reg) {
  Wire.beginTransmission(this->address);
  Wire.write(reg);
  Wire.endTransmission(false);

  Wire.requestFrom(address, (byte) 1);

  while(!Wire.available());
  return Wire.read();
}

void Accelerometer::writeRegister(byte reg, byte data) {
  Wire.beginTransmission(address);
  Wire.write(reg);
  Wire.write(data);
  Wire.endTransmission();
}


// Behaves like latch storing didMove into local temporary boolean variable.
// It resets the didMove variable instantly, and returns original value of
// didMove. For demo purpose getDidMove is called to output value of didMove 
// to a Serial Port and external Pin.
// In reality this could be used to control battery saving logic in embedded applications.
// An example would be to call getDidMove to make a decision if an internet connection 
// should be made to upload telemetry
bool Accelerometer::getDidMove() {
  bool tempDidMove = didMove;
  didMove = false;
  return tempDidMove;
}

// Makes use of latched didMove variable by calling getDidMove. 
// Updates LED and prints to serial console the status of motion. 
void Accelerometer::retrieveDidMove() {
  bool tempDidMove = getDidMove();
  digitalWrite(PB4, tempDidMove);
  Serial.printf("%s\n", tempDidMove ? "Moved" : "Not moved");
}

void Accelerometer::begin() {
  byte whoAmI = readRegister(0x0D);
  Serial.printf("Who Am I Byte: 0x%04x\n", whoAmI);

  // Put device into standby mode 0x18
  Serial.printf("CTRL_REG1 initial: 0x%04x\n", readRegister(CTRL_REG1));

  writeRegister(CTRL_REG1, readRegister(CTRL_REG1) & ~(0x01));

  Serial.printf("CTRL_REG1: 0x%04x\n", readRegister(CTRL_REG1));


  // Set configuration register for motion detection
  Serial.printf("FF_MT_CFG initial: 0x%04x\n", readRegister(FF_MT_CFG));

  byte cfg = 0x00; //latch not enabled
  cfg |= 0x40;
  cfg |= 0x08;
  cfg |= 0x10;
  cfg |= 0x20;


  writeRegister(FF_MT_CFG, cfg);

  Serial.printf("FF_MT_CFG: 0x%04x\n", readRegister(FF_MT_CFG));


  //Set threshold of > 3g
  Serial.printf("FF_MT_THS initial: 0x%04x\n", readRegister(FF_MT_THS));
  
  writeRegister(FF_MT_THS, 0x10);

  Serial.printf("FF_MT_THS: 0x%04x\n", readRegister(FF_MT_THS));

  //Set the debounce counter to eliminate false readings for 100 Hz sample rate with requirement of 100 ms timer
  Serial.printf("FF_MT_COUNT initial: 0x%04x\n", readRegister(FF_MT_COUNT));
  
  writeRegister(FF_MT_COUNT, 0x0A);

  Serial.printf("FF_MT_COUNT: 0x%04x\n", readRegister(FF_MT_COUNT));

  //Enable motion detection Interrupt function in CRTL_REG4
  Serial.printf("CTRL_REG4 initial: 0x%04x\n", readRegister(CTRL_REG4));
  
  writeRegister(CTRL_REG4, 0x04);

  Serial.printf("CTRL_REG4: 0x%04x\n", readRegister(CTRL_REG4));

  //Router Interrupt function to INT1 hardware interrupt pin
  Serial.printf("CTRL_REG5 initial: 0x%04x\n", readRegister(CTRL_REG5));
  
  writeRegister(CTRL_REG5, 0x04);

  Serial.printf("CTRL_REG5: 0x%04x\n", readRegister(CTRL_REG5));

  // Put device in Active mode
  byte ctrlReg1Data = readRegister(CTRL_REG1);
  ctrlReg1Data |= 0x01;
  writeRegister(CTRL_REG1, ctrlReg1Data | 0x01);
  Serial.printf("CTRL_REG1: 0x%04x\n", readRegister(CTRL_REG1));

};