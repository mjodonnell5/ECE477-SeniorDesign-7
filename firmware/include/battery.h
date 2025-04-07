#ifndef BATTERY
#define BATTERY


#include <stm32l496xx.h>
#include <stdint.h>
#include <stddef.h>

#define BQ27441_I2C_ADDRESS  0x55
#define BQ27441_COMMAND_VOLTAGE        0x04
#define BQ27441_COMMAND_SOC            0x1C
#define BQ27441_COMMAND_REM_CAPACITY   0x0C
#define BQ27441_SOC_REG       0x2C // State of charge register


void GPIO_Init(void);
void I2C1_Init(void);
uint16_t BQ27441_ReadWord(uint8_t reg);
void I2C1_Start(uint32_t targadr, uint8_t size, uint8_t dir);
void I2C1_Stop(void);
void I2C1_Write(uint8_t data);
uint8_t I2C1_Read(uint8_t ack);
void i2c_waitidle(void);
int8_t i2c_senddata(uint8_t targadr, uint8_t data[], uint8_t size);
int i2c_recvdata(uint8_t targadr, void *data, uint8_t size);
void i2c_clearnack(void);
int i2c_checknack(void);
#endif
