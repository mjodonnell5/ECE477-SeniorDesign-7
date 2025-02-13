#include <stm32l432xx.h>
#include "commands.h"

#define RX_BUFF_SIZE 128


void UART_Init(void);
char UART_ReadChar(void);

int main(void)
{
    mount();
    // log_to_sd();
    log_to_sd("testing UART \n");
    // char c = 'c';
    // char message[2] = {c, '\0'};  // Convert character to a string
    // log_to_sd(message);  // Log to SD card

    // parseJson("test.txt");
    UART_Init(); //initialize the UART module
    while(1){
        char i = UART_ReadChar();  // Read a single character
        char message[2] = {i, '\0'};  // Convert character to a string
        log_to_sd(message);  // Log to SD card

    }
    

    
    // cat("ABC.txt");
    // delete_file("zoerocks.txt");
    // ls();
    return 0;
}

//uart1 - rx - pb7
//
void uart_rx_tofile(){

    //enable usart 1

    //enable gpiob
    RCC->AHB2ENR |= RCC_AHB2ENR_GPIOBEN;

    //configure pb7

}

void UART_Init(void) {
    RCC->APB1ENR1 |= RCC_APB1ENR1_USART2EN; /* Enable USART2 Clock */
    RCC->AHB2ENR |= RCC_AHB2ENR_GPIOAEN; /* Enable GPIOA Clock */
    RCC->CCIPR |= RCC_CCIPR_USART2SEL_0; /* Use the System Clock for USART2 */

    GPIOA->MODER &= ~GPIO_MODER_MODE3_Msk;
    GPIOA->MODER |= GPIO_MODER_MODE3_1; /* Enable Alt Function for PA_3 */
    // GPIOA->AFR[0] = GPIO_AFRL_AFSEL3_2 | GPIO_AFRL_AFSEL3_1 | GPIO_AFRL_AFSEL3_0; /* Enable USART2_Tx for PA_3 */
    GPIOA->AFR[0] |= (7 << GPIO_AFRL_AFSEL3_Pos);

    USART2->BRR = 4000000 / 9600; /* 9600 baudrate */
    USART2->CR1 |= USART_CR1_RE; /* Enable Receiver */
    USART2->CR1 |= USART_CR1_UE; /* Enable USART2 */
}

// Function to transmit a character via UART
char UART_ReadChar(void) {
    while (!(USART2->ISR & USART_ISR_RXNE));  // Wait until data is received
    return USART2->RDR;  // Read the received character
}