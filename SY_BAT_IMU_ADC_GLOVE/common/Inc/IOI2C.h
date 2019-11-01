#ifndef __IOI2C_H
#define __IOI2C_H
#include "stm32f1xx_hal.h"

//IO口操作宏定义
//#define BITBAND(addr, bitnum) ((addr & 0xF0000000)+0x2000000+((addr &0xFFFFF)<<5)+(bitnum<<2))
//#define MEM_ADDR(addr)  *((volatile unsigned long  *)(addr))
//#define BIT_ADDR(addr, bitnum)   MEM_ADDR(BITBAND(addr, bitnum))

#define MCP4728_DEVICE_ADDR 0xC0
#define MCP4728_VREF 4096

typedef struct{
	uint8_t addr;
	uint16_t dac[4];
}MCP4728_TypeDef;

unsigned char IIC_ReadOneByte(uint8_t IO, unsigned char I2C_Addr,unsigned char addr);
unsigned char IICwriteByte(uint8_t IO, unsigned char dev, unsigned char reg, unsigned char data);
uint8_t IICwriteBytes(uint8_t IO, uint8_t dev, uint8_t reg, uint8_t length, uint8_t* data);
uint8_t IICwriteBit(uint8_t IO, uint8_t dev,uint8_t reg,uint8_t bitNum,uint8_t data);
uint8_t IICreadBytes(uint8_t IO, uint8_t SlaveAddress,uint8_t REG_Address,uint8_t len,uint8_t *data);


uint8_t MCP4728FastWrite(uint8_t IO, MCP4728_TypeDef *dac);
uint8_t MCP4728WriteVref(uint8_t IO, MCP4728_TypeDef *dac, uint8_t type);
void I2cSdaWritePinHgih(uint8_t IO);
void I2cSdaWritePinLow(uint8_t IO);
void I2cSclWritePinHgih(uint8_t IO);
void I2cSclWritePinLow(uint8_t IO);
GPIO_PinState I2cSdaReadPin(uint8_t IO);
GPIO_PinState I2cSclReadPin(uint8_t IO);


#endif

//------------------End of File----------------------------
