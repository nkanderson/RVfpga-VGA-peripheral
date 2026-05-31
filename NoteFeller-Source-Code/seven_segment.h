#ifndef SEVEN_SEGMENT_H
#define SEVEN_SEGMENT_H

#include <stdint.h>

#define SEVENSEG_ENABLES_REG (*(volatile uint32_t *)0x80001038UL)
#define SEVENSEG_DIGITS_REG  (*(volatile uint32_t *)0x8000103CUL)

void sevenseg_init(void);
void sevenseg_clear(void);
void sevenseg_display_hex(uint32_t value);
void sevenseg_display_score(uint32_t score);
void sevenseg_set_raw(uint8_t enables, uint32_t digits);

#endif