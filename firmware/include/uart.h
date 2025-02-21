#include <stm32l432xx.h>
#include <stdint.h>
#include <stddef.h>


void UART_Init(void);
char UART_ReadChar(void);
void UART_WriteChar(char ch) ;