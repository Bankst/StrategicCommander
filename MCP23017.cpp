#include "MCP23017.h"

/**
 * Bit number associated to a give Pin
 */
uint8_t MCP23017::bitForPin(uint8_t pin){
  return pin%8;
}

/**
 * Register address, port dependent, for a given PIN
 */
uint8_t MCP23017::regForPin(uint8_t pin, uint8_t portAaddr, uint8_t portBaddr){
  return(pin<8) ?portAaddr:portBaddr;
}

/**
 * Reads a given register
 */
uint8_t MCP23017::readRegister(uint8_t addr){
  // read the current GPINTEN
  Wire.beginTransmission(i2cAddr);
  Wire.write(addr);
  Wire.endTransmission();
  Wire.requestFrom(i2cAddr, 1);
  return Wire.read();
}

uint16_t MCP23017::readGPIOAB() {
  uint16_t ba = 0;
  uint8_t a;

  // read the current GPIO output latches
  Wire.beginTransmission(i2cAddr);
  Wire.write(MCP23017_GPIOA);
  Wire.endTransmission();

  Wire.requestFrom(i2cAddr, 2);
  a = Wire.read();
  ba = Wire.read();
  ba <<= 8;
  ba |= a;
  return ~ba;
}

/**
 * Writes a given register
 */
void MCP23017::writeRegister(uint8_t regAddr, uint8_t regValue){
  // Write the register
  Wire.beginTransmission(i2cAddr);
  Wire.write(regAddr);
  Wire.write(regValue);
  Wire.endTransmission();
}

void MCP23017::begin(uint8_t addr) {
  i2cAddr = addr;

  Wire.begin();

  // set defaults!
  // all inputs on port A and B
  writeRegister(MCP23017_IODIRA, 0xFF);
  writeRegister(MCP23017_IODIRB, 0xFF);

  // all inputs pull-up
  writeRegister(MCP23017_GPPUA, 0xFF);
  writeRegister(MCP23017_GPPUB, 0xFF);
}

void MCP23017::begin(void) {
  begin(MCP23017_ADDRESS);
}
