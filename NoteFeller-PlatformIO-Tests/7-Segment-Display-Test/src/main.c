#include <stdint.h>
#include "seven_segment.h"

static void delay(volatile uint32_t count)
{
    while (count--) {
        __asm__ volatile ("nop");
    }
}

int main(void)
{
    sevenseg_init();

    while (1) {
        sevenseg_display_hex(0x12345678);
        delay(1000000);

        sevenseg_display_hex(0x87654321);
        delay(1000000);

        sevenseg_display_score(0);
        delay(1000000);

        sevenseg_display_score(1234);
        delay(1000000);

        sevenseg_display_score(98765432);
        delay(1000000);

        sevenseg_clear();
        delay(1000000);
    }

    return 0;
}