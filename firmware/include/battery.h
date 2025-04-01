#ifndef BATTERY
#define BATTERY


#include <stm32l432xx.h>
#include <stdint.h>
#include <stddef.h>

#define BQ27441_I2C_ADDRESS  0x55
#define BQ27441_COMMAND_VOLTAGE        0x04
#define BQ27441_COMMAND_SOC            0x1C
#define BQ27441_COMMAND_REM_CAPACITY   0x0C

void GPIO_Init(void);
void I2C1_Init(void);
uint16_t BQ27441_ReadWord(uint8_t reg);
void I2C1_Start(void);
void I2C1_Stop(void);
void I2C1_Write(uint8_t data);
uint8_t I2C1_Read(uint8_t ack);

#endif
