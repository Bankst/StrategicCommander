#ifndef _MCP23017_H_
#define _MCP23017_H_

#include <i2c_t3.h>

class MCP23017 {
  public:
    void begin(uint8_t addr);
    void begin(void);

    uint16_t readGPIOAB();

  private:
    uint8_t i2cAddr;
  
    uint8_t bitForPin(uint8_t pin);
    uint8_t regForPin(uint8_t pin, uint8_t portAaddr, uint8_t portBaddr);
  
    uint8_t readRegister(uint8_t addr);
    void writeRegister(uint8_t addr, uint8_t value);
};

#define MCP23017_ADDRESS 0x20

// registers
#define MCP23017_IODIRA 0x00
#define MCP23017_IPOLA 0x02
#define MCP23017_GPINTENA 0x04
#define MCP23017_DEFVALA 0x06
#define MCP23017_INTCONA 0x08
#define MCP23017_IOCONA 0x0A
#define MCP23017_GPPUA 0x0C
#define MCP23017_INTFA 0x0E
#define MCP23017_INTCAPA 0x10
#define MCP23017_GPIOA 0x12
#define MCP23017_OLATA 0x14


#define MCP23017_IODIRB 0x01
#define MCP23017_IPOLB 0x03
#define MCP23017_GPINTENB 0x05
#define MCP23017_DEFVALB 0x07
#define MCP23017_INTCONB 0x09
#define MCP23017_IOCONB 0x0B
#define MCP23017_GPPUB 0x0D
#define MCP23017_INTFB 0x0F
#define MCP23017_INTCAPB 0x11
#define MCP23017_GPIOB 0x13
#define MCP23017_OLATB 0x15

#define MCP23017_INT_ERR 255

#endif
