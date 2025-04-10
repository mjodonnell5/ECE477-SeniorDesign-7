#include <stm32l496xx.h>
#include <stdint.h>
#include <stddef.h>

/* For e-ink */

//        MOSI        MISO       CLK         CS
//OLD     PA7         PA6        PA5          PA4
//NEW   PB5(D11)    PB4(D12)    PB3(d13)     PB1(D6)



void init_spi1_slow() {
    RCC->AHB2ENR |= RCC_AHB2ENR_GPIOGEN; //enable gpiob

    GPIOG->MODER &= ~( GPIO_MODER_MODER10 | GPIO_MODER_MODER11 | GPIO_MODER_MODER9); // clear pb3,4,5
    GPIOG->MODER |= ( GPIO_MODER_MODER10_1 | GPIO_MODER_MODER11_1 | GPIO_MODER_MODER9_1); //set pb3,4,5 to af

    GPIOG->AFR[1] &= ~(GPIO_AFRH_AFSEL10 | GPIO_AFRH_AFSEL11 | GPIO_AFRH_AFSEL9);
    GPIOG->AFR[1] |= (GPIO_AFRH_AFSEL10_1 | GPIO_AFRH_AFSEL10_2
                    | GPIO_AFRH_AFSEL11_1 | GPIO_AFRH_AFSEL11_2
                    | GPIO_AFRH_AFSEL9_1 | GPIO_AFRH_AFSEL9_2);

    RCC->APB1ENR1 |= RCC_APB1ENR1_SPI3EN; //enable spi

    SPI3->CR1 &= ~SPI_CR1_SPE; //disable SPI to set up
    
    SPI3->CR1 |= SPI_CR1_MSTR; // master mode
    
    SPI3->CR1 |= SPI_CR1_BR;// slow speed
    SPI3->CR1 &= ~SPI_CR1_BR_0; //128
   
    SPI3->CR2 |= (SPI_CR2_DS_0 | SPI_CR2_DS_1 | SPI_CR2_DS_2); // 8-bit data
    SPI3->CR2 &= ~SPI_CR2_DS_3;
    
    SPI3->CR1 |= SPI_CR1_SSM; // software slave management
    
    SPI3->CR1 |= SPI_CR1_SSI; // internal slave select

    SPI3->CR1 &= ~SPI_CR1_RXONLY; //full duplex
    
    SPI3->CR2 |= SPI_CR2_FRXTH; // FIFO reception threshold
   
    SPI3->CR1 |= SPI_CR1_SPE; // SPI channel enable

}
void enable_sdcard() { //ACTIVE LOW
    GPIOG->BRR = GPIO_BRR_BR_12; //set pB1 low to enable sd card
}
void disable_sdcard() {
    GPIOG->BSRR = GPIO_BSRR_BS_12; //set pb1 low to enable sd card
}
void init_sdcard_io() {
    init_spi1_slow();
    RCC->AHB2ENR |= RCC_AHB2ENR_GPIOGEN; //configure pb2 as output
    GPIOG->MODER &= ~(GPIO_MODER_MODER12);
    GPIOG->MODER |= GPIO_MODER_MODER12_0;
    disable_sdcard();

}
void sdcard_io_high_speed() {
    SPI3->CR1 &= ~SPI_CR1_SPE;
    SPI3->CR1 &= ~SPI_CR1_BR;
    SPI3->CR1 |= SPI_CR1_BR_0;
    SPI3->CR1 |= SPI_CR1_SPE;
}
