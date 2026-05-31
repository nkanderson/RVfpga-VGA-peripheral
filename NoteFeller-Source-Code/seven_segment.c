////////////////////////////////////////////////////////////////////////////////
// Engineer: Chris Kane-Pardy
// Create Date: 05/31/2026
// File Name: seven_segment.c
// Project Name: Note Feller
//
// Description:
//   Software API for the existing RVfpgaEL2 8-digit seven-segment display
//   controller. The controller is memory-mapped through the System Controller.
//
// Register Map:
//   0x80001038 : Enables register
//                bit = 0 enables digit, bit = 1 disables digit
//
//   0x8000103C : Digits register
//                each 4-bit nibble represents one hex digit
////////////////////////////////////////////////////////////////////////////////

#include "seven_segment.h"

void sevenseg_init(void)
{
    sevenseg_clear();
}

void sevenseg_clear(void)
{
    // Disable all 8 digits.
    SEVENSEG_ENABLES_REG = 0xFF;
    SEVENSEG_DIGITS_REG  = 0x00000000;
}

void sevenseg_set_raw(uint8_t enables, uint32_t digits)
{
    SEVENSEG_ENABLES_REG = enables;
    SEVENSEG_DIGITS_REG  = digits;
}

void sevenseg_display_hex(uint32_t value)
{
    // Enable all digits and display raw hex value.
    SEVENSEG_ENABLES_REG = 0x00;
    SEVENSEG_DIGITS_REG  = value;
}

void sevenseg_display_score(uint32_t score)
{
    uint32_t digits = 0;
    uint32_t place = 0;

    if (score > 99999999) {
        score = 99999999;
    }

    // Convert decimal score into packed BCD nibbles.
    while (score > 0 && place < 8) {
        digits |= (score % 10) << (place * 4);
        score /= 10;
        place++;
    }

    // Enable all digits for now.
    SEVENSEG_ENABLES_REG = 0x00;
    SEVENSEG_DIGITS_REG  = digits;
}