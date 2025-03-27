#include <stdio.h>
#include <string.h>
#include <stm32l432xx.h>

#include "../include/eink.h"
#include "../include/spi.h"
#include "../include/clock.h"
#include "../include/ui.h"
#include "../include/buttons.h"
#include "../include/state.h"
#include "../include/uart.h"
#include "../include/commands.h"

/* NOTE: Any EXTI line setup for interrupts will wakeup from stop 2 mode */
/* FIXME: Should disable every button except for power switch so that only 
 * the power switch will trigger a wakeup. Also we want to perform a system
 * reset after waking up and we only want to have to do that in the power 
 * switch ISR */
void enter_stop_2()
{
    NVIC->ISER[0] &= ~((1 << EXTI9_5_IRQn) | (1 << EXTI0_IRQn) | (1 << EXTI1_IRQn));

    PWR->CR1 &= ~PWR_CR1_LPMS;
    PWR->CR1 |= PWR_CR1_LPMS_STOP2;

    SCB->SCR |= SCB_SCR_SLEEPDEEP_Msk;

    __WFI();
}

int main(void)
{
    clock_init();

    spi1_init();

    eink_init();

    button_init();

    uart_init();

    mount();

    // eink_clear(0xFF);
    // char test[280];
    // draw_header("TESTING");
    // snprintf(test, 280, "this is a test of the character count of the display. I am trying to figure out how many characters we can write before we will up the entire display. So I am just writing a lot text onto here to fill. Lets keep adding characters until we get to the point where it is too full :)");
    // draw_centered_string_wrapped(large_font, test, BLACK);
    // eink_render_framebuffer();

    render_pending = 1;
    state_machine();

    eink_sleep();

    for (;;);
    return 0;
}
