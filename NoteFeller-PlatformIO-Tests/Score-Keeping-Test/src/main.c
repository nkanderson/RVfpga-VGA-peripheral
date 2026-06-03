#include <stdint.h>

#include "score.h"
#include "input_controller.h"

static void delay(volatile uint32_t count)
{
    while (count--) {
        __asm__ volatile ("nop");
    }
}

int main(void)
{
    input_init();
    score_init();

    while (1)
    {
        uint32_t input = input_poll_new_presses();

        // Any lane key = successful hit
        if (input & 0x0F)
        {
            score_register_hit();
        }

        // Start button = miss
        if (input & INPUT_LANE_4)
        {
            score_register_miss();
        }

        delay(10000);
    }

    return 0;
}