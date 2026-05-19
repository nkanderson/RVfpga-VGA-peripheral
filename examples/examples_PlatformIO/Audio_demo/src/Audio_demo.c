#include <stdint.h>

#define GPIO_SWs 0x80001400u
#define AUDIO_BASE 0x80004000u
#define AUDIO_CTRL (*(volatile uint32_t *)(AUDIO_BASE + 0x00))
#define AUDIO_NOTE (*(volatile uint32_t *)(AUDIO_BASE + 0x04))

#define AUDIO_EN (1u << 0)
#define AUDIO_VOL(v) (((v) & 0xF) << 4)
#define NOTE_ON (1u << 8)

#define GPIO_OE (*(volatile uint32_t *)0x80001408u)
#define GPIO_LEDs (*(volatile uint32_t *)0x80001404u)

int main(void) {
  AUDIO_CTRL = AUDIO_EN | AUDIO_VOL(3); // amp on, volume 3, once

  GPIO_OE = 0x0000FFFFu;

  while (1) {
    GPIO_LEDs = (*(volatile uint32_t *)GPIO_SWs >> 16) & 0xFF;
    uint32_t sw = (*(volatile uint32_t *)GPIO_SWs >> 16) & 0xFF;
    if (sw == 0) {
      AUDIO_NOTE = 0; // silence
    } else {
      uint32_t idx = 0, tmp = sw;
      while ((tmp & 1) == 0) {
        tmp >>= 1;
        idx++;
      } // find lowest set bit
      AUDIO_NOTE = NOTE_ON | (idx & 0xF);
    }
  }
  return 0;
}
