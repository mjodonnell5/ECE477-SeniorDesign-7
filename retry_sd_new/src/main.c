#include <stm32l432xx.h>
#include "commands.h"
#include "uart.h"

#define RX_BUFF_SIZE 128


void UART_Init(void);
char UART_ReadChar(void);
void UART_WriteChar(char ch);

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
   
    while (1) {
        char i = UART_ReadChar();  // Read character from RX
        char message[2] = {i, '\0'};  // Convert character to a string
        log_to_sd(message);  // Log received data to SD card
        UART_WriteChar(i);         // Transmit same character back
    }
    

    
    // cat("ABC.txt");
    // delete_file("zoerocks.txt");
    // ls();
    return 0;
}

