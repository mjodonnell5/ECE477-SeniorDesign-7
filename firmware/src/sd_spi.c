#include <stm32l496xx.h>
#include <stdint.h>
#include <stddef.h>

/* For e-ink */

//        MOSI        MISO       CLK         CS
//OLD     PA7         PA6        PA5          PA4
//NEW   PB5(D11)    PB4(D12)    PB3(d13)     PB1(D6)



void init_spi1_slow() {
    RCC->AHB2ENR |= RCC_AHB2ENR_GPIODEN; //enable gpiob

    GPIOD->MODER &= ~( GPIO_MODER_MODER1 | GPIO_MODER_MODER3 | GPIO_MODER_MODER4); // clear pb3,4,5
    GPIOD->MODER |= ( GPIO_MODER_MODER1_1 | GPIO_MODER_MODER3_1 | GPIO_MODER_MODER4_1); //set pb3,4,5 to af

    GPIOD->AFR[0] &= ~(GPIO_AFRL_AFSEL1 | GPIO_AFRL_AFSEL3 | GPIO_AFRL_AFSEL4);
    GPIOD->AFR[0] |= (GPIO_AFRL_AFSEL1_0 | GPIO_AFRL_AFSEL1_2
                    | GPIO_AFRL_AFSEL3_0 | GPIO_AFRL_AFSEL3_2
                    | GPIO_AFRL_AFSEL4_0 | GPIO_AFRL_AFSEL4_2);

    RCC->APB1ENR1 |= RCC_APB1ENR1_SPI2EN; //enable spi

    SPI2->CR1 &= ~SPI_CR1_SPE; //disable SPI to set up
    
    SPI2->CR1 |= SPI_CR1_MSTR; // master mode
    
    SPI2->CR1 |= SPI_CR1_BR;// slow speed
    SPI2->CR1 &= ~SPI_CR1_BR_0; //128
   
    SPI2->CR2 |= (SPI_CR2_DS_0 | SPI_CR2_DS_1 | SPI_CR2_DS_2); // 8-bit data
    SPI2->CR2 &= ~SPI_CR2_DS_3;
    
    SPI2->CR1 |= SPI_CR1_SSM; // software slave management
    
    SPI2->CR1 |= SPI_CR1_SSI; // internal slave select

    SPI2->CR1 &= ~SPI_CR1_RXONLY; //full duplex
    
    SPI2->CR2 |= SPI_CR2_FRXTH; // FIFO reception threshold
   
    SPI2->CR1 |= SPI_CR1_SPE; // SPI channel enable

}
void enable_sdcard() { //ACTIVE LOW
    GPIOD->BRR = GPIO_BRR_BR_0; //set pB1 low to enable sd card
}
void disable_sdcard() {
    GPIOD->BSRR = GPIO_BSRR_BS_0; //set pb1 low to enable sd card
}
void init_sdcard_io() {
    init_spi1_slow();
    RCC->AHB2ENR |= RCC_AHB2ENR_GPIODEN; //configure pb2 as output
    GPIOD->MODER &= ~(GPIO_MODER_MODER0);
    GPIOD->MODER |= GPIO_MODER_MODER0_0;
    disable_sdcard();

}
void sdcard_io_high_speed() {

    SPI2 -> CR1 &= ~SPI_CR1_SPE;
    SPI2 -> CR1 &= ~SPI_CR1_BR;
    SPI2 -> CR1 |= SPI_CR1_BR_0;
    SPI2 -> CR1 |= SPI_CR1_SPE;
}
