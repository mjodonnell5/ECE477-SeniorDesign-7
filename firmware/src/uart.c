#include <stm32l432xx.h>
#include <stdint.h>
#include <stddef.h>

void uart_init(void) {
    RCC->APB1ENR1 |= RCC_APB1ENR1_USART2EN; /* Enable USART2 Clock */
    RCC->AHB2ENR |= RCC_AHB2ENR_GPIOAEN; /* Enable GPIOA Clock */
    RCC->CCIPR |= RCC_CCIPR_USART2SEL_0; /* Use the System Clock for USART2 */

    //rx pin setup pa3
    GPIOA->MODER &= ~GPIO_MODER_MODE15_Msk;
    GPIOA->MODER |= GPIO_MODER_MODE15_1; /* Enable Alt Function for PA_3 */
    // GPIOA->AFR[0] |= GPIO_AFRL_AFSEL3_2 | GPIO_AFRL_AFSEL3_1 | GPIO_AFRL_AFSEL3_0; //af7 for pa3
    // GPIOA->PUPDR |= (1 << GPIO_PUPDR_PUPD3_Pos); // Enable pull-up for PA3
    GPIOA->AFR[1]  |=  (3 << ((15 - 8) * 4));  // Set AF3 for PA15 (USART2_RX)


    //tx pin setup pa2
    GPIOA->MODER &= ~GPIO_MODER_MODE2_Msk;
    GPIOA->MODER |= GPIO_MODER_MODE2_1; /* Enable Alt Function for PA_2 */
    GPIOA->AFR[0] |= GPIO_AFRL_AFSEL2_2 | GPIO_AFRL_AFSEL2_1 | GPIO_AFRL_AFSEL2_0; /* Enable USART2_Tx for PA_2 */

    // SystemCoreClockUpdate();  // Update system clock
    USART2->BRR = 4000000 / 9600;  /* 9600 baudrate */

    USART2->CR1 |= USART_CR1_RE; /* Enable Receiver */
    USART2->CR1 |= USART_CR1_TE; /* Enable Transmitter */
    USART2->CR1 |= USART_CR1_UE; /* Enable USART2 */

}


// Function to transmit a character via UART
char uart_read_char(void) {
    while (!(USART2->ISR & USART_ISR_RXNE));  // Wait until data is received
    return USART2->RDR;  // Read the received character
    
}

void uart_write_char(char ch) {
    USART2->TDR = ch;   // Load data into transmit data register
    while (!(USART2->ISR & USART_ISR_TXE));   // Wait until transmit data register is empty
}
