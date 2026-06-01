#include "gpio.h"

void gpio_init(void) {
    // Set all GPIO pins as outputs
    GPIO_DIR = 0xFFFF;      // LEDs
    GPIO2_DIR = 0xFFFF;     // 7-Segment Display

    // Clear all outputs
    GPIO_OUTPUT = 0;
    GPIO2_OUTPUT = 0;
}

void gpio_display_number(uint32_t number) {
    // Extract digits from the number
    uint8_t digit0 = number % 10;
    uint8_t digit1 = (number / 10) % 10;
    uint8_t digit2 = (number / 100) % 10;
    uint8_t digit3 = (number / 1000) % 10;

    // Pack digits into a 32-bit value (Nexys A7 uses 4 separate 8-bit digits)
    uint32_t sseg_pattern = 0;
    sseg_pattern |= (SSEG_PATTERNS[digit0] << 0);   // Digit 0 (rightmost)
    sseg_pattern |= (SSEG_PATTERNS[digit1] << 8);   // Digit 1
    sseg_pattern |= (SSEG_PATTERNS[digit2] << 16);  // Digit 2
    sseg_pattern |= (SSEG_PATTERNS[digit3] << 24);  // Digit 3 (leftmost)

    GPIO2_OUTPUT = sseg_pattern;  // Write to 7-segment display
}