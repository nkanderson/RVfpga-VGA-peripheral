////////////////////////////////////////////////////////////////////////////////
// Engineer: Niklas Anderson
// Create Date: 05/26/2026
// File Name: main.c
// Project Name: Note Feller
//
// Description:
// Exercises the audio driver API end-to-end and provides interactive control.
//
// Phase 1 - Ascending scale demo:
//   Plays each voice (C4..C5) one at a time, ~0.6 s each. Exercises
//   audio_set_voice() for individual on/off control. LED[v] lights for the
//   active voice; LED 15 indicates "playing".
//
// Phase 2 - C major triad with shifting per-voice balance:
//   Plays C4 + E4 + G4 as a chord with three different volume profiles
//   - first balanced (all max), then root-dominant (C loudest), then
//   top-dominant (G loudest). Demonstrates audio_set_voice_volume() for
//   selective adjustments and audio_set_all_volumes() for resetting in
//   one bulk write.
//
// Phase 3 - Interactive:
//   SW[7:0] each map to one voice (SW0 = C4 ... SW7 = C5). Flip multiple
//   switches simultaneously to play chords. LEDs[7:0] mirror the switch
//   state; LED 15 indicates any voice active.
//
//
// LLM Acknowledgment: This code was generated with the assistance of a
// language model, which provided the initial structure and logic.The
// final implementation was reviewed and edited by the author to ensure
// correctness and functionality on the target hardware platform.
////////////////////////////////////////////////////////////////////////////////

#include "audio.h"
#include <stdint.h>

// -----------------------------------------------------------------------------
// GPIO layout (Nexys A7):
//   GPIO_DATA[31:16]  = 16 slide switches
//   GPIO_OUTPUT[15:0] = 16 LEDs (driven when GPIO_OUTPUT_EN[15:0] is set)
// -----------------------------------------------------------------------------

#define GPIO_BASE 0x80001400UL
#define GPIO_DATA (*(volatile uint32_t *)(GPIO_BASE + 0x00))
#define GPIO_OUTPUT (*(volatile uint32_t *)(GPIO_BASE + 0x04))
#define GPIO_OUTPUT_EN (*(volatile uint32_t *)(GPIO_BASE + 0x08))

#define LED_PLAYING (1U << 15) // lit whenever any voice is active

static void delay(volatile uint32_t count) {
  while (count--) {
    __asm__ volatile("nop");
  }
}

int main(void) {
  GPIO_OUTPUT_EN = 0xFFFF;
  GPIO_OUTPUT = 0;

  audio_init(AUDIO_VOL_MAX);

  // ---------- Phase 1: ascending scale, one voice at a time ----------
  for (uint8_t v = 0; v < AUDIO_NUM_VOICES; v++) {
    audio_set_voice(v, 1);
    GPIO_OUTPUT = ((uint32_t)1U << v) | LED_PLAYING;
    delay(750000);
    audio_set_voice(v, 0);
  }
  delay(350000);

  // ---------- Phase 2: C major triad with shifting balance ----------
  const uint8_t triad =
      (1U << AUDIO_VOICE_C4) | (1U << AUDIO_VOICE_E4) | (1U << AUDIO_VOICE_G4);

  audio_set_voices(triad);
  GPIO_OUTPUT = triad | LED_PLAYING;
  delay(1300000); // balanced

  // Root-dominant: keep C4 at max, drop E4 and G4
  audio_set_voice_volume(AUDIO_VOICE_E4, 4);
  audio_set_voice_volume(AUDIO_VOICE_G4, 4);
  delay(1300000);

  // Top-dominant: drop C4, raise G4
  audio_set_voice_volume(AUDIO_VOICE_C4, 4);
  audio_set_voice_volume(AUDIO_VOICE_G4, AUDIO_VOL_MAX);
  delay(1300000);

  audio_silence();
  GPIO_OUTPUT = 0;

  // Reset all voices to max in one bulk write before interactive mode
  uint8_t flat[AUDIO_NUM_VOICES];
  for (uint8_t i = 0; i < AUDIO_NUM_VOICES; i++) {
    flat[i] = AUDIO_VOL_MAX;
  }
  audio_set_all_volumes(flat);
  delay(400000);

  // ---------- Phase 3: interactive (switches drive voices) ----------
  while (1) {
    uint8_t sw_mask = (uint8_t)((GPIO_DATA >> 16) & 0xFFu);
    audio_set_voices(sw_mask);
    GPIO_OUTPUT = (sw_mask != 0) ? ((uint32_t)sw_mask | LED_PLAYING) : 0;
  }

  return 0;
}
