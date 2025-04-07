#include <stm32l496xx.h>
#include <stdint.h>
#include <stddef.h>

void uart_init(void) {
    // RCC->APB1ENR1 |= RCC_APB1ENR1_USART1EN; /* Enable USART2 Clock */
    RCC->APB2ENR |= RCC_APB2ENR_USART1EN; /* Enable USART1 Clock */
    RCC->AHB2ENR |= RCC_AHB2ENR_GPIOAEN; /* Enable GPIOA Clock */
    RCC->CCIPR |= RCC_CCIPR_USART1SEL_0; /* Use the System Clock for USART2 */

    //tx pin setup pa2 -- do not need this
    GPIOA->MODER &= ~GPIO_MODER_MODE10_Msk;
    GPIOA->MODER |= GPIO_MODER_MODE10_1; /* Enable Alt Function for PA_2 */
    // GPIOA->AFR[1] |= GPIO_AFRL_AFSEL10_2 | GPIO_AFRL_AFSEL10_1 | GPIO_AFRL_AFSEL10_0; /* Enable USART2_Tx for PA_2 */
    GPIOA->AFR[1] |= (7 << GPIO_AFRH_AFSEL10_Pos);

    // SystemCoreClockUpdate();  // Update system clock
    USART1->BRR = 16000000 / 9600;  /* 9600 baudrate */

    USART1->CR1 |= USART_CR1_RE; /* Enable Receiver */
    USART1->CR1 |= USART_CR1_TE; /* Enable Transmitter */
    USART1->CR1 |= USART_CR1_UE; /* Enable USART2 */

}


// Function to transmit a character via UART
char uart_read_char(void) {
    while (!(USART1->ISR & USART_ISR_RXNE));  // Wait until data is received
    return USART1->RDR;  // Read the received character
    
}


// do not need this
void uart_write_char(char ch) {
    USART1->TDR = ch;   // Load data into transmit data register
    while (!(USART1->ISR & USART_ISR_TXE));   // Wait until transmit data register is empty
}
