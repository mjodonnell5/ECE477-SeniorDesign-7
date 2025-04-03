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

/* Any EXTI line setup for interrupts will wakeup from stop 2 mode */
void enter_stop_2()
{
    PWR->CR1 &= ~PWR_CR1_LPMS;
    PWR->CR1 |= PWR_CR1_LPMS_STOP2;

    SCB->SCR |= SCB_SCR_SLEEPDEEP_Msk;

    __WFI();
}

/* This will be falling edge of power switch interrupt */
void power_off()
{
    /* Disable button interrupts; we ONLY want the power switch to be able to wakeup */
    disable_buttons();

    eink_sleep();

    /* FIXME: Turn off clocks to everything? */

    enter_stop_2();
}

/* This will be rising edge of power switch interrupt */
void wake_up()
{
    /* Upon wake up we just want to reset so that we can run the main function 
     * again, we could just call all the functions again but this is simpler I think*/
    NVIC_SystemReset();
}

int main(void)
{
    clock_init();

    spi1_init();

    eink_init();

    button_init();

    uart_init();

    mount();

    render_pending = 1;
    state_machine();

    eink_sleep();

    for (;;);
    return 0;
}
