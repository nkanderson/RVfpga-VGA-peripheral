#include <stdint.h>
#include "input_controller.h"

#define GPIO_BASE       0x80001400UL
#define GPIO_OUTPUT     (*(volatile uint32_t *)(GPIO_BASE + 0x04))
#define GPIO_OUTPUT_EN  (*(volatile uint32_t *)(GPIO_BASE + 0x08))

#define LED_PASS        (1U << 15)
#define LED_FAIL        (1U << 14)

static void delay(volatile uint32_t count)
{
    while (count--) {
        __asm__ volatile ("nop");
    }
}

int main(void)
{
    GPIO_OUTPUT_EN = 0xFFFF;
    GPIO_OUTPUT = 0;

    input_init();

    while (1) {
        uint32_t status = input_get_status();
        uint32_t edges  = input_get_edges();

        /*
         * LED layout:
         * LEDs [4:0]   = current held button state
         * LEDs [12:8]  = newly detected button press edges
         * LED 15       = edge detected / pass indicator
         * LED 14       = should stay off unless something unexpected happens
         */


        GPIO_OUTPUT = (status & 0x1F) | ((edges & 0x1F) << 8);

        if (edges) {
            GPIO_OUTPUT |= LED_PASS;

            delay(200000);

            input_clear_edges(edges);

            delay(200000);

            if ((input_get_edges() & edges) != 0) {
                GPIO_OUTPUT |= LED_FAIL;
            }
        }
    }

    return 0;
}