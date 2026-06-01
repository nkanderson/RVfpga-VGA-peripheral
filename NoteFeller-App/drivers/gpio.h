#ifndef GPIO_H
#define GPIO_H

#include <stdint.h>

// GPIO Base Addresses
#define GPIO_BASE       0x80001400UL  // LEDs
#define GPIO2_BASE      0x80001600UL  // 7-Segment Display (Adjusted to avoid conflict)

// GPIO Registers (LEDs)
#define GPIO_OUTPUT     (*(volatile uint32_t *)(GPIO_BASE + 0x04))
#define GPIO_DIR        (*(volatile uint32_t *)(GPIO_BASE + 0x08))

// GPIO2 Registers (7-Segment Display)
#define GPIO2_OUTPUT    (*(volatile uint32_t *)(GPIO2_BASE + 0x00))  // Direct write to 7-segment
#define GPIO2_DIR       (*(volatile uint32_t *)(GPIO2_BASE + 0x04))  // Direction register

// 7-Segment Patterns (Common Cathode)
static const uint8_t SSEG_PATTERNS[16] = {
    0x3F,  // 0
    0x06,  // 1
    0x5B,  // 2
    0x4F,  // 3
    0x66,  // 4
    0x6D,  // 5
    0x7D,  // 6
    0x07,  // 7
    0x7F,  // 8
    0x6F,  // 9
    0x77,  // A
    0x7C,  // B
    0x39,  // C
    0x5E,  // D
    0x79,  // E
    0x71   // F
};

// Initialize GPIO
void gpio_init(void);

// Display a number on the 7-segment display
void gpio_display_number(uint32_t number);

#endif // GPIO_H