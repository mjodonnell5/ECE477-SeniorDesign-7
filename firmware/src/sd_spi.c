#include <stm32l432xx.h>
#include <stdint.h>
#include <stddef.h>

/* For e-ink */

//        MOSI        MISO       CLK         CS
//OLD     PA7         PA6        PA5          PA4
//NEW   PB5(D11)    PB4(D12)    PB3(d13)     PB1(D6)



void init_spi1_slow() {
    // RCC->AHB2ENR |= RCC_AHB2ENR_GPIOAEN; //enable gpiob
    RCC->AHB2ENR |= RCC_AHB2ENR_GPIOBEN; //enable gpiob

    // GPIOA->MODER &= ~( GPIO_MODER_MODER5 | GPIO_MODER_MODER6 | GPIO_MODER_MODER7); // clear pb3,4,5
    // GPIOA->MODER |= ( GPIO_MODER_MODER5_1 | GPIO_MODER_MODER6_1 | GPIO_MODER_MODER7_1); //set pb3,4,5 to af
    
    GPIOB->MODER &= ~( GPIO_MODER_MODER3 | GPIO_MODER_MODER4 | GPIO_MODER_MODER5); // clear pb3,4,5
    GPIOB->MODER |= ( GPIO_MODER_MODER3_1 | GPIO_MODER_MODER4_1 | GPIO_MODER_MODER5_1); //set pb3,4,5 to af

    // GPIOA->AFR[0] &= ~(GPIO_AFRL_AFSEL5 | GPIO_AFRL_AFSEL6 | GPIO_AFRL_AFSEL7);
    // GPIOA->AFR[0] |= (GPIO_AFRL_AFSEL5_0 | GPIO_AFRL_AFSEL5_2
    //                 | GPIO_AFRL_AFSEL6_0 | GPIO_AFRL_AFSEL6_2
    //                 | GPIO_AFRL_AFSEL7_0 | GPIO_AFRL_AFSEL7_2);

    GPIOB->AFR[0] &= ~(GPIO_AFRL_AFSEL3 | GPIO_AFRL_AFSEL4 | GPIO_AFRL_AFSEL5);
    GPIOB->AFR[0] |= (GPIO_AFRL_AFSEL3_1 | GPIO_AFRL_AFSEL3_2
                    | GPIO_AFRL_AFSEL4_1 | GPIO_AFRL_AFSEL4_2
                    | GPIO_AFRL_AFSEL5_1 | GPIO_AFRL_AFSEL5_2);

    // RCC->APB2ENR |= RCC_APB2ENR_SPI1EN; //enable spi
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
    // GPIOA->BRR = GPIO_BRR_BR_4; //set pa4 low to enable sd card
    GPIOB->BRR = GPIO_BRR_BR_1; //set pB1 low to enable sd card
}
void disable_sdcard() {
    // GPIOA->BSRR = GPIO_BSRR_BS_4; //set pa4 high to disable sd card
    GPIOB->BSRR = GPIO_BSRR_BS_1; //set pb1 low to enable sd card
}
void init_sdcard_io() {
    init_spi1_slow();
    RCC->AHB2ENR |= RCC_AHB2ENR_GPIOBEN; //configure pb2 as output
    GPIOB->MODER &= ~(GPIO_MODER_MODER1);
    GPIOB->MODER |= GPIO_MODER_MODER1_0;
    disable_sdcard();

}
void sdcard_io_high_speed() {

    SPI3 -> CR1 &= ~SPI_CR1_SPE;
    SPI3 -> CR1 &= ~SPI_CR1_BR;
    SPI3 -> CR1 |= SPI_CR1_BR_0;
    SPI3 -> CR1 |= SPI_CR1_SPE;
}
