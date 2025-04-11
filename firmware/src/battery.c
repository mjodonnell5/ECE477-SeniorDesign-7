#include "stm32l496xx.h"
#include <stdio.h>
#include "battery.h"


// Function to read battery state of charge (SOC) from the BQ27441
uint16_t BQ27441_ReadSOC(void) {
    return BQ27441_ReadWord(BQ27441_SOC_REG);
}

// I2C communication functions

void I2C1_Start(uint32_t targadr, uint8_t size, uint8_t dir)
{
    uint32_t tmpreg = I2C4->CR2;
    tmpreg &= ~(I2C_CR2_SADD | I2C_CR2_NBYTES | I2C_CR2_RD_WRN | I2C_CR2_START | I2C_CR2_STOP);

    // Set read/write direction.
    if (dir == 1) tmpreg |= I2C_CR2_RD_WRN; // Read from slave
    else tmpreg &= ~I2C_CR2_RD_WRN; // Write to slave

    // Set the target's address and the data size.
    tmpreg |= ((targadr << 1) & I2C_CR2_SADD) | ((size << 16) & I2C_CR2_NBYTES);
    tmpreg |= I2C_CR2_START;

    // Start conversion.
    I2C4->CR2 = tmpreg;
}

void I2C1_Stop(void)
{
    I2C4->CR2 |= I2C_CR2_STOP;
    while (I2C4->ISR & I2C_ISR_BUSY); // Wait for STOP condition to complete
}

void I2C1_Write(uint8_t data)
{
    while (!(I2C4->ISR & I2C_ISR_TXE)); // Wait for TXE flag (Transmit Data Register empty)
    I2C4->TXDR = data; // Write data to TXDR
}

uint8_t I2C1_Read(uint8_t ack)
{
    while (!(I2C4->ISR & I2C_ISR_RXNE)); // Wait for RXNE flag (Receive Data Register not empty)
    uint8_t data = I2C4->RXDR; // Read data from RXDR
    if (ack) I2C4->CR2 |= I2C_CR2_NACK; // Send NACK if ack = 0
    return data;
}

void i2c_waitidle(void)
{
    while ((I2C4->ISR & I2C_ISR_BUSY) == I2C_ISR_BUSY); // while busy, wait.
}

int8_t i2c_senddata(uint8_t targadr, uint8_t data[], uint8_t size)
{
    i2c_waitidle();
    I2C1_Start(targadr, size, 0); // Last argument is dir: 0 = sending data to the slave device
    for (int i = 0; i < size; i++) {
        int count = 0;
        while ((I2C4->ISR & I2C_ISR_TXIS) == 0) { // Wait for TXIS flag
            count++;
            if (count > 1000000) return -1;
            if (i2c_checknack()) { // Check for NACK
                i2c_clearnack();
                I2C1_Stop();
                return -1;
            }
        }
        I2C4->TXDR = data[i] & I2C_TXDR_TXDATA; // Write data to TXDR
    }

    while ((I2C4->ISR & I2C_ISR_TC) == 0 && (I2C4->ISR & I2C_ISR_NACKF) == 0); // Wait for TC flag or NACK
    if ((I2C4->ISR & I2C_ISR_NACKF) != 0) return -1;
    I2C1_Stop();
    return 0;
}

int i2c_recvdata(uint8_t targadr, void *data, uint8_t size)
{
    if (size <= 0 || data == 0) return -1;

    uint8_t *udata = (uint8_t*)data;
    i2c_waitidle();
    I2C1_Start(targadr, size, 1); // Last argument is dir: 1 = receiving data from the slave device

    for (int i = 0; i < size; i++) {
        int count = 0;
        while ((I2C4->ISR & I2C_ISR_RXNE) == 0) { // Wait for RXNE flag
            count++;
            if (count > 1000000) return -1;
            if (i2c_checknack()) {
                i2c_clearnack();
                I2C1_Stop();
                return -1;
            }
        }
        udata[i] = I2C4->RXDR; // Read data from RXDR
    }

    while ((I2C4->ISR & I2C_ISR_TC) == 0 && (I2C4->ISR & I2C_ISR_NACKF) == 0); // Wait for TC or NACK
    if ((I2C4->ISR & I2C_ISR_NACKF) != 0) return -1;

    I2C1_Stop(); // Remove if autoend=1
    return 0;
}

void i2c_clearnack(void)
{
    I2C4->ICR |= I2C_ICR_NACKCF; // Clear NACK flag
}

int i2c_checknack(void)
{
    return ((I2C4->ISR & I2C_ISR_NACKF) == I2C_ISR_NACKF); // Check if NACK flag is set
}

// BQ27441 functions

uint16_t BQ27441_ReadWord(uint8_t reg)
{
    uint8_t data[2];
    i2c_waitidle();
    I2C1_Start(BQ27441_I2C_ADDRESS, 1, 0); // Send register address to the BQ27441
    i2c_senddata(BQ27441_I2C_ADDRESS, &reg, 1); // Send register address
    I2C1_Start(BQ27441_I2C_ADDRESS, 2, 1); // Re-start for reading
    i2c_recvdata(BQ27441_I2C_ADDRESS, data, 2); // Read 2 bytes of data
    return (data[1] << 8) | data[0]; // Combine data into 16-bit value
}

// GPIO initialization for I2C pins
void GPIO_Init(void)
{
    RCC->AHB2ENR |= RCC_AHB2ENR_GPIODEN; // Enable GPIOD clock
    GPIOD->MODER &= ~((3 << (12 * 2)) | (3 << (13 * 2))); // Clear mode bits
    GPIOD->MODER |= (2 << (12 * 2)) | (2 << (13 * 2));    // Set PD12 & PD13 to alternate function mode
    GPIOD->OTYPER |= (1 << 12) | (1 << 13); // Open-drain output type
    GPIOD->OSPEEDR |= (3 << (12 * 2)) | (3 << (13 * 2)); // High speed
    GPIOD->PUPDR &= ~((3 << (12 * 2)) | (3 << (13 * 2))); // Clear pull-up/pull-down bits
    GPIOD->PUPDR |= (1 << (12 * 2)) | (1 << (13 * 2));    // Enable pull-up resistors
    // GPIOD->AFR[1] |= (4 << ((12-8) * 4)) | (4 << ((13-8) * 4)); // AF4 for I2C on PD12 and PD13
    GPIOD->AFR[1] |= (4 << GPIO_AFRH_AFSEL12_Pos) | (4 << GPIO_AFRH_AFSEL13_Pos);
}

// I2C initialization function
void I2C1_Init(void)
{
    RCC->APB1ENR2 |= RCC_APB1ENR2_I2C4EN; // Enable I2C1 clock
    I2C4->TIMINGR = 0x20404768;           // Timing register setup for I2C
    I2C4->CR1 |= I2C_CR1_PE;              // Enable I2C1 peripheral
}
