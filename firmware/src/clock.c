#include <stdint.h>

void delay_ms(uint32_t ms)
{
    uint32_t count;
    while (ms--) {
        count = 1000;  // Approximate count for 1 ms at 4 MHz
        while (count--) {
            __asm__("nop");  // Prevent compiler optimization
        }
    }
}
