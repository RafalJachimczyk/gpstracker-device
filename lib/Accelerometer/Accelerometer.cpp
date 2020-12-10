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

bool Accelerometer::getDidMove() {
  
  byte status = readRegister(FF_MT_SRC);

  // Serial.printf("INT_SOURCE: status: 0x%016x\n", status);
  return ((status) & (1<<(7)));
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

  byte cfg = 0x80; //latch enabled
  cfg |= 0x40;
  cfg |= 0x08;
  cfg |= 0x10;
  cfg |= 0x20;

  // 11111000

  writeRegister(FF_MT_CFG, cfg);

  Serial.printf("FF_MT_CFG: 0x%04x\n", readRegister(FF_MT_CFG));


  //Set threshold of > 1.008g (1.008/0.063)
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