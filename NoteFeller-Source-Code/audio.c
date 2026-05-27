////////////////////////////////////////////////////////////////////////////////
// Engineer: Niklas Anderson
// Create Date: 05/26/2026
// File Name: audio.c
// Project Name: Note Feller
//
// Description:
//   Software driver for the Note Feller wb_audio peripheral. This file
//   abstracts the memory-mapped audio registers behind a small API that
//   the game engine can call without depending on raw hardware addresses.
//
//   Provides initialization, per-voice on/off control, and per-voice
//   volume control. All multi-bit register fields are packed/unpacked
//   here so callers can work with simple uint8_t values.
//
// Register Map:
//   BASE + 0x00 : AUDIO_CTRL
//                 [0]   = amplifier enable (drives the on-board AUD_SD pin).
//                 [7:4] = reserved (writes preserved on readback, no audio
//                         effect; intended for a future master gain stage).
//
//   BASE + 0x04 : AUDIO_VOICES
//                 [7:0] = per-voice on/off mask. Bit i = voice i playing.
//
//   BASE + 0x08 : AUDIO_VOL
//                 Per-voice 4-bit volumes packed lowest voice first:
//                 [3:0]   = voice 0 volume (C4)
//                 [7:4]   = voice 1 volume (D4)
//                 [11:8]  = voice 2 volume (E4)
//                 [15:12] = voice 3 volume (F4)
//                 [19:16] = voice 4 volume (G4)
//                 [23:20] = voice 5 volume (A4)
//                 [27:24] = voice 6 volume (B4)
//                 [31:28] = voice 7 volume (C5)
//
// LLM Acknowledgment: This code was generated with the assistance of a
// language model, which provided the initial structure and logic.The
// final implementation was reviewed and edited by the author to ensure
// correctness and functionality on the target hardware platform.
////////////////////////////////////////////////////////////////////////////////

#include "audio.h"

#define AUDIO_BASE 0x80004000u
#define AUDIO_CTRL_REG (*(volatile uint32_t *)(AUDIO_BASE + 0x00))
#define AUDIO_VOICES_REG (*(volatile uint32_t *)(AUDIO_BASE + 0x04))
#define AUDIO_VOL_REG (*(volatile uint32_t *)(AUDIO_BASE + 0x08))

#define AUDIO_CTRL_EN (1u << 0)

static uint32_t pack_uniform_volume(uint8_t v) {
  uint32_t packed = 0;
  uint32_t nibble = v & 0xFu;
  for (int i = 0; i < AUDIO_NUM_VOICES; i++) {
    packed |= nibble << (i * 4);
  }
  return packed;
}

void audio_init(uint8_t default_voice_volume) {
  AUDIO_VOICES_REG = 0; // silent
  AUDIO_VOL_REG = pack_uniform_volume(default_voice_volume);
  AUDIO_CTRL_REG = AUDIO_CTRL_EN; // amp on
}

void audio_silence(void) { AUDIO_VOICES_REG = 0; }

void audio_set_voices(uint8_t voice_mask) { AUDIO_VOICES_REG = voice_mask; }

uint8_t audio_get_voices(void) { return (uint8_t)(AUDIO_VOICES_REG & 0xFFu); }

void audio_set_voice(uint8_t voice_idx, int on) {
  if (voice_idx >= AUDIO_NUM_VOICES)
    return;
  uint8_t mask = audio_get_voices();
  if (on)
    mask |= (uint8_t)(1u << voice_idx);
  else
    mask &= (uint8_t) ~(1u << voice_idx);
  AUDIO_VOICES_REG = mask;
}

void audio_set_voice_volume(uint8_t voice_idx, uint8_t volume) {
  if (voice_idx >= AUDIO_NUM_VOICES)
    return;
  uint32_t packed = AUDIO_VOL_REG;
  uint32_t shift = (uint32_t)voice_idx * 4u;
  uint32_t hole = 0xFu << shift;
  packed = (packed & ~hole) | (((uint32_t)volume & 0xFu) << shift);
  AUDIO_VOL_REG = packed;
}

void audio_set_all_volumes(const uint8_t volumes[AUDIO_NUM_VOICES]) {
  uint32_t packed = 0;
  for (int i = 0; i < AUDIO_NUM_VOICES; i++) {
    packed |= ((uint32_t)volumes[i] & 0xFu) << (i * 4);
  }
  AUDIO_VOL_REG = packed;
}
